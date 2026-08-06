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

// 3. Khai báo cấu hình tham số dành riêng cho Gemini
struct LLMConfig {
    // Phân biệt provider: "gemini" hoặc "ollama"
    std::string provider = "gemini";             
    
    // Cấu hình dành riêng cho Gemini
    std::string gemini_model = "gemma-4-31b-it"; 
    std::string gemini_api_url = "https://generativelanguage.googleapis.com/v1beta";
    std::string api_key = "YOUR_GEMINI_API_KEY";

    // Cấu hình dành riêng cho Ollama (chạy local)
    std::string ollama_model = "llama3";         // Hoặc mistral, gemma, v.v.
    std::string ollama_host = "http://localhost:11434";

    // Tham số chung
    float temperature = 0.7f;
    int timeout_seconds = 60;                    
    int max_tokens = 2048;                       
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

} // namespace oop_agent
