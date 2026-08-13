#include "../src/harness/HarnessRunner.h"
#include <iostream>
#include <fstream>

using namespace oop_agent;

int main() {
    std::cout << "=====================================================\n";
    std::cout << "   DEMO SCENARIO MULTI-AGENT RUNNER (WEEK 7 - C)     \n";
    std::cout << "=====================================================\n\n";

    HarnessRunner harness("benchmark/tasks.json");
    if (!harness.runMultiAgentDemo()) {
        std::cerr << "[Main] Không thể hoàn tất multi-agent workflow.\n";
        return 1;
    }

    std::cout << "\n=====================================================\n";
    std::cout << "       DEMO MULTI-AGENT HOÀN THÀNH THÀNH CÔNG        \n";
    std::cout << "=====================================================\n";

    return 0;
}
