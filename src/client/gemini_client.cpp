#include "client/gemini_client.h"

#include <chrono>
#include <curl/curl.h>
#include <iostream>
#include <thread>

namespace oop_agent {

namespace {
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total_size);
    return total_size;
}
} // namespace

GeminiClient::GeminiClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_name_(model) {}

std::string GeminiClient::build_url() const {
    return "https://generativelanguage.googleapis.com/v1beta/models/" + model_name_ + ":generateContent?key=" + api_key_;
}

nlohmann::json GeminiClient::build_request_body(
    const std::vector<Message>& history,
    const LLMConfig& config
) const {
    (void)config;
    nlohmann::json request = nlohmann::json::object();
    nlohmann::json contents = nlohmann::json::array();

    for (const auto& msg : history) {
        nlohmann::json part = nlohmann::json::object();
        part["text"] = msg.content;
        nlohmann::json content_entry = nlohmann::json::object();
        content_entry["role"] = msg.role;
        content_entry["parts"] = nlohmann::json::array({part});
        contents.push_back(content_entry);
    }

    request["contents"] = contents;
    return request;
}

size_t GeminiClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    return WriteCallback(contents, size, nmemb, userp);
}

HttpResponse GeminiClient::send_request_raw(const nlohmann::json& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {0, ""};
    }

    std::string response_string;
    long http_code = 0;

    std::string url = build_url();
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string json_str = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    } else {
        std::cerr << "[GeminiClient] cURL error: " << curl_easy_strerror(res) << "\n";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return {static_cast<int>(http_code), response_string};
}

HttpResponse GeminiClient::send_request(const nlohmann::json& payload) {
    const int max_retries = 3;

    for (int attempt = 1; attempt <= max_retries; ++attempt) {
        HttpResponse res = send_request_raw(payload);
        std::cout << "HTTP Code: " << res.status_code << std::endl;

        if (res.status_code == 200) {
            return res;
        }

        if (res.status_code == 0 || res.status_code == 503 || res.status_code == 429) {
            if (attempt < max_retries) {
                int sleep_sec = attempt * 2;
                std::cout << "[GeminiClient] Warning: Got HTTP " << res.status_code
                          << " on attempt " << attempt << ". Retrying in " << sleep_sec << "s...\n";
                std::this_thread::sleep_for(std::chrono::seconds(sleep_sec));
                continue;
            }
        }

        return res;
    }

    return {0, ""};
}

std::expected<std::string, LLMError> GeminiClient::generate_chat(
    const std::vector<Message>& conversation_history,
    const LLMConfig& config
) {
    nlohmann::json payload = build_request_body(conversation_history, config);
    HttpResponse res = send_request(payload);

    if (res.status_code != 200) {
        if (res.status_code == 429) {
            return std::unexpected(LLMError::RateLimit);
        }
        return std::unexpected(LLMError::ConnectionRefused);
    }

    try {
        auto res_json = nlohmann::json::parse(res.body);
        if (!res_json.contains("candidates") || res_json["candidates"].empty()) {
            return std::unexpected(LLMError::MalformedJSON);
        }

        const auto& first_candidate = res_json["candidates"][0];
        if (!first_candidate.contains("content") || !first_candidate["content"].contains("parts") || first_candidate["content"]["parts"].empty()) {
            return std::unexpected(LLMError::MalformedJSON);
        }

        return first_candidate["content"]["parts"][0]["text"].get<std::string>();
    } catch (...) {
        return std::unexpected(LLMError::MalformedJSON);
    }
}

} // namespace oop_agent