#include "agent/LoopDetector.h"
#include <algorithm>
#include <ranges> // C++20 Ranges

namespace oop_agent {

LoopDetector::LoopDetector(int warning_threshold, int critical_threshold)
    : warning_threshold_(warning_threshold), critical_threshold_(critical_threshold) {}

void LoopDetector::reset() {
    history_.clear();
}

bool LoopDetector::check_generic_repeat(int n) const {
    if (static_cast<int>(history_.size()) < n) {
        return false;
    }

    // C++20 Feature: Dùng std::views::drop để bỏ qua các phần tử đầu, chỉ lấy n phần tử cuối
    auto tail = history_ | std::views::drop(history_.size() - n);
    const std::string& last_action = history_.back();

    // C++20 Feature: Dùng std::ranges::all_of để kiểm tra tất cả phần tử trong view
    return std::ranges::all_of(tail, [&last_action](const std::string& act) {
        return act == last_action;
    });
}

bool LoopDetector::check_ping_pong(int cycles) const {
    int required_len = cycles * 2;
    if (static_cast<int>(history_.size()) < required_len) {
        return false;
    }

    size_t sz = history_.size();
    const std::string& a = history_[sz - 2];
    const std::string& b = history_[sz - 1];

    if (a == b) {
        return false;
    }

    for (int i = 0; i < cycles; ++i) {
        if (history_[sz - 2 - (i * 2)] != a || history_[sz - 1 - (i * 2)] != b) {
            return false;
        }
    }
    return true;
}

LoopDetector::Status LoopDetector::add_action(std::string_view action) {
    // Chuyển string_view thành std::string khi push vào vector
    history_.emplace_back(action);

    if (check_generic_repeat(critical_threshold_) || check_ping_pong(critical_threshold_)) {
        return Status::Critical;
    }

    if (check_generic_repeat(warning_threshold_) || check_ping_pong(warning_threshold_)) {
        return Status::Warning;
    }

    return Status::Normal;
}

} // namespace oop_agent