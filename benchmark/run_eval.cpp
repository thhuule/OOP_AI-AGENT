#include "agent/agent_loop.h"
#include "client/gemini_client.h"
#include "client/ollama_client.h" // Giữ lại OllamaClient để đa hình
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
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    // -------------------------------------------------------------
    // 1. Đọc file config.json để chọn LLM Provider linh hoạt (Gemini/Ollama)
    // -------------------------------------------------------------
    std::string config_path = "config.json";
    if (!std::filesystem::exists(config_path)) {
        config_path = "../config.json";
    }

    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "[ERROR] Khong tim thay file config.json tai: " << config_path << std::endl;
        return 1;
    }

    json config_json;
    config_file >> config_json;

    std::shared_ptr<oop_agent::LLMClient> llm_client;
    std::string provider = config_json.value("provider", "gemini");

    if (provider == "gemini") {
        std::string api_key = config_json.value("api_key", "");
        std::string model = config_json.value("model", "gemini-2.5-flash");
        
        std::cout << "[INFO] Khoi tao GeminiClient voi model: " << model << std::endl;
        llm_client = std::make_shared<oop_agent::GeminiClient>(api_key, model);
    } else {
        std::string api_url = config_json.value("api_url", "http://localhost:11434");
        std::string model = config_json.value("model", "gemma4:e4b");
        
        std::cout << "[INFO] Khoi tao OllamaClient voi URL: " << api_url << std::endl;
        llm_client = std::make_shared<oop_agent::OllamaClient>(api_url, model);
    }

    // -------------------------------------------------------------
    // 2. Setup AgentLoop + Tools (Sử dụng client đa hình)
    // -------------------------------------------------------------
    oop_agent::AgentLoop agent(llm_client);
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

    // -------------------------------------------------------------
    // 3. Nạp SkillLoader thật
    // -------------------------------------------------------------
    std::string skills_path = "src/skills";
    if (!std::filesystem::exists(skills_path)) {
        skills_path = "../src/skills";
    }
    auto skill_loader = std::make_shared<oop_agent::SkillLoader>(skills_path);
    skill_loader->loadAll();
    agent.set_skill_loader(skill_loader);

    // -------------------------------------------------------------
    // 4. Setup HarnessRunner
    // -------------------------------------------------------------
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

    // -------------------------------------------------------------
    // 5. Inject StepHook & Chạy Benchmark
    // -------------------------------------------------------------
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);

    // 6. Chạy benchmark
    auto results = harness.runAll();
    harness.exportResults(results);

    return 0;
}
