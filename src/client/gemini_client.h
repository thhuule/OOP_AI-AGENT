#pragma once
#include <nlohmann/json.hpp>
#include "client/llm_client.h"

namespace oop_agent {

struct HttpResponse {
    int status_code = 0;
    std::string body;
};

class GeminiClient : public LLMClient {
public:
    GeminiClient() = default;
    GeminiClient(const std::string& api_key, const std::string& model = "gemma-4-31b-it");
    ~GeminiClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

    LLMConfig resolve_config(const LLMConfig& config) const;
    std::string build_url() const;
    std::string build_url(const LLMConfig& config) const;
    nlohmann::json build_request_body(
        const std::vector<Message>& history,
        const LLMConfig& config = LLMConfig{}
    ) const;

private:
    std::string api_key_;
    std::string model_name_;

    HttpResponse send_request_raw(const nlohmann::json& payload, const LLMConfig& config);
    HttpResponse send_request(const nlohmann::json& payload, const LLMConfig& config);
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent