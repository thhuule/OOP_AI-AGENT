#pragma once
#include <nlohmann/json.hpp>
#include "client/llm_client.h"

namespace oop_agent {

class GeminiClient : public LLMClient {
public:
    GeminiClient(const std::string& api_key, const std::string& model = "gemini-2.5-flash");
    ~GeminiClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

private:
    std::string api_key_;
    std::string model_name_;
    
    // Build URL: https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent?key={key}
    std::string build_url() const;
    
    // Convert Message vector → Gemini JSON format
    nlohmann::json build_request_body(
        const std::vector<Message>& history, 
        const LLMConfig& config
    ) const;
    
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent