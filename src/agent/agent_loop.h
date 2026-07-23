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
        // Chuyển shared_ptr thành unique_ptr bằng cách wrap/thực thi qua ToolRegistry nếu cần
        // Hoặc đơn giản tạo unique_ptr mới trỏ tới phiên bản kế thừa Tool
        tools_.register_tool(std::unique_ptr<Tool>(tool.get())); 
        // LƯU Ý: Nếu ToolRegistry quản lý quyền sở hữu unique_ptr, 
        // cách an toàn nhất để khớp cả 2 là khởi tạo unique_ptr trực tiếp:
    }

    void set_skill_loader(std::shared_ptr<SkillLoader> loader) { skill_loader_ = std::move(loader); }
    void set_step_hook(StepHook hook) { step_hook_ = std::move(hook); }

    std::string run(const std::string& user_instruction, int max_steps = 10);

private:
    std::shared_ptr<LLMClient> client_;
    ToolRegistry tools_;
    std::shared_ptr<SkillLoader> skill_loader_;
    StepHook step_hook_;
    
    LoopDetector loop_detector_;
};

} // namespace oop_agent