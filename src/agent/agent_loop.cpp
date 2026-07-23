#include "agent/agent_loop.h"
#include "tools/Tool.h"
#include <iostream>

namespace oop_agent {

std::string AgentLoop::run(const std::string& user_instruction, int max_steps) {
    loop_detector_.reset();

    std::vector<Message> conversation_history;
    
    // ToolRegistry không có phương thức get_tool_descriptions(), nên dùng Prompt tĩnh/đơn giản
    std::string system_prompt = "You are a helpful AI Agent with access to tools.";

    conversation_history.push_back({"system", system_prompt, {}});
    conversation_history.push_back({"user", user_instruction, {}});

    for (int step = 0; step < max_steps; ++step) {
        auto response_result = client_->generate_chat(conversation_history);
        if (!response_result.has_value()) {
            std::cerr << "[AgentLoop] LLM Client Error!\n";
            break;
        }

        std::string llm_text = response_result.value();
        conversation_history.push_back({"assistant", llm_text, {}});

        std::string tool_name = ""; 
        std::string tool_args = ""; 
        bool is_final_answer = false;

        size_t action_pos = llm_text.find("ACTION:");
        if (action_pos != std::string::npos) {
            std::string action_str = llm_text.substr(action_pos + 7);
            size_t open_paren = action_str.find("(");
            size_t close_paren = action_str.find(")");
            if (open_paren != std::string::npos && close_paren != std::string::npos) {
                tool_name = action_str.substr(0, open_paren);
                tool_name.erase(0, tool_name.find_first_not_of(" \t\n\r"));
                tool_name.erase(tool_name.find_last_not_of(" \t\n\r") + 1);
                
                tool_args = action_str.substr(open_paren + 1, close_paren - open_paren - 1);
            }
        } else {
            is_final_answer = true;
        }

        // --- LOOP DETECTOR & TOOL EXECUTION ---
        if (!is_final_answer && !tool_name.empty()) {
            auto loop_status = loop_detector_.add_action(tool_name);

            if (loop_status == LoopDetector::Status::Warning) {
                std::cerr << "[AgentLoop] Warning: Loop detected for tool '" << tool_name << "'!\n";
            } else if (loop_status == LoopDetector::Status::Critical) {
                std::cerr << "[AgentLoop] Critical: Stopping agent due to infinite loop on tool '" << tool_name << "'!\n";
                conversation_history.push_back({"user", "System Error: Infinite loop detected. Task aborted.", {}});
                break;
            }

            // Kiểm tra xem Tool có được phép thực thi không trước khi gọi get_tool
            if (!tools_.is_allowed(tool_name)) {
                conversation_history.push_back({"user", "Error: Tool is not allowed: " + tool_name, {}});
                continue;
            }

            // get_tool trả về con trỏ thô Tool*
            Tool* tool = tools_.get_tool(tool_name);
            if (tool) {
                auto tool_res = tool->execute(tool_args);
                std::string result_str = tool_res.has_value() ? tool_res.value() : "Tool Execution Error";
                
                if (step_hook_) {
                    step_hook_(llm_text, tool_name, result_str);
                }

                conversation_history.push_back({"tool", result_str, {}});
            } else {
                conversation_history.push_back({"user", "Error: Tool not found: " + tool_name, {}});
            }
        } else {
            return llm_text;
        }
    }

    return "Agent reached maximum step limit or stopped due to loop.";
}

} // namespace oop_agent