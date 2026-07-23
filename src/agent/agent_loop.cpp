#include "agent/agent_loop.h"
#include "tools/Tool.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <variant>      // THÊM DÒNG NÀY
#include <type_traits>  // THÊM DÒNG NÀY

namespace oop_agent {
Action AgentLoop::parse_llm_response(const std::string& llm_text)
{
    size_t action_pos = llm_text.find("ACTION:");

    if (action_pos == std::string::npos)
        return FinalAnswerAction{llm_text};

    std::string action_str = llm_text.substr(action_pos + 7);

    size_t open = action_str.find("(");
    size_t close = action_str.find(")");

    if (open == std::string::npos || close == std::string::npos)
        return FinalAnswerAction{llm_text};

    std::string tool_name = action_str.substr(0, open);

    tool_name.erase(0, tool_name.find_first_not_of(" \t\n\r"));
    tool_name.erase(tool_name.find_last_not_of(" \t\n\r") + 1);

    std::string args =
        action_str.substr(open + 1, close - open - 1);

    return ToolCallAction{tool_name, args};
}

std::string AgentLoop::run(const std::string& user_instruction, int max_steps) {
    loop_detector_.reset();

    std::vector<Message> conversation_history;
    
    // ToolRegistry không có phương thức get_tool_descriptions(), nên dùng Prompt tĩnh/đơn giản
    std::string system_prompt = "You are a helpful AI Agent with access to tools.";

    conversation_history.push_back({"system", system_prompt, {}});
    conversation_history.push_back({"user", user_instruction, {}});

    for (int step = 0; step < max_steps; ++step) {
        // Nghỉ 1 giây giữa các bước để đảm bảo tốc độ dưới 60 RPM
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto response_result = client_->generate_chat(conversation_history);
        if (!response_result.has_value()) {
            std::cerr << "[AgentLoop] LLM Client Error!\n";
            break;
        }

        std::string llm_text = response_result.value();
        conversation_history.push_back({"assistant", llm_text, {}});
        Action action = parse_llm_response(llm_text);

        // --- LOOP DETECTOR & TOOL EXECUTION ---
        bool finished = false;
        std::string final_answer;

        std::visit([&](auto&& act)
        {
            using T = std::decay_t<decltype(act)>;

            if constexpr(std::is_same_v<T, ToolCallAction>)
            {
                auto loop_status =
                    loop_detector_.add_action(act.tool_name);

                if(loop_status == LoopDetector::Status::Warning)
                {
                    std::cerr << "[AgentLoop] Warning: Loop detected!\n";
                }
                else if(loop_status == LoopDetector::Status::Critical)
                {
                    std::cerr << "[AgentLoop] Critical: Infinite loop!\n";

                    conversation_history.push_back({
                        "user",
                        "System Error: Infinite loop detected.",
                        {}
                    });

                    finished = true;
                    return;
                }

                if(!tools_.is_allowed(act.tool_name))
                {
                    conversation_history.push_back({
                        "user",
                        "Error: Tool is not allowed: " + act.tool_name,
                        {}
                    });
                    return;
                }

                Tool* tool = tools_.get_tool(act.tool_name);

                if(tool)
                {
                    auto res = tool->execute(act.args);

                    std::string result =
                        res.has_value()
                        ? res.value()
                        : "Tool Execution Error";

                    if(step_hook_)
                        step_hook_(llm_text, act.tool_name, result);

                    conversation_history.push_back({
                        "tool",
                        result,
                        {}
                    });
                }
                else
                {
                    conversation_history.push_back({
                        "user",
                        "Error: Tool not found: " + act.tool_name,
                        {}
                    });
                }
            }
            else if constexpr(std::is_same_v<T, FinalAnswerAction>)
            {
                finished = true;
                final_answer = act.content;
            }

        }, action);

        if(finished){
            return final_answer; 
            }
    }

    return "Agent reached maximum step limit or stopped due to loop.";
}

} // namespace oop_agent