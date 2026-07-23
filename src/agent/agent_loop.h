#pragma once

#include "client/llm_client.h"
#include "agent/LoopDetector.h"
#include "agent/SkillLoader.h"
#include "tools/ToolRegistry.h"
#include "tools/Tool.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace oop_agent {

using StepHook = std::function<void(const std::string& thought, const std::string& action, const std::string& result)>;

class AgentLoop {
public:
    explicit AgentLoop(std::shared_ptr<LLMClient> client) : client_(std::move(client)) {}
    ~AgentLoop() = default;

    // 1. Dành cho std::unique_ptr (như ToolRegistry yêu cầu)
    void register_tool(std::unique_ptr<Tool> tool) {
        tools_.register_tool(std::move(tool));
    }

    // 2. Dành cho std::shared_ptr (như run_eval.cpp đang gọi)
    template <typename T>
    void register_tool(std::shared_ptr<T> tool) {
        struct SharedToolWrapper : public Tool {
            std::shared_ptr<T> tool_;
            explicit SharedToolWrapper(std::shared_ptr<T> t) : tool_(std::move(t)) {}
            [[nodiscard]] std::string_view get_name() const noexcept override { return tool_->get_name(); }
            [[nodiscard]] std::string_view get_description() const noexcept override { return tool_->get_description(); }
            std::expected<std::string, ToolError> execute(const std::string& args) override { return tool_->execute(args); }
        };
        tools_.register_tool(std::make_unique<SharedToolWrapper>(std::move(tool)));
    }

    void set_skill_loader(std::shared_ptr<SkillLoader> loader) { skill_loader_ = std::move(loader); }
    void set_step_hook(StepHook hook) { step_hook_ = std::move(hook); }

    std::string run(const std::string& user_instruction, int max_steps = 10);

private:
    std::shared_ptr<LLMClient> client_;
    ToolRegistry tools_;
    std::shared_ptr<SkillLoader> skill_loader_ = nullptr;
    StepHook step_hook_ = nullptr;
    
    LoopDetector loop_detector_;
};

} // namespace oop_agent