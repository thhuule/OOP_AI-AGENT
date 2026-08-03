#include "../src/multiagent/MultiAgentRunner.h"
#include <iostream>
#include <chrono>
#include <string>

using namespace oop_agent;

int main() {
    std::cout << "=== TEST MULTI-AGENT FRAMEWORK ===\n\n";

    MultiAgentRunner runner;

    runner.registerAgent("worker", "Agent kiểm thử message bus", [](MessageQueue& in, MessageQueue& out) {
        auto msg = in.pop(2000);
        if (msg) {
            out.push(AgentMessage("worker", "main", "RESULT:" + msg->content));
        }
    });

    runner.startAll();
    runner.sendMessage(AgentMessage("test", "worker", "ping"));

    auto result = runner.receiveMessage("main", 3000);
    runner.stopAndJoinAll();

    if (!result || result->sender != "worker" || result->content != "RESULT:ping") {
        std::cerr << "FAILED: dispatcher không chuyển đúng kết quả về main\n";
        return 1;
    }
    if (runner.isRunning()) {
        std::cerr << "FAILED: runner vẫn running sau stopAndJoinAll\n";
        return 1;
    }

    std::cout << "ALL PASSED\n";
    return 0;
}
