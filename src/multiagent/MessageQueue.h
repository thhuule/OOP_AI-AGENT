#pragma once

#include "Message.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace oop_agent {

/**
 * @brief Hàng đợi tin nhắn Thread-Safe hỗ trợ giao tiếp bất đồng bộ giữa các Agent threads.
 */
class MessageQueue {
public:
    MessageQueue() = default;
    ~MessageQueue() = default;

    // Không cho copy
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;

    /**
     * @brief Gửi một tin nhắn vào hàng đợi.
     */
    void push(Message msg) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(msg));
        }
        cv_.notify_one();
    }

    /**
     * @brief Lấy tin nhắn ra khỏi hàng đợi (chờ tối đa timeout_ms).
     * @return Message nếu có, hoặc std::nullopt nếu hết thời gian chờ.
     */
    std::optional<Message> pop(int timeout_ms = 500) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
            return !queue_.empty() || stopped_;
        })) {
            if (stopped_ && queue_.empty()) {
                return std::nullopt;
            }
            Message msg = std::move(queue_.front());
            queue_.pop();
            return msg;
        }
        return std::nullopt;
    }

    /**
     * @brief Đánh dấu dừng hàng đợi (dùng khi shutdown hệ thống).
     */
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

    /**
     * @brief Kiểm tra hàng đợi có rỗng không.
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Lấy số lượng tin nhắn trong hàng đợi.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Message> queue_;
    bool stopped_ = false;
};

} // namespace oop_agent
