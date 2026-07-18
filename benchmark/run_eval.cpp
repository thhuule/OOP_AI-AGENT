#include "agent/agent_loop.h"
#include "client/ollama_client.h"
#include "tools/CalculatorTool.h"
#include "tools/ExecTool.h"
#include "tools/FileTool.h"
#include "tools/WebSearchTool.h"
#include "tools/MemoryTool.h"
#include "tools/TimeTool.h"
#include "tools/JsonTool.h"
#include "tools/GitTool.h"
#include "agent/SkillLoader.h"
#include "harness/HarnessRunner.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <filesystem>

int main() {
  // 1. Khởi tạo Client Ollama thật với địa chỉ URL và model
  auto ollama_client = std::make_shared<oop_agent::OllamaClient>(
      "http://oihnt-35-233-204-204.free.pinggy.net",
      "gemma4:e4b"
  );

  // 2. Setup AgentLoop + tools
  oop_agent::AgentLoop agent(ollama_client);
  agent.register_tool(std::make_shared<oop_agent::CalculatorTool>());
  agent.register_tool(std::make_shared<oop_agent::ExecTool>());
  agent.register_tool(std::make_shared<oop_agent::FileTool>());
  agent.register_tool(std::make_shared<oop_agent::FileReadTool>());
  agent.register_tool(std::make_shared<oop_agent::FileWriteTool>());
  agent.register_tool(std::make_shared<oop_agent::WebSearchTool>());
  agent.register_tool(std::make_shared<oop_agent::MemoryTool>());
  agent.register_tool(std::make_shared<oop_agent::TimeTool>());
  agent.register_tool(std::make_shared<oop_agent::JsonTool>());
  agent.register_tool(std::make_shared<oop_agent::GitTool>());

  // 3. Nạp SkillLoader thật với đường dẫn động tương thích thư mục build
  std::string skills_path = "src/skills";
  if (!std::filesystem::exists(skills_path)) {
      skills_path = "../src/skills";
  }
  auto skill_loader = std::make_shared<oop_agent::SkillLoader>(skills_path);
  skill_loader->loadAll();
  agent.set_skill_loader(skill_loader);

  // 4. Setup HarnessRunner
  std::string tasks_path = "benchmark/tasks.json";
  std::string output_dir = "benchmark/results";
  {
    std::ifstream test_f(tasks_path);
    if (!test_f.is_open()) {
      tasks_path = "../benchmark/tasks.json";
      output_dir = "../benchmark/results";
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