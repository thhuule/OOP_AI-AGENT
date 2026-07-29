#pragma once

#include <variant>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <expected>        // [C++23] Standard Expected for error handling
#include <inplace_vector>  // [C++26] Fixed-capacity stack vector

#include "client/llm_client.h"
#include "agent/LoopDetector.h"
#include "agent/SkillLoader.h"
#include "tools/ToolRegistry.h"
#include "tools/Tool.h"

namespace oop_agent {

using StepHook = std::function<void(const std::string& thought, const std::string& action, const std::string& result)>;

struct ToolCallAction {
    std::string tool_name;
    std::string args;
};

struct FinalAnswerAction {
    std::string content;
};

using Action = std::variant<ToolCallAction, FinalAnswerAction>;

class AgentLoop {
public:
    explicit AgentLoop(std::shared_ptr<LLMClient> client) : client_(std::move(client)) {}
    ~AgentLoop() = default;

    // Register Tool bằng unique_ptr
    void register_tool(std::unique_ptr<Tool> tool) {
        tools_.register_tool(std::move(tool));
    }

    // Register Tool bằng shared_ptr (Adapter Wrapper Pattern)
    template <typename T>
    void register_tool(std::shared_ptr<T> tool) {
        struct SharedToolWrapper : public Tool {
            std::shared_ptr<T> tool_;
            explicit SharedToolWrapper(std::shared_ptr<T> t) : tool_(std::move(t)) {}
            [[nodiscard]] std::string_view get_name() const noexcept override { return tool_->get_name(); }
            [[nodiscard]] std::string_view get_description() const noexcept override { return tool_->get_description(); }
            
            // [C++23] std::expected return type
            std::expected<std::string, ToolError> execute(const std::string& args) override { 
                return tool_->execute(args); 
            }
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

    Action parse_llm_response(const std::string& llm_text);
    void truncate_history(std::vector<Message>& history, size_t max_messages = 10);
};

} // namespace oop_agent