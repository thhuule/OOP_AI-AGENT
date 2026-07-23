#include "../src/multiagent/MultiAgentRunner.h"
#include <iostream>
#include <chrono>

using namespace oop_agent;

int main() {
    std::cout << "=== DEMO MULTI-AGENT FRAMEWORK ===\n\n";

    MultiAgentRunner runner;

    // Agent 1: Coder
    runner.registerAgent("coder", "Agent viết mã", [](MessageQueue& in, MessageQueue& out) {
        // Nhận yêu cầu
        auto msg = in.pop(2000);
        if (msg) {
            std::cout << "[Coder] Nhận nhiệm vụ từ " << msg->sender << ": " << msg->content << "\n";
            // Xử lý và gửi kết quả cho Tester
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            out.push(AgentMessage("coder", "tester", "Đã viết xong hàm calculateSum()"));
        }
    });

    // Agent 2: Tester
    runner.registerAgent("tester", "Agent kiểm thử", [](MessageQueue& in, MessageQueue& out) {
        auto msg = in.pop(2000);
        if (msg) {
            std::cout << "[Tester] Nhận kết quả từ " << msg->sender << ": " << msg->content << "\n";
            std::cout << "[Tester] Tiến hành chạy unit tests: ALL PASSED!\n";
        }
    });

    // Bắt đầu các thread
    runner.startAll();

    // Gửi tin nhắn khởi động cho Coder
    runner.sendMessage(AgentMessage("user", "coder", "Hãy viết hàm tính tổng 2 số"));

    // Đợi hoàn thành
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    runner.stopAndJoinAll();

    std::cout << "\n=== DEMO KẾT THÚC THÀNH CÔNG ===\n";
    return 0;
}
