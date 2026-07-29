#include "agent/agent_loop.h"
#include "tools/Tool.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <print>
#include <regex>
#include <string_view>
#include <thread>
#include <type_traits>
#include <variant>
#include <inplace_vector>

namespace oop_agent {

constexpr std::string_view trim_sv(std::string_view sv) noexcept {
    auto start = sv.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return "";
    auto end = sv.find_last_not_of(" \t\n\r");
    return sv.substr(start, end - start + 1);
}

void AgentLoop::truncate_history(std::vector<Message>& history, size_t max_messages) {
    if (history.empty()) return;
    if (history.size() <= max_messages) return;

    constexpr size_t kHistoryCapacity = 12;
    const size_t effective_max_messages = std::min(max_messages, kHistoryCapacity);

    std::inplace_vector<Message, kHistoryCapacity> truncated;
    truncated.push_back(history.front());

    size_t start_idx = history.size() - (effective_max_messages - 1);
    for (size_t i = start_idx; i < history.size(); ++i) {
        truncated.push_back(history[i]);
    }
    history.assign(truncated.begin(), truncated.end());
}

Action AgentLoop::parse_llm_response(const std::string& llm_text) {
    std::string clean_text = llm_text;

    std::regex md_json_regex(R"(```(?:json)?\s*(\{[\s\S]*?\})\s*```)", std::regex::icase);
    std::smatch match;
    if (std::regex_search(llm_text, match, md_json_regex)) {
        clean_text = match[1].str();
    }

    // 1. Dùng Regex cải tiến để khớp cả Object và String args
    std::regex tool_json_regex(R"delim(\{\s*"tool"\s*:\s*"([^"]+)"\s*,\s*"args"\s*:\s*(\{[\s\S]*?\}|"[^"]*")\s*\})delim");
    if (std::regex_search(clean_text, match, tool_json_regex)) {
        std::string tool_name = match[1].str();
        std::string args = match[2].str();
        return ToolCallAction{tool_name, args};
    }

    size_t action_pos = clean_text.find("ACTION:");
    if (action_pos != std::string::npos) {
        std::string_view action_str = clean_text;
        action_str = action_str.substr(action_pos + 7);
        size_t open = action_str.find("(");
        size_t close = action_str.rfind(")");

        if (open != std::string_view::npos && close != std::string_view::npos && close > open) {
            std::string_view raw_tool_name = action_str.substr(0, open);
            std::string_view clean_tool_name = trim_sv(raw_tool_name);
            std::string_view args_view = action_str.substr(open + 1, close - open - 1);

            return ToolCallAction{std::string(clean_tool_name), std::string(args_view)};
        }
    }

    return FinalAnswerAction{llm_text};
}

std::string AgentLoop::run(const std::string& user_instruction, int max_steps) {
    loop_detector_.reset();

    std::inplace_vector<std::string, 10> recent_tool_calls;

    std::vector<Message> conversation_history;

    // 2. Tối ưu System Prompt cho Gemini
   std::string system_prompt = 
        "You are an AI Agent equipped with tools to solve tasks.\n"
        "CRITICAL RULES:\n"
        "1. When calling a tool, respond ONLY with standard JSON format:\n"
        "   {\"tool\": \"tool_name\", \"args\": \"arguments\"}\n"
        "2. STRICT FINAL ANSWER REQUIREMENT:\n"
        "   When you finish your task, your final textual response MUST directly include:\n"
        "   - Every exact filename, extension (.cpp, .h), or string content returned by tools.\n"
        "   - The literal word 'PASS' if any script or test execution output contains it.\n"
        "   NEVER summarize in your own words. Copy the exact raw outputs and keywords literally.";
    conversation_history.push_back({"system", system_prompt, std::nullopt});
    conversation_history.push_back({"user", user_instruction, std::nullopt});

    for (int step = 0; step < max_steps; ++step) {
        truncate_history(conversation_history, 12);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto response_result = client_->generate_chat(conversation_history);
        if (!response_result.has_value()) {
            std::println(std::cerr, "[AgentLoop] LLM Client Error at step {}!", step + 1);
            break;
        }

        std::string llm_text = response_result.value();
        conversation_history.push_back({"assistant", llm_text, std::nullopt});
        Action action = parse_llm_response(llm_text);

        bool finished = false;
        std::string final_answer;

        std::visit([&](auto&& act) {
            using T = std::decay_t<decltype(act)>;

            if constexpr (std::is_same_v<T, ToolCallAction>) {
                if (recent_tool_calls.size() == recent_tool_calls.capacity()) {
                    recent_tool_calls.erase(recent_tool_calls.begin());
                }
                recent_tool_calls.push_back(act.tool_name);

                auto loop_status = loop_detector_.add_action(act.tool_name);

                if (loop_status == LoopDetector::Status::Warning) {
                    std::println(std::cerr, "[LoopDetector][WARNING] Possible loop detected for tool: '{}'", act.tool_name);
                } else if (loop_status == LoopDetector::Status::Critical) {
                    std::println(std::cerr, "[LoopDetector][CRITICAL] Infinite loop detected for tool: '{}'! Stopping Agent Loop.", act.tool_name);
                    conversation_history.push_back({"user", "System Error: Infinite loop detected. Stop calling this tool.", std::nullopt});
                    finished = true;
                    final_answer = "Agent stopped due to infinite loop detection.";
                    return;
                }

                if (!tools_.is_allowed(act.tool_name)) {
                    conversation_history.push_back({"user", "Error: Tool is not allowed: " + act.tool_name, std::nullopt});
                    return;
                }

                Tool* tool = tools_.get_tool(act.tool_name);
                if (tool) {
                    auto res = tool->execute(act.args);
                    std::string result = res.has_value() ? res.value() : "Tool Execution Error";

                    if (step_hook_) {
                        step_hook_(llm_text, act.tool_name, result);
                    }

                    // 3. Chèn SYSTEM NOTE để duy trì keyword trong Final Answer
                    std::string formatted_result = result + 
                        "\n\n[SYSTEM NOTE]: Include all raw output details, file names, extensions (.cpp, .h), or exact text from this tool result in your final answer.";

                    conversation_history.push_back({"tool", formatted_result, std::nullopt});
                } else {
                    conversation_history.push_back({"user", "Error: Tool not found: " + act.tool_name, std::nullopt});
                }
            } else if constexpr (std::is_same_v<T, FinalAnswerAction>) {
                finished = true;
                final_answer = act.content;
            }
        }, action);

        if (finished) {
            return final_answer;
        }
    }

    return "Agent reached maximum step limit or stopped due to loop.";
}

} // namespace oop_agent