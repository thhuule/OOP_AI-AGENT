#pragma once

#include <string>
#include <chrono>

namespace oop_agent {

/**
 * @brief Cấu trúc dữ liệu đại diện cho một tin nhắn trao đổi giữa các Sub-Agent.
 */
struct AgentMessage {
    std::string sender;      // ID hoặc tên của agent gửi tin
    std::string receiver;    // ID hoặc tên của agent nhận tin ("broadcast" nếu gửi cho tất cả)
    std::string content;     // Nội dung tin nhắn
    uint64_t timestamp = 0;  // Thời điểm gửi (epoch milliseconds)

    AgentMessage() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    AgentMessage(std::string s, std::string r, std::string c)
        : sender(std::move(s)), receiver(std::move(r)), content(std::move(c)) {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }
};

} // namespace oop_agent
