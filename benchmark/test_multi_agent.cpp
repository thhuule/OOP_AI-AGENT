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
    const bool demo_success = harness.runMultiAgentDemo(
        MultiAgentDemoInput{"2 * 3", "Japan capital"}, report.string());
    std::ifstream report_file(report);
    std::string contents((std::istreambuf_iterator<char>(report_file)), {});
    std::filesystem::remove(report);
    if (contents.empty() || contents.find("STATUS=") == std::string::npos ||
        contents.find("CALC=6") == std::string::npos) {
        std::cerr << "FAILED: report multi-agent thieu status/ket qua calculator\n";
        return 1;
    }
    if (demo_success && (contents.find("STATUS=PASS") == std::string::npos ||
                         contents.find("CAPITAL=") == std::string::npos)) {
        std::cerr << "FAILED: demo PASS nhung report thieu ket qua researcher\n";
        return 1;
    }
    if (!demo_success && (contents.find("STATUS=FAIL") == std::string::npos ||
                          contents.find("ERROR=researcher:") == std::string::npos)) {
        std::cerr << "FAILED: demo FAIL khong ghi ro loi researcher\n";
        return 1;
    }

    const std::filesystem::path failed_report =
        std::filesystem::path("artifacts") / "test_multi_agent_failure_report.txt";
    if (harness.runMultiAgentDemo(
            MultiAgentDemoInput{"2 * 3", ""}, failed_report.string())) {
        std::cerr << "FAILED: worker error khong duoc phep tao demo PASS\n";
        return 1;
    }
    std::ifstream failed_report_file(failed_report);
    std::string failed_contents(
        (std::istreambuf_iterator<char>(failed_report_file)), {});
    std::filesystem::remove(failed_report);
    if (failed_contents.find("STATUS=FAIL") == std::string::npos ||
        failed_contents.find("ERROR=researcher:InvalidArgument") == std::string::npos ||
        failed_contents.find("CAPITAL=") != std::string::npos) {
        std::cerr << "FAILED: worker error bi bao cao sai thanh success\n";
        return 1;
    }

    std::cout << "ALL PASSED\n";
    return 0;
}
