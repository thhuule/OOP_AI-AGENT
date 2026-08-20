#pragma once

#include "client/llm_client.h"
#include <nlohmann/json.hpp>
#include <string>

namespace oop_agent {

class OllamaClient : public LLMClient {
public:
    // Constructor nhận url và model
    OllamaClient(const std::string& url = "", const std::string& model = "") 
        : base_url_(url), model_name_(model) {}
    
    ~OllamaClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

    [[nodiscard]] LLMUsage last_usage() const noexcept override { return last_usage_; }

    LLMConfig resolve_config(const LLMConfig& config) const;
    std::string build_endpoint_url(const LLMConfig& config) const;
    nlohmann::json build_request_body(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) const;
    [[nodiscard]] static LLMUsage parse_usage(const nlohmann::json& response) noexcept;

private:
    std::string base_url_;
    std::string model_name_;
    LLMUsage last_usage_;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent
