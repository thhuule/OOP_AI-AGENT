#include "agent/agent_loop.h"
#include "client/gemini_client.h"
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
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

// ── Find repo root (contains benchmark/tasks.json) ────────────────────────────
static fs::path findRepoRoot() {
    fs::path cwd = fs::current_path();
    // Walk up until we find benchmark/tasks.json
    for (fs::path p = cwd; p != p.parent_path(); p = p.parent_path()) {
        if (fs::exists(p / "benchmark" / "tasks.json"))
            return p;
    }
    return cwd; // fallback
}

int main() {
    // ── 0. Change to repo root so relative paths (hello.sh, notes.txt …) work ──
    const fs::path repo_root = findRepoRoot();
    std::error_code ec;
    fs::current_path(repo_root, ec);
    if (ec) {
        std::cerr << "[WARN] Could not chdir to repo root: " << ec.message() << "\n";
    } else {
        std::cout << "[INFO] Working directory: " << fs::current_path().string() << "\n";
    }

    // ── 1. Read config.json ───────────────────────────────────────────────────
    fs::path config_path = repo_root / "config.json";
    if (!fs::exists(config_path))
        config_path = "config.json";

    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "[ERROR] Cannot open config.json at: "
                  << config_path.string() << "\n";
        return 1;
    }

    json config_json;
    config_file >> config_json;

    std::shared_ptr<oop_agent::LLMClient> llm_client;
    std::string provider = config_json.value("provider", "gemini");

    if (provider == "gemini") {
        std::string api_key = config_json.value("api_key", "");
        std::string model   = config_json.value("model", "gemini-2.5-flash");
        std::cout << "[INFO] GeminiClient model: " << model << "\n";
        llm_client = std::make_shared<oop_agent::GeminiClient>(api_key, model);
    } else {
        std::string api_url = config_json.value("api_url", "http://localhost:11434");
        std::string model   = config_json.value("model", "gemma4:e4b");
        std::cout << "[INFO] OllamaClient URL: " << api_url << "\n";
        llm_client = std::make_shared<oop_agent::OllamaClient>(api_url, model);
    }

    // ── 2. Setup AgentLoop + all Tools ───────────────────────────────────────
    oop_agent::AgentLoop agent(llm_client);
    agent.register_tool(std::make_shared<oop_agent::CalculatorTool>());
    agent.register_tool(std::make_shared<oop_agent::ExecTool>());
    agent.register_tool(std::make_shared<oop_agent::FileTool>());
    agent.register_tool(std::make_shared<oop_agent::FileReadTool>());
    agent.register_tool(std::make_shared<oop_agent::FileWriteTool>());
    agent.register_tool(std::make_shared<oop_agent::FileAppendTool>());
    agent.register_tool(std::make_shared<oop_agent::WebSearchTool>());
    agent.register_tool(std::make_shared<oop_agent::MemoryTool>());
    agent.register_tool(std::make_shared<oop_agent::TimeTool>());
    agent.register_tool(std::make_shared<oop_agent::JsonTool>());
    agent.register_tool(std::make_shared<oop_agent::GitTool>());

    // ── 3. SkillLoader ────────────────────────────────────────────────────────
    fs::path skills_dir = repo_root / "skills";
    if (!fs::exists(skills_dir))
        skills_dir = repo_root / "src" / "skills";

    auto skill_loader = std::make_shared<oop_agent::SkillLoader>(skills_dir.string());
    skill_loader->loadAll();
    agent.set_skill_loader(skill_loader);

    // ── 4. HarnessRunner ──────────────────────────────────────────────────────
    const std::string tasks_path  = (repo_root / "benchmark" / "tasks.json").string();
    const std::string output_dir  = (repo_root / "benchmark" / "results").string();

    oop_agent::HarnessRunner harness(tasks_path, output_dir);
    if (!harness.loadTasks()) {
        std::cerr << "[ERROR] Cannot load benchmark tasks from: " << tasks_path << "\n";
        return 1;
    }

    // ── 5. Run ────────────────────────────────────────────────────────────────
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);

    auto results = harness.runAll();
    if (!harness.exportResults(results)) {
        std::cerr << "[ERROR] Cannot export benchmark results.\n";
        return 1;
    }

    return 0;
}