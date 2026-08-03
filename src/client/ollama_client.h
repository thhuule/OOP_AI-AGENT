#pragma once

#include "client/llm_client.h"
#include <string>

namespace oop_agent {

class OllamaClient : public LLMClient {
public:
    // Constructor nhận url và model
    OllamaClient(const std::string& url, const std::string& model) 
        : base_url(url), model_name(model) {}

    // Constructor mặc định
    OllamaClient() = default;
    
    ~OllamaClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

private:
    std::string base_url;
    std::string model_name;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent