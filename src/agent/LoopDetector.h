#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace oop_agent {

class LoopDetector {
public:
    enum class Status { Normal, Warning, Critical };

    explicit LoopDetector(int warning_threshold = 3, int critical_threshold = 5);
    ~LoopDetector() = default;

    // C++20: Dùng std::string_view làm tham số để tránh copy string không cần thiết
    Status add_action(std::string_view action);

    void reset();

private:
    std::vector<std::string> history_;
    int warning_threshold_ = 3;
    int critical_threshold_ = 5;

    bool check_generic_repeat(int n) const;
    bool check_ping_pong(int cycles) const;
};

} // namespace oop_agentgent