#include "MultiAgentRunner.h"
#include <iostream>

namespace oop_agent {

MultiAgentRunner::~MultiAgentRunner() {
    if (running_) {
        stopAndJoinAll();
    }
}

void MultiAgentRunner::registerAgent(
    const std::string& id,
    const std::string& role_description,
    std::function<void(MessageQueue& in_queue, MessageQueue& out_queue)> task_func) {

    SubAgentConfig config;
    config.id = id;
    config.role_description = role_description;
    config.task_func = std::move(task_func);

    agents_[id] = std::move(config);
    in_queues_[id] = std::make_unique<MessageQueue>();

    std::cout << "[MultiAgentRunner] Đã đăng ký Sub-Agent: " << id
              << " (" << role_description << ")\n";
}

void MultiAgentRunner::startAll() {
    if (running_) return;
    running_ = true;

    std::cout << "[MultiAgentRunner] Khởi chạy " << agents_.size() << " Sub-Agents...\n";

    // Khởi chạy thread điều phối (Dispatcher) trung chuyển tin nhắn từ global_bus_ đến các agent
    dispatcher_thread_ = std::thread([this]() {
        while (running_) {
            auto msg = global_bus_.pop(100);
            if (msg.has_value()) {
                sendMessage(msg.value());
            }
        }
    });

    for (auto& [id, config] : agents_) {
        auto* in_q = in_queues_[id].get();
        auto task_func = config.task_func; // Copy task_func ra biến cục bộ trước khi spawn thread

        // Spawn thread riêng cho từng Sub-Agent
        worker_threads_.emplace_back([this, id, func = std::move(task_func), in_q]() {
            std::cout << "[SubAgent:" << id << "] Thread đã khởi động.\n";
            try {
                // Thực thi hàm công việc của Agent
                func(*in_q, global_bus_);
            } catch (const std::exception& e) {
                std::cerr << "[SubAgent:" << id << "] Lỗi ngoại lệ: " << e.what() << "\n";
            }
            std::cout << "[SubAgent:" << id << "] Thread kết thúc.\n";
        });
    }
}

void MultiAgentRunner::sendMessage(const AgentMessage& msg) {
    if (msg.receiver == "broadcast") {
        for (auto& [id, queue] : in_queues_) {
            queue->push(msg);
        }
    } else {
        auto it = in_queues_.find(msg.receiver);
        if (it != in_queues_.end()) {
            it->second->push(msg);
        } else {
            std::cerr << "[MultiAgentRunner] Không tìm thấy receiver: " << msg.receiver << "\n";
        }
    }
}

void MultiAgentRunner::stopAndJoinAll() {
    if (!running_) return;

    std::cout << "[MultiAgentRunner] Đang dừng tất cả Sub-Agents...\n";

    // 1. Gửi tín hiệu stop cho tất cả các hàng đợi
    for (auto& [id, queue] : in_queues_) {
        queue->stop();
    }
    global_bus_.stop();

    // 2. Join thread điều phối (Dispatcher)
    if (dispatcher_thread_.joinable()) {
        dispatcher_thread_.join();
    }

    // 3. Join tất cả worker threads
    for (auto& t : worker_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }

    worker_threads_.clear();
    running_ = false;

    std::cout << "[MultiAgentRunner] Đã dừng toàn bộ Sub-Agents an toàn.\n";
}

} // namespace oop_agent
