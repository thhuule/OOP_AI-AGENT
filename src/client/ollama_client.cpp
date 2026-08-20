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

LLMConfig OllamaClient::resolve_config(const LLMConfig& config) const {
    LLMConfig effective = config;
    if (effective.ollama_host.empty()) {
        effective.ollama_host = base_url_;
    }
    if (effective.ollama_model.empty()) {
        effective.ollama_model = model_name_;
    }
    return effective;
}

std::string OllamaClient::build_endpoint_url(const LLMConfig& config) const {
    const LLMConfig effective = resolve_config(config);
    std::string host = effective.ollama_host;
    if (!host.empty() && host.back() == '/') host.pop_back();
    if (host.rfind("/api/chat") != host.size() - 9) {
        host += "/api/chat";
    }
    return host;
}

nlohmann::json OllamaClient::build_request_body(
    const std::vector<Message>& conversation_history,
    const LLMConfig& config
) const {
    const LLMConfig effective = resolve_config(config);
    json request_json = json::object();
    request_json["model"] = effective.ollama_model;
    request_json["stream"] = false;

    json options = json::object();
    options["temperature"] = effective.temperature;
    if (effective.max_tokens > 0) {
        options["num_predict"] = effective.max_tokens;
    }
    request_json["options"] = options;

    json messages_arr = json::array();
    for (const auto& msg : conversation_history) {
        json m = { {"role", msg.role}, {"content", msg.content} };
        if (msg.images.has_value() && !msg.images->empty()) {
            std::vector<std::string> clean_images;
            for (const auto& img : *msg.images) {
                if (img.empty()) continue;
                auto pos = img.find(";base64,");
                if (pos != std::string::npos) {
                    clean_images.push_back(img.substr(pos + 8));
                } else {
                    clean_images.push_back(img);
                }
            }
            if (!clean_images.empty()) {
                m["images"] = clean_images;
            }
        }
        messages_arr.push_back(m);
    }
    request_json["messages"] = messages_arr;
    return request_json;
}

LLMUsage OllamaClient::parse_usage(const nlohmann::json& response) noexcept {
    try {
        return {response.value("prompt_eval_count", 0),
                response.value("eval_count", 0)};
    } catch (...) {
        return {};
    }
}

std::expected<std::string, LLMError> OllamaClient::generate_chat(
    const std::vector<Message>& conversation_history,
    const LLMConfig& config
) {
    last_usage_ = {};
    const LLMConfig effective = resolve_config(config);
    if (effective.ollama_host.empty() || effective.ollama_model.empty()) {
        return std::unexpected(LLMError::ConnectionRefused);
    }

    CURL* curl = curl_easy_init();
    if (!curl) return std::unexpected(LLMError::UnknownError);

    std::string read_buffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    json request_json = build_request_body(conversation_history, effective);
    std::string request_data = request_json.dump();
    std::string endpoint_url = build_endpoint_url(effective);

    curl_easy_setopt(curl, CURLOPT_URL, endpoint_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OllamaClient::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(effective.timeout_seconds));

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        if (res == CURLE_OPERATION_TIMEDOUT) {
            return std::unexpected(LLMError::Timeout);
        }
        return std::unexpected(LLMError::ConnectionRefused);
    }

    if (http_code != 200) {
        if (http_code == 429) return std::unexpected(LLMError::RateLimit);
        return std::unexpected(LLMError::ConnectionRefused);
    }

    try {
        auto res_json = json::parse(read_buffer);
        last_usage_ = parse_usage(res_json);
        if (res_json.contains("message") && res_json["message"].contains("content")) {
            return res_json["message"]["content"].get<std::string>();
        }
        return std::unexpected(LLMError::MalformedJSON);
    } catch (...) {
        return std::unexpected(LLMError::MalformedJSON);
    }
}

} // namespace oop_agent
