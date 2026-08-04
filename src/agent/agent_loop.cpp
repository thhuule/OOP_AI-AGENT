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

AgentLoop::AgentLoop(std::shared_ptr<LLMClient>   llm,
                     std::shared_ptr<SkillLoader> skills,
                     ToolRegistry                 registry)
    : llm_(std::move(llm))
    , skills_(std::move(skills))
    , registry_(std::move(registry))
{}

// ── Template Method skeleton ──────────────────────────────────────────────────

std::string AgentLoop::run(const std::string& instruction, int max_steps) {
    abort_  = false;
    history_.clear();
    detector_.reset();

    // ── Setup ──────────────────────────────────────────────────────────────
    history_.push_back({"system", build_system_prompt(instruction), std::nullopt});
    history_.push_back({"user",   instruction, std::nullopt});

    // ── ReAct loop ─────────────────────────────────────────────────────────
    for (int step = 1; step <= max_steps; ++step) {
        current_step_ = step;

        // THINK + ACT
        auto action = think_and_act(step);

        if (abort_) return "Aborted at step " + std::to_string(step);

        // FINAL ANSWER branch
        if (auto* fa = std::get_if<FinalAnswerAction>(&action)) {
            TrajectoryStep ts;
            ts.step      = step;
            ts.thought     = last_thought_;
            ts.action      = "final_answer";
            ts.tool_name   = "final_answer";
            ts.args        = "";
            ts.result      = fa->answer;
            ts.success     = true;
            ts.tokens_used = 0;
            emit_hook(ts);
            return fa->answer;
        }

        // TOOL CALL branch
        auto& tc = std::get<ToolCallAction>(action);

        auto t_start = std::chrono::steady_clock::now();
        auto result  = execute_tool(tc);
        auto t_end   = std::chrono::steady_clock::now();
        double latency = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        TrajectoryStep ts;
        ts.step      = step;
        ts.thought     = last_thought_;
        ts.action      = tc.tool_name;
        ts.tool_name   = tc.tool_name;
        ts.args        = tc.args;
        ts.latency_ms  = latency;
        ts.tokens_used = 0;

        if (result) {
            ts.result  = *result;
            ts.success = true;
            observe(*result);
        } else {
            ts.result  = "TOOL_ERROR: " + result.error();
            ts.success = false;
            observe("TOOL_ERROR: " + result.error());
        }
        emit_hook(ts);

        // LOOP DETECTION (Sử dụng đúng API Status từ LoopDetector)
        auto status = detector_.add_action(tc.tool_name);
        if (status == LoopDetector::Status::Critical) {
            on_loop_detected();
            return "Loop detected — aborting";
        }

        if (abort_) return "Aborted at step " + std::to_string(step);
    }

    // MAX STEPS
    on_max_steps_reached();
    return "Max steps reached";
}

// ── Primitive operations (default implementations) ────────────────────────────

std::string AgentLoop::build_system_prompt(const std::string& /*instruction*/) {
    std::string skill_content = skills_ ? skills_->getSystemPrompt() : "";
    return "You are a helpful AI agent with tool-use capabilities.\n\n"
           "Available tools: use JSON format {\"tool\":\"name\",\"args\":\"...\"} or "
           "reply with Final Answer.\n\n" + skill_content;
}

std::variant<ToolCallAction, FinalAnswerAction>
AgentLoop::think_and_act(int /*step*/) {
    // Call LLM
    auto response = llm_->generate_chat(history_);
    if (!response) {
        last_thought_ = "";
        observe("LLM_ERROR: LLM generation failed");
        abort_ = true;
        return FinalAnswerAction{"LLM error encountered."};
    }

    const std::string& text = *response;
    last_thought_ = text;

    // Parse response for Action / Final Answer
    size_t action_pos = text.find("ACTION:");
    if (action_pos != std::string::npos) {
        std::string_view action_str = text;
        action_str = action_str.substr(action_pos + 7);
        size_t open = action_str.find("(");
        size_t close = action_str.rfind(")");

        if (open != std::string_view::npos && close != std::string_view::npos && close > open) {
            std::string_view raw_tool_name = action_str.substr(0, open);
            std::string_view args_view = action_str.substr(open + 1, close - open - 1);
            return ToolCallAction{std::string(raw_tool_name), std::string(args_view)};
        }
    }

    if (text.find("Final Answer:") != std::string::npos) {
        auto pos = text.find("Final Answer:") + 13;
        return FinalAnswerAction{text.substr(pos)};
    }

    return FinalAnswerAction{text};
}

std::expected<std::string, std::string>
AgentLoop::execute_tool(const ToolCallAction& action) {
    Tool* tool = registry_.lookup(action.tool_name);
    if (!tool) {
        return std::unexpected("TOOL_NOT_FOUND: " + action.tool_name);
    }
    auto res = tool->execute(action.args);
    if (!res) {
        return std::unexpected("Tool execution error");
    }
    return *res;
}

void AgentLoop::observe(const std::string& text) {
    history_.push_back({"user", "Observation: " + text, std::nullopt});
}

void AgentLoop::on_loop_detected() {
    std::println("[AgentLoop] Loop detected at step {}. Aborting.", current_step_);
}

void AgentLoop::on_max_steps_reached() {
    std::println("[AgentLoop] Max steps reached at step {}.", current_step_);
}

// ── Helper ────────────────────────────────────────────────────────────────────

void AgentLoop::emit_hook(const TrajectoryStep& ts) {
    if (step_hook_) step_hook_(ts);
}

} // namespace oop_agent