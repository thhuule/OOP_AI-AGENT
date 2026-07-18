#pragma once

#include "client/llm_client.h"
#include <string> // Cần thiết cho std::string

namespace oop_agent {

class OllamaClient : public LLMClient {
public:
    // Cập nhật: Thêm constructor nhận tham số
    OllamaClient(const std::string& url, const std::string& model) 
        : base_url(url), model_name(model) {}

    // Giữ lại constructor mặc định nếu cần
    OllamaClient() = default;
    
    ~OllamaClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

private:
    // Thêm các biến thành viên để lưu trữ cấu hình
    std::string base_url;
    std::string model_name;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent