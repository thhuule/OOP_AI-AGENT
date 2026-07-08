#pragma once

#include "client/llm_client.h"

namespace oop_agent {

/**
 * @brief Lớp thực thi cụ thể để kết nối với Ollama server [cite: 5, 31]
 * Sử dụng libcurl để thực hiện HTTP POST request.
 */
class OllamaClient : public LLMClient {
public:
    OllamaClient() = default;
    ~OllamaClient() override = default;

    /**
     * @brief Hiện thực hóa việc gửi request đến endpoint /api/chat của Ollama [cite: 5, 27]
     */
    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

private:
    /**
     * @brief Hàm callback để libcurl ghi nhận dữ liệu trả về từ server
     */
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent