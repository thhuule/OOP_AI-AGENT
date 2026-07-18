#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../client/llm_client.h"
#include "../tools/Tool.h"
#include "SkillLoader.h"

namespace oop_agent {

// Định nghĩa StepHook để thành viên C (Eval/Infra) đăng ký lắng nghe dữ liệu từng bước
using StepHook = std::function<void(const std::string&, 
                                     const std::string&, 
                                     const std::string&)>;

class AgentLoop {
public:
    // Sử dụng std::move để tối ưu hóa hiệu năng nạp client
    explicit AgentLoop(std::shared_ptr<LLMClient> client) : client_(std::move(client)) {}

    // Đăng ký các công cụ xử lý từ thành viên B
    void register_tool(std::shared_ptr<Tool> tool);

    // Tuần 5: Cung cấp API để thành viên C inject hook lưu vết trajectory
    void set_step_hook(StepHook hook) { step_hook_ = std::move(hook); }
    void set_skill_loader(std::shared_ptr<SkillLoader> loader) { skill_loader_ = std::move(loader); }

    // Vòng lặp điều hành ReAct chính thức
    std::string run(const std::string& instruction, int max_steps = 10);

private:
    std::shared_ptr<LLMClient> client_;
    std::shared_ptr<SkillLoader> skill_loader_ = nullptr;
    std::vector<std::shared_ptr<Tool>> tools_;
    std::shared_ptr<Tool> find_tool(std::string_view name);
    std::vector<Message> memory_; // Lưu trữ ngữ cảnh hội thoại (System prompt + History)
    StepHook step_hook_ = nullptr;
    bool parse_tool_call(const std::string& llm_text, std::string& tool_name, std::string& tool_args);
};

} // namespace oop_agent