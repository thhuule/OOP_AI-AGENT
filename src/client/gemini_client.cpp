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

std::string GeminiClient::build_url(const LLMConfig& config) const {
    // Use provided config values for URL, model and API key
    std::string base = config.gemini_api_url;
    // Ensure no trailing slash
    if (!base.empty() && base.back() == '/') base.pop_back();
    return base + "/models/" + config.gemini_model + ":generateContent?key=" + config.api_key;
}

GeminiClient::GeminiClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_name_(model) {}

std::string GeminiClient::build_url() const {
    return "https://generativelanguage.googleapis.com/v1beta/models/" + model_name_ + ":generateContent?key=" + api_key_;
}

nlohmann::json GeminiClient::build_request_body(
    const std::vector<Message>& history,
    const LLMConfig& config
) const {
    nlohmann::json request = nlohmann::json::object();
    nlohmann::json contents = nlohmann::json::array();
    nlohmann::json system_parts = nlohmann::json::array();

    for (const auto& msg : history) {
        // FIX (Bug 2): Gemini's REST API has no "system" role inside `contents`.
        // System prompts must go into `systemInstruction`, otherwise the API
        // either drops them or the model behaves unpredictably (native
        // function-calling attempts with no declared tools -> MALFORMED_FUNCTION_CALL).
        if (msg.role == "system") {
            nlohmann::json part = nlohmann::json::object();
            part["text"] = msg.content;
            system_parts.push_back(part);
            continue;
        }

        // Role mapping: assistant -> model, tool -> user (observation goes
        // back to the model as a user turn), user -> user.
        std::string gemini_role = msg.role;
        if (gemini_role == "assistant") gemini_role = "model";
        else if (gemini_role == "tool") gemini_role = "user";
        else gemini_role = "user";

        nlohmann::json part = nlohmann::json::object();
        part["text"] = msg.content;

        nlohmann::json content_entry = nlohmann::json::object();
        content_entry["role"] = gemini_role;
        content_entry["parts"] = nlohmann::json::array({part});
        contents.push_back(content_entry);
    }

    request["contents"] = contents;

    if (!system_parts.empty()) {
        nlohmann::json system_instruction = nlohmann::json::object();
        system_instruction["parts"] = system_parts;
        request["systemInstruction"] = system_instruction;
    }

    nlohmann::json generation_config = nlohmann::json::object();
    generation_config["temperature"] = config.temperature;
    request["generationConfig"] = generation_config;

    return request;
}

size_t GeminiClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    return WriteCallback(contents, size, nmemb, userp);
}

HttpResponse GeminiClient::send_request_raw(const nlohmann::json& payload, const LLMConfig& config) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {0, ""};
    }

    std::string response_string;
    long http_code = 0;

    std::string url = build_url(config);
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string json_str = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(config.timeout_seconds));

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

HttpResponse GeminiClient::send_request(const nlohmann::json& payload, const LLMConfig& config) {
    const int max_retries = 3;

    for (int attempt = 1; attempt <= max_retries; ++attempt) {
        HttpResponse res = send_request_raw(payload, config);
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
    HttpResponse res = send_request(payload, config);

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
        if (!first_candidate.contains("content") ||
            !first_candidate["content"].contains("parts")) {
            return std::unexpected(LLMError::MalformedJSON);
        }

        const auto& parts = first_candidate["content"]["parts"];

        // FIX (Bug 1): scan ALL parts instead of blindly taking parts[0].
        // A "thought" text part often comes before the actual functionCall
        // part; grabbing parts[0] silently threw every tool call away.
        std::string accumulated_text;

        for (const auto& part : parts) {
            // Some Gemini "thinking" outputs mark internal reasoning with
            // "thought": true — skip those, they are not meant to be acted on.
            const bool is_thought = part.contains("thought") && part["thought"].get<bool>();

            if (part.contains("functionCall") && !is_thought) {
                const auto& fc = part["functionCall"];
                nlohmann::json tool_call = nlohmann::json::object();
                tool_call["tool"] = fc.value("name", "");
                if (fc.contains("args") && !fc["args"].is_null()) {
                    // Tool::execute() takes a single string; downstream tools
                    // (Role B) parse JSON args, so stringify the object here.
                    tool_call["args"] = fc["args"].dump();
                } else {
                    tool_call["args"] = "";
                }
                // Returning this as the "llm_text" lets AgentLoop's existing
                // JSON-tool-call regex in parse_llm_response() pick it up
                // without needing a separate Action variant for native calls.
                return tool_call.dump();
            }

            if (part.contains("text") && !is_thought) {
                accumulated_text += part["text"].get<std::string>();
            }
        }

        if (!accumulated_text.empty()) {
            return accumulated_text;
        }

        // Nothing usable came back (e.g. finishReason == MALFORMED_FUNCTION_CALL
        // with only thought parts). Surface it as malformed so AgentLoop can
        // retry instead of silently returning "".
        return std::unexpected(LLMError::MalformedJSON);
    } catch (...) {
        return std::unexpected(LLMError::MalformedJSON);
    }
}

} // namespace oop_agent