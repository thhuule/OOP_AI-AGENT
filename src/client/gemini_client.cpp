#include "client/gemini_client.h"
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <chrono>

using json = nlohmann::json;

namespace oop_agent {

// Constructor
GeminiClient::GeminiClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_name_(model) {}

// Build URL theo chuẩn Gemini API (Không bắt buộc đính `?key=` ở URL vì dùng Header x-goog-api-key)
std::string GeminiClient::build_url() const {
    return "https://generativelanguage.googleapis.com/v1beta/models/" 
           + model_name_ + ":generateContent";
}

// Chuyển đổi Message vector -> Gemini JSON format
json GeminiClient::build_request_body(
    const std::vector<Message>& history,
    const LLMConfig& config
) const {
    json body;
    json contents = json::array();
    
    for (const auto& msg : history) {
        if (msg.role == "system") {
            // System prompt -> systemInstruction (ngoài contents)
            body["systemInstruction"] = {
                {"parts", {{{"text", msg.content}}}}
            };
            continue;
        }
        
        // Map role: assistant -> model, tool -> user
        std::string gemini_role = msg.role;
        if (gemini_role == "assistant") gemini_role = "model";
        if (gemini_role == "tool")      gemini_role = "user";
        
        json parts = json::array();
        parts.push_back({{"text", msg.content}});
        
        // Hỗ trợ Multimodal (ảnh base64)
        if (msg.images.has_value()) {
            for (const auto& img : msg.images.value()) {
                parts.push_back({
                    {"inlineData", {
                        {"mimeType", "image/png"},
                        {"data", img}
                    }}
                });
            }
        }
        
        contents.push_back({{"role", gemini_role}, {"parts", parts}});
    }
    
    body["contents"] = contents;
    body["generationConfig"] = {
        {"temperature", config.temperature}
    };
    
    return body;
}

// Callback cho libcurl
size_t GeminiClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// Hàm thực thi chính
std::expected<std::string, LLMError> GeminiClient::generate_chat(
    const std::vector<Message>& conversation_history,
    const LLMConfig& config
) {
    const int max_attempts = 2;

    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) return std::unexpected(LLMError::UnknownError);

        std::string read_buffer;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // 💡 Bổ sung Header x-goog-api-key để chấp nhận định dạng Key mới
        std::string key_header = "x-goog-api-key: " + api_key_;
        headers = curl_slist_append(headers, key_header.c_str());

        std::string url = build_url();
        json request_json = build_request_body(conversation_history, config);
        std::string request_data = request_json.dump();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GeminiClient::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout_seconds);

        CURLcode res = curl_easy_perform(curl);

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            if (res == CURLE_OPERATION_TIMEDOUT) {
                return std::unexpected(LLMError::Timeout);
            }
            return std::unexpected(LLMError::ConnectionRefused);
        }

        bool is_429 = (http_code == 429);
        json res_json;
        bool parse_success = false;

        try {
            res_json = json::parse(read_buffer);
            parse_success = true;
            if (res_json.contains("error")) {
                int err_code = res_json["error"].value("code", 0);
                if (err_code == 429) {
                    is_429 = true;
                }
            }
        } catch (...) {
            if (!is_429) {
                return std::unexpected(LLMError::MalformedJSON);
            }
        }

        // Xử lý lỗi Rate Limit HTTP 429: sleep 2 giây rồi retry 1 lần
        if (is_429) {
            if (attempt == 0) {
                std::cerr << "[GeminiClient] HTTP 429 (Rate Limit). Sleep 2s roi retry...\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            } else {
                return std::unexpected(LLMError::RateLimit);
            }
        }

        if (parse_success) {
            if (res_json.contains("error")) {
                return std::unexpected(LLMError::UnknownError);
            }

            if (res_json.contains("candidates") && !res_json["candidates"].empty()) {
                const auto& candidate = res_json["candidates"][0];
                if (candidate.contains("content") && candidate["content"].contains("parts")) {
                    const auto& parts = candidate["content"]["parts"];
                    if (!parts.empty() && parts[0].contains("text")) {
                        return parts[0]["text"].get<std::string>();
                    }
                }
            }

            return std::unexpected(LLMError::MalformedJSON);
        }

        return std::unexpected(LLMError::MalformedJSON);
    }

    return std::unexpected(LLMError::RateLimit);
}

} // namespace oop_agent