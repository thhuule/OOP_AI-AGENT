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
    std::optional<std::vector<std::string>> images = std::nullopt; 
};

// 3. Khai báo cấu hình tham số
struct LLMConfig {
    // Phân biệt provider: "gemini" hoặc "ollama"
    std::string provider = "gemini";             
    
    // Cấu hình dành riêng cho Gemini
    std::string gemini_model = "gemini-2.5-flash"; 
    std::string gemini_api_url = "https://generativelanguage.googleapis.com/v1beta";
    std::string api_key = "";

    // Cấu hình dành riêng cho Ollama (chạy local)
    std::string ollama_model = "gemma4:e4b";
    std::string ollama_host = "http://localhost:11434";

    // Tham số chung
    float temperature = 0.7f;
    int timeout_seconds = 60;                    
    int max_tokens = 2048;                       
};

struct LLMUsage {
    int prompt_tokens = 0;
    int completion_tokens = 0;

    [[nodiscard]] int total_tokens() const noexcept {
        return prompt_tokens + completion_tokens;
    }
};

// 4. Lớp giao tiếp cơ sở
class LLMClient {
public:
    virtual ~LLMClient() = default;

    virtual std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) = 0;

    /// Token metadata for the most recent generate_chat() call. Providers
    /// that do not return usage keep the default zero value (not measured).
    [[nodiscard]] virtual LLMUsage last_usage() const noexcept { return {}; }
};

} // namespace oop_agent
