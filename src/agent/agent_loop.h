#pragma once

#include <variant>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <expected>        // [C++23] Standard Expected for error handling

#include "client/llm_client.h"
#include "agent/LoopDetector.h"
#include "agent/SkillLoader.h"
#include "tools/ToolRegistry.h"
#include "tools/Tool.h"

namespace oop_agent {

// ── Action types ──────────────────────────────────────────────────────────────

struct ToolCallAction {
    std::string tool_name;
    std::string args;
};

struct FinalAnswerAction {
    std::string answer;
};

// ── TrajectoryStep (forward-declared for StepHook) ────────────────────────────

struct TrajectoryStep {
    int         step        = 0;
    std::string source      = "llm"; // "llm", "tool", "fixture"
    std::string thought;
    std::string action;
    std::string tool_name;
    std::string args;
    std::string result;
    bool        success     = false;
    double      latency_ms  = 0.0;
    int         tokens      = 0;   // NOTE: currently 0 — measurement gap
    int         tokens_used = 0;
};

// ── StepHook (Observer pattern) ───────────────────────────────────────────────

using StepHook = std::function<void(const TrajectoryStep&)>;

// ── AgentLoop ─────────────────────────────────────────────────────────────────

/// Implements the ReAct agent loop using Template Method pattern.
///
/// run() is the fixed skeleton: it calls protected virtual primitive operations
/// in a defined order. Subclasses may override individual steps (e.g. for
/// testing) without rewriting the entire loop.
///
/// Design patterns present:
///   - Template Method : run() skeleton + protected virtual steps
///   - Observer/Hook   : StepHook callback (AgentLoop does NOT include Harness)
///   - Strategy        : ToolRegistry uses pluggable Tool implementations
class AgentLoop {
public:
    explicit AgentLoop(std::shared_ptr<LLMClient> llm)
        : AgentLoop(std::move(llm), nullptr, ToolRegistry{}) {}

    AgentLoop(std::shared_ptr<LLMClient>   llm,
              std::shared_ptr<SkillLoader> skills,
              ToolRegistry                 registry);

    virtual ~AgentLoop() = default;

    /// Install observer hook. Called after each step with trajectory data.
    /// AgentLoop never includes HarnessRunner — hook is the only coupling.
    void set_step_hook(StepHook hook) { step_hook_ = std::move(hook); }

    void set_skill_loader(std::shared_ptr<SkillLoader> skills) {
        skills_ = std::move(skills);
    }

    void register_tool(std::shared_ptr<Tool> tool) {
        registry_.set_tool(std::move(tool));
    }

    void set_fallback_enabled(bool enabled) { fallback_enabled_ = enabled; }
    [[nodiscard]] bool is_fallback_enabled() const noexcept { return fallback_enabled_; }

    void set_config(LLMConfig config) { config_ = std::move(config); }
    [[nodiscard]] const LLMConfig& get_config() const noexcept { return config_; }

    // ── Template Method skeleton (non-virtual) ─────────────────────────────
    /// Fixed ReAct loop. Calls primitive operations in order.
    /// NOT virtual — subclasses override the primitives, not the skeleton.
    std::string run(const std::string& instruction, int max_steps);

protected:
    // ── Primitive operations (override in subclasses / tests) ──────────────

    /// Build the system prompt (skill injection).
    virtual std::string build_system_prompt(const std::string& instruction);

    /// Call LLM and parse response into an action.
    /// Returns variant or error string that is fed back as an observation.
    virtual std::variant<ToolCallAction, FinalAnswerAction>
    think_and_act(int step);

    /// Execute a tool call. Returns result or ToolError.
    virtual std::expected<std::string, std::string>
    execute_tool(const ToolCallAction& action);

    /// Append an observation to the conversation history.
    virtual void observe(const std::string& text);

    /// Called when LoopDetector fires.
    virtual void on_loop_detected();

    /// Called when max_steps is exhausted without a final answer.
    virtual void on_max_steps_reached();

    // ── Internal helpers (not overrideable primitives) ─────────────────────
    void emit_hook(const TrajectoryStep& step);

    // ── State ──────────────────────────────────────────────────────────────
    std::shared_ptr<LLMClient>   llm_;
    std::shared_ptr<SkillLoader> skills_;
    ToolRegistry                 registry_;
    LoopDetector                 detector_;
    StepHook                     step_hook_;
    std::vector<Message>         history_;
    LLMConfig                    config_;

    // Runtime state set during run() for primitives to read
    int         current_step_   = 0;
    std::string last_thought_;
    std::string current_instruction_;
    std::string last_fallback_result_;
    std::vector<ToolCallAction> fallback_plan_;
    std::size_t fallback_index_ = 0;
    bool        used_fallback_action_ = false;
    bool        fallback_enabled_     = false;
    bool        abort_          = false;
};

} // namespace oop_agent
