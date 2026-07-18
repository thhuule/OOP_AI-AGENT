#include "agent/SkillLoader.h"
#include "agent/agent_loop.h"
#include "client/ollama_client.h"
#include "harness/HarnessRunner.h"
#include "tools/CalculatorTool.h"
#include "tools/ExecTool.h"
#include "tools/FileTool.h"
#include <iostream>


int main() {
  // 1. Setup LLM client
  auto client = std::make_shared<oop_agent::OllamaClient>();

  // 2. Setup AgentLoop + tools
  oop_agent::AgentLoop agent(client);
  agent.register_tool(std::make_shared<oop_agent::ExecTool>());
  agent.register_tool(std::make_shared<oop_agent::FileTool>());
  agent.register_tool(std::make_shared<oop_agent::CalculatorTool>());

  // 3. Setup SkillLoader
  auto skill_loader = std::make_shared<oop_agent::SkillLoader>("../src/skills");
  skill_loader->loadAll();
  agent.set_skill_loader(skill_loader);

  // 4. Setup HarnessRunner
  oop_agent::HarnessRunner harness("benchmark/tasks.json");
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