#include "agent/agent_loop.h"
#include "tools/WebSearchTool.h"
#include "tools/MemoryTool.h"
#include "tools/TimeTool.h"
#include "tools/JsonTool.h"
#include "tools/GitTool.h"
#include <iostream>
#include <memory>
#include <string>

int main() {
  // 1. Khởi tạo Client Ollama thật với địa chỉ URL và model
  auto ollama_client = std::make_shared<oop_agent::OllamaClient>(
      "http://oihnt-35-233-204-204.free.pinggy.net",
      "gemma4:e4b"
  );

int main()
{
    auto client = std::make_shared<OllamaClient>();

    AgentLoop agent(client);

    // Register tools
    agent.register_tool(std::make_shared<CalculatorTool>());
    agent.register_tool(std::make_shared<FileTool>());
    agent.register_tool(std::make_shared<ExecTool>());
    agent.register_tool(std::make_shared<WebSearchTool>());
    agent.register_tool(std::make_shared<MemoryTool>());
    agent.register_tool(std::make_shared<TimeTool>());
    agent.register_tool(std::make_shared<JsonTool>());
    agent.register_tool(std::make_shared<GitTool>());

    std::cout << "========================================\n";
    std::cout << "        OOP AI Agent - Week 5\n";
    std::cout << "========================================\n";
    std::cout << "Type 'exit' to quit.\n";

    while (true)
    {
        std::string instruction;

        std::cout << "\nUser> ";
        std::getline(std::cin, instruction);

        if (instruction == "exit")
        {
            break;
        }

        std::string answer = agent.run(instruction);

        std::cout << "\nAgent> "
                  << answer
                  << "\n";
    }
  }
  oop_agent::HarnessRunner harness(tasks_path, output_dir);
  harness.loadTasks();

  // 5. Inject StepHook
  agent.set_step_hook(harness.createStepHook());

  // Kết nối Agent vào Harness
  harness.set_agent(&agent);

  // 6. Chạy benchmark
  auto results = harness.runAll();
  harness.exportResults(results);

  return 0;
}