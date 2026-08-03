#include "client/ollama_client.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace oop_agent {

size_t OllamaClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::expected<std::string, LLMError> OllamaClient::generate_chat(
    const std::vector<Message>& conversation_history,
    const LLMConfig& config
) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected(LLMError::UnknownError);

    std::string read_buffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // 1. Khởi tạo cấu trúc JSON Request theo chuẩn Ollama API
    json request_json;
    request_json["model"] = config.model_name; // Mặc định tuân theo config là "gemma4" hoặc "gemma4:e4b"
    request_json["stream"] = false;
    request_json["options"] = { {"temperature", config.temperature} };

    json messages_arr = json::array();
    for (const auto& msg : conversation_history) {
        json m = { {"role", msg.role}, {"content", msg.content} };
        // Nếu có ảnh (Multimodal hỗ trợ base64) thì đính kèm vào luôn
        if (msg.images.has_value() && !msg.images->empty()) {
            m["images"] = msg.images.value();
        }
        messages_arr.push_back(m);
    }
    request_json["messages"] = messages_arr;

    std::string request_data = request_json.dump();

    // 2. Cấu hình cURL
    curl_easy_setopt(curl, CURLOPT_URL, config.api_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OllamaClient::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout_seconds);

    // 3. Thực thi Request & Xử lý lỗi hệ thống mạng
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            return std::unexpected(LLMError::Timeout);
        }
        return std::unexpected(LLMError::ConnectionRefused);
    }

    curl_easy_cleanup(curl);

    // 4. Parse phản hồi từ Ollama
    try {
        auto res_json = json::parse(read_buffer);
        if (res_json.contains("message") && res_json["message"].contains("content")) {
            return res_json["message"]["content"].get<std::string>();
        }
        return std::unexpected(LLMError::MalformedJSON);
    } catch (...) {
        return std::unexpected(LLMError::MalformedJSON);
    }
}

} // namespace oop_agent