#include "../src/multiagent/MultiAgentRunner.h"
#include "../src/harness/HarnessRunner.h"
#include <filesystem>
#include <fstream>
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

    const std::filesystem::path report =
        std::filesystem::path("artifacts") / "test_multi_agent_report.txt";
    HarnessRunner harness("benchmark/tasks.json");
    if (!harness.runMultiAgentDemo(report.string())) {
        std::cerr << "FAILED: Harness khong chay duoc multi-agent workflow\n";
        return 1;
    }
    std::ifstream report_file(report);
    std::string contents((std::istreambuf_iterator<char>(report_file)), {});
    std::filesystem::remove(report);
    if (contents.find("CALC=1081") == std::string::npos ||
        contents.find("CAPITAL=") == std::string::npos) {
        std::cerr << "FAILED: report multi-agent thieu ket qua worker\n";
        return 1;
    }

    std::cout << "ALL PASSED\n";
    return 0;
}
