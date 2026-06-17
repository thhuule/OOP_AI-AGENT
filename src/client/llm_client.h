#pragma once

#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include <expected> 

namespace oop_agent {

// 1. Phải khai báo enum lỗi ngay đầu namespace để các struct bên dưới nhìn thấy
enum class LLMError {
    ConnectionRefused, 
    Timeout,           
    MalformedJSON,     
    RateLimit,         
    UnknownError       
};

// 2. Khai báo cấu trúc dữ liệu Message
struct Message {
    std::string role;    
    std::string content; 
    std::optional<std::vector<std::string>> images; 
};

// 3. Khai báo cấu hình tham số
struct LLMConfig {
    std::string model_name = "gemma4"; 
    std::string api_url = "http://localhost:11434/api/chat"; 
    float temperature = 0.7f;
    int timeout_seconds = 30; 
};

// 4. Lớp giao tiếp cơ sở
class LLMClient {
public:
    virtual ~LLMClient() = default;

    virtual std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) = 0;
};

// 5. Lớp thực thi cụ thể cho Ollama
class OllamaClient : public LLMClient {
private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    OllamaClient() = default;
    ~OllamaClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;
};

} // namespace oop_agent
