#pragma once

#include "MessageQueue.h"
#include "../agent/agent_loop.h"
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <unordered_map>
#include <functional>
#include <atomic>

namespace oop_agent {

/**
 * @brief Cấu hình và state cho một Sub-Agent trong Multi-Agent Framework.
 */
struct SubAgentConfig {
    std::string id;                                     // ID duy nhất của Sub-Agent (ví dụ: "coder", "tester")
    std::string role_description;                       // Vai trò / Prompt ban đầu
    std::function<void(MessageQueue&, MessageQueue&)> task_func; // Hàm thực thi công việc của agent
};

/**
 * @brief Bộ điều phối chạy nhiều Agent đồng thời trên các thread riêng biệt.
 */
class MultiAgentRunner {
public:
    MultiAgentRunner() = default;
    ~MultiAgentRunner();

    // Không cho copy
    MultiAgentRunner(const MultiAgentRunner&) = delete;
    MultiAgentRunner& operator=(const MultiAgentRunner&) = delete;

    /**
     * @brief Đăng ký một Sub-Agent vào hệ thống.
     * @param id ID duy nhất của agent
     * @param role_description Mô tả vai trò
     * @param task_func Hàm công việc dạng [](MessageQueue& in, MessageQueue& out)
     */
    void registerAgent(const std::string& id,
                       const std::string& role_description,
                       std::function<void(MessageQueue& in_queue, MessageQueue& out_queue)> task_func);

    /**
     * @brief Khởi chạy toàn bộ Sub-Agent trên các std::thread riêng.
     */
    void startAll();

    /**
     * @brief Gửi tin nhắn đến một Agent cụ thể hoặc broadcast.
     */
    void sendMessage(const AgentMessage& msg);

    /**
     * @brief Nhận tin nhắn đã được dispatcher chuyển tới một receiver.
     * @return Tin nhắn nếu nhận được trước timeout, std::nullopt nếu hết hạn.
     */
    [[nodiscard]] std::optional<AgentMessage>
    receiveMessage(const std::string& receiver, int timeout_ms = 500);

    /**
     * @brief Chờ tất cả Sub-Agent hoàn thành nhiệm vụ và dọn dẹp threads.
     */
    void stopAndJoinAll();

    /**
     * @brief Kiểm tra hệ thống có đang chạy không.
     */
    bool isRunning() const { return running_; }

private:
    std::unordered_map<std::string, SubAgentConfig> agents_;
    std::unordered_map<std::string, std::unique_ptr<MessageQueue>> in_queues_;
    MessageQueue global_bus_; // Bus tin nhắn chung

    std::vector<std::thread> worker_threads_;
    std::thread dispatcher_thread_;
    std::atomic<bool> running_{false};
};

} // namespace oop_agent
