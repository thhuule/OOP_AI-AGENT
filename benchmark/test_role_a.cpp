// Role A (Systems / Core) focused test suite — Tuần 10 plan (KH_Tuan10_ChiTiet §5).
//
// Covers the components owned by Role A:
//   - LLMClient abstract interface + polymorphism (Strategy via shared_ptr)
//   - Client error contract (5 LLMError -> classified reason)
//   - Multimodal interface (text + optional images on the same Message)
//   - Parser variants (raw / fenced JSON, ACTION:, functionCall, call:provider:tool)
//   - Malformed tool-intent is treated as a tool call, never a final answer
//   - LoopDetector (generic-repeat + ping-pong, warning/critical)
//   - SkillLoader (.md scan / inject before every run)
//   - AgentLoop Template Method skeleton + primitive overrides:
//       * normal call sequence
//       * loop detection path
//       * max-steps path + history growth
//       * deterministic fallback-plan path executing REAL tools
//   - Observer/Hook (StepHook) and Registry/Factory/Strategy (ToolRegistry)
//   - C++ feature matrix (C++17/20/23/26 + portable inplace_vector fallback)
//   - NativeEnvironment (Environment abstraction)
//
// No network / no real LLM required: everything is driven by mocks that
// return std::expected values, keeping the test deterministic and fast.

#include "agent/agent_loop.h"
#include "agent/LoopDetector.h"
#include "agent/SkillLoader.h"
#include "client/llm_client.h"
#include "tools/ToolRegistry.h"
#include "tools/CalculatorTool.h"
#include "environment/Environment.h"
#include "environment/NativeEnvironment.h"

#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <vector>
#include <variant>

using namespace oop_agent;
namespace fs = std::filesystem;

// ── Tiny assertion helper ──────────────────────────────────────────────────────
static int g_failures = 0;

static bool check(bool cond, const std::string& msg) {
    if (cond) {
        std::cout << "  [OK] " << msg << "\n";
    } else {
        std::cerr << "  [FAIL] " << msg << "\n";
        ++g_failures;
    }
    return cond;
}

// ── Mock LLM client (satisfies LLMClient, no network) ──────────────────────────
class MockLLMClient : public LLMClient {
public:
    explicit MockLLMClient(std::string reply) : reply_(std::move(reply)) {}

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& history = {},
        const LLMConfig& = {}) override {
        last_history_ = history;
        if (fail_with_.has_value())
            return std::unexpected(*fail_with_);
        return reply_;
    }

    void set_reply(std::string r) { reply_ = std::move(r); }
    void set_failure(LLMError e) { fail_with_ = e; }
    const std::vector<Message>& last_history() const { return last_history_; }

private:
    std::string reply_;
    std::optional<LLMError> fail_with_;
    std::vector<Message> last_history_;
};

// Instruction that matches NO fallback keyword (so think_and_act goes to the LLM).
static const char* kNoFallback = "perform a generic classification task now";

// ── 1. testClientErrorContract ────────────────────────────────────────────────
void testClientErrorContract() {
    std::cout << "[TEST] testClientErrorContract\n";

    struct Case { LLMError err; std::string klass; };
    Case cases[] = {
        {LLMError::ConnectionRefused, "Connection refused"},
        {LLMError::Timeout,           "Timeout"},
        {LLMError::MalformedJSON,     "Malformed JSON"},
        {LLMError::RateLimit,         "Rate limit"},
        {LLMError::UnknownError,      "Unknown error"},
    };
    for (const auto& c : cases) {
        auto client = std::make_shared<MockLLMClient>("");
        client->set_failure(c.err);
        AgentLoop agent(client);
        std::string result = agent.run(kNoFallback, 3);
        check(result.find("LLM error:") == 0,
              "error " + c.klass + " -> reason starts with 'LLM error:'");
        check(!result.empty() && result.find(c.klass) != std::string::npos,
              "error " + c.klass + " -> reason is non-empty and classified");
    }
}

// ── 2. testMultimodalInterface ────────────────────────────────────────────────
void testMultimodalInterface() {
    std::cout << "[TEST] testMultimodalInterface\n";

    // A single Message can carry text AND images through the same LLMClient.
    Message m{"user", "describe the screenshot",
              std::vector<std::string>{"data:image/png;base64,AAAA", "data:image/png;base64,BBBB"}};
    auto client = std::make_shared<MockLLMClient>("Final Answer: ok");
    auto r = client->generate_chat({m});
    check(r.has_value() && *r == "Final Answer: ok", "multimodal client returns expected");
    check(client->last_history().size() == 1, "one message forwarded to client");
    const auto& imgs = client->last_history()[0].images;
    check(imgs.has_value() && imgs->size() == 2,
          "images preserved on the same Message (text+image one interface)");
}

// ── 3. testParserVariants ─────────────────────────────────────────────────────
class CaptureAgent : public AgentLoop {
public:
    explicit CaptureAgent(std::shared_ptr<LLMClient> llm) : AgentLoop(std::move(llm)) {}
    std::vector<ToolCallAction> captured;
protected:
    std::expected<std::string, std::string> execute_tool(const ToolCallAction& a) override {
        captured.push_back(a);
        return "ok";
    }
};

void testParserVariants() {
    std::cout << "[TEST] testParserVariants\n";

    const char* cases[][3] = {
        // reply, expected tool, expected args
        {R"({"tool":"calculator","args":"1+1"})",                       "calculator", "1+1"},
        {"```json\n{\"tool\":\"write_file\",\"args\":\"a.txt,x\"}\n```", "write_file", "a.txt,x"},
        {"ACTION: calculator(2+2)",                                     "calculator", "2+2"},
        {R"({"functionCall":{"name":"calculator","args":"3+3"}})",       "calculator", "3+3"},
        {"call:gemini:calculator{4+4}",                                 "calculator", "4+4"},
    };
    for (const auto& c : cases) {
        auto client = std::make_shared<MockLLMClient>(c[0]);
        CaptureAgent agent(client);
        agent.run(kNoFallback, 1);
        bool ok = agent.captured.size() == 1 &&
                  agent.captured[0].tool_name == c[1] &&
                  agent.captured[0].args == c[2];
        check(ok, std::string("parse '") + c[0] + "' -> " + c[1] + "(" + c[2] + ")");
    }
}

// ── 4. testMalformedToolIntentNotFinalAnswer ──────────────────────────────────
void testMalformedToolIntentNotFinalAnswer() {
    std::cout << "[TEST] testMalformedToolIntentNotFinalAnswer\n";

    // Model emits a valid tool-call protocol but names an UNKNOWN tool.
    // Agent must attempt a tool call (execute_tool invoked), never treat the
    // raw text as a final answer.
    auto client = std::make_shared<MockLLMClient>(R"({"tool":"ghost_tool","args":"x"})");
    CaptureAgent agent(client);
    std::string result = agent.run(kNoFallback, 1);

    check(!agent.captured.empty() && agent.captured[0].tool_name == "ghost_tool",
          "unknown-tool protocol still drives a tool call, not a final answer");
    check(result.find("ghost_tool") == std::string::npos,
          "raw JSON was NOT returned as the final answer");
}

// ── 5. testMaxStepsAndHistoryGrowth ───────────────────────────────────────────
class ProbeAgent : public AgentLoop {
public:
    explicit ProbeAgent(std::shared_ptr<LLMClient> llm) : AgentLoop(std::move(llm)) {}
    std::size_t history_size() const { return history_.size(); }
    bool max_reached = false;
protected:
    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int) override {
        return ToolCallAction{"calculator", "1+1"};
    }
    std::expected<std::string, std::string> execute_tool(const ToolCallAction&) override {
        return "2";
    }
    void on_max_steps_reached() override {
        max_reached = true;
        AgentLoop::on_max_steps_reached();
    }
};

void testMaxStepsAndHistoryGrowth() {
    std::cout << "[TEST] testMaxStepsAndHistoryGrowth\n";

    auto client = std::make_shared<MockLLMClient>("Final Answer: never");
    ProbeAgent agent(client);
    agent.run(kNoFallback, 3);

    check(agent.max_reached, "on_max_steps_reached invoked after max_steps");
    // history starts with [system, user], each step appends one observation.
    check(agent.history_size() >= 5,
          "conversation history grows across steps (>= 5 entries after 3 steps)");
}

// ── 6. testSkillInjectionBeforeEachRun ────────────────────────────────────────
class SkillProbe : public AgentLoop {
public:
    using AgentLoop::AgentLoop;
    int build_count = 0;
    std::string last_prompt;
protected:
    std::string build_system_prompt(const std::string& instruction) override {
        ++build_count;
        last_prompt = AgentLoop::build_system_prompt(instruction);
        return last_prompt;
    }
    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int) override {
        return FinalAnswerAction{"done"};
    }
};

void testSkillInjectionBeforeEachRun() {
    std::cout << "[TEST] testSkillInjectionBeforeEachRun\n";

    const fs::path dir = fs::temp_directory_path() / "role_a_skills_inject";
    fs::remove_all(dir);
    fs::create_directories(dir);
    std::ofstream(dir / "planner.md")   << "# Planner\nBreak into steps.";
    std::ofstream(dir / "reviewer.md")  << "# Reviewer\nCheck style.";
    std::ofstream(dir / "coder.md")     << "# Coder\nWrite code.";

    SkillLoader loader(dir.string());
    loader.loadAll();

    auto client = std::make_shared<MockLLMClient>("Final Answer: x");
    SkillProbe agent(client, std::make_shared<SkillLoader>(loader), ToolRegistry{});

    agent.run("first run", 1);
    agent.run("second run", 1);

    check(agent.build_count == 2, "system prompt built before EACH run (2 runs -> 2 builds)");
    check(agent.last_prompt.find("Planner") != std::string::npos &&
          agent.last_prompt.find("Reviewer") != std::string::npos &&
          agent.last_prompt.find("Coder") != std::string::npos,
          ">= 3 skills injected into system prompt");

    fs::remove_all(dir);
}

// ── 7. testLoopDetectorUnit ───────────────────────────────────────────────────
void testLoopDetectorUnit() {
    std::cout << "[TEST] testLoopDetectorUnit\n";

    { // generic repeat -> Critical at critical_threshold (5)
        LoopDetector d;
        LoopDetector::Status s = LoopDetector::Status::Normal;
        for (int i = 0; i < 5; ++i) s = d.add_action("A::B");
        check(s == LoopDetector::Status::Critical, "generic repeat x5 => Critical");
    }
    { // generic repeat -> Warning at warning_threshold (3)
        LoopDetector d;
        LoopDetector::Status s = LoopDetector::Status::Normal;
        for (int i = 0; i < 3; ++i) s = d.add_action("X::Y");
        check(s == LoopDetector::Status::Warning, "generic repeat x3 => Warning");
    }
    { // distinct -> Normal
        LoopDetector d;
        LoopDetector::Status s = d.add_action("a");
        s = d.add_action("b"); s = d.add_action("c");
        check(s == LoopDetector::Status::Normal, "distinct actions stay Normal");
    }
    { // ping-pong uses same 5-cycle critical threshold
        LoopDetector d;
        LoopDetector::Status s = LoopDetector::Status::Normal;
        for (int i = 0; i < 10; ++i) s = d.add_action((i % 2 == 0) ? "A" : "B");
        check(s == LoopDetector::Status::Critical, "ping-pong A,B x5 => Critical");
    }
    { // reset -> Normal
        LoopDetector d;
        for (int i = 0; i < 5; ++i) d.add_action("Z::Z");
        d.reset();
        check(d.add_action("new") == LoopDetector::Status::Normal, "reset clears history => Normal");
    }
}

// ── 8. testLoopAbortIntegration ───────────────────────────────────────────────
class LoopAgent : public AgentLoop {
public:
    explicit LoopAgent(std::shared_ptr<LLMClient> llm) : AgentLoop(std::move(llm)) {}
    bool loop_detected = false;
protected:
    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int) override {
        return ToolCallAction{"calculator", "1+1"};
    }
    std::expected<std::string, std::string> execute_tool(const ToolCallAction&) override {
        return "2";
    }
    void on_loop_detected() override {
        loop_detected = true;
        AgentLoop::on_loop_detected();
    }
};

void testLoopAbortIntegration() {
    std::cout << "[TEST] testLoopAbortIntegration\n";

    auto client = std::make_shared<MockLLMClient>("Final Answer: never");
    LoopAgent agent(client);
    std::string result = agent.run(kNoFallback, 10);

    check(agent.loop_detected, "on_loop_detected invoked");
    check(result == "Loop detected — aborting", "run aborts with loop-detected message");
}

// ── 9. testTemplateMethodSkeleton ─────────────────────────────────────────────
class SequenceAgent : public AgentLoop {
public:
    explicit SequenceAgent(std::shared_ptr<LLMClient> llm) : AgentLoop(std::move(llm)) {}
    std::vector<std::string> calls;
protected:
    std::string build_system_prompt(const std::string& i) override {
        calls.push_back("build_system_prompt");
        return AgentLoop::build_system_prompt(i);
    }
    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int step) override {
        calls.push_back("think_and_act_" + std::to_string(step));
        return FinalAnswerAction{"done-" + std::to_string(step)};
    }
    std::expected<std::string, std::string> execute_tool(const ToolCallAction& a) override {
        calls.push_back("execute_tool_" + a.tool_name);
        return "ok";
    }
    void on_loop_detected() override { calls.push_back("on_loop_detected"); }
    void on_max_steps_reached() override { calls.push_back("on_max_steps_reached"); }
};

void testTemplateMethodSkeleton() {
    std::cout << "[TEST] testTemplateMethodSkeleton\n";

    auto client = std::make_shared<MockLLMClient>("Final Answer: skipped");
    SequenceAgent agent(client);
    std::string result = agent.run(kNoFallback, 4);

    check(result == "done-1", "run returns first final answer");
    check(!agent.calls.empty(), "at least one primitive invoked");
    check(agent.calls[0] == "build_system_prompt",
          "build_system_prompt is the first primitive called (fixed skeleton)");
    check(agent.calls[1] == "think_and_act_1",
          "think_and_act follows build_system_prompt in the template order");
}

// ── 10. testObserverHook ──────────────────────────────────────────────────────
class HookAgent : public AgentLoop {
public:
    explicit HookAgent(std::shared_ptr<LLMClient> llm) : AgentLoop(std::move(llm)) {}
    int hook_calls = 0;
    std::string seen_tool;
protected:
    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int step) override {
        if (step == 1) return ToolCallAction{"calculator", "1+1"};
        return FinalAnswerAction{"done"};
    }
    std::expected<std::string, std::string> execute_tool(const ToolCallAction&) override {
        return "2";
    }
};

void testObserverHook() {
    std::cout << "[TEST] testObserverHook\n";

    auto client = std::make_shared<MockLLMClient>("Final Answer: never");
    HookAgent agent(client);
    agent.set_step_hook([&](const TrajectoryStep& ts) {
        ++agent.hook_calls;
        agent.seen_tool = ts.tool_name;
    });
    agent.run(kNoFallback, 4);

    check(agent.hook_calls == 1, "StepHook fired for the tool-execution step");
    check(agent.seen_tool == "calculator", "hook observed the executed tool name");
}

// ── 11. testRegistryFactoryStrategy ───────────────────────────────────────────
void testRegistryFactoryStrategy() {
    std::cout << "[TEST] testRegistryFactoryStrategy\n";

    ToolRegistry reg;
    // Factory: create fresh instances by canonical name.
    reg.register_creator("calculator", [] { return std::make_unique<CalculatorTool>(); });
    auto made = reg.create("calculator");
    check(made != nullptr && std::string(made->get_name()) == "calculator",
          "Factory create() builds a fresh Tool by name");

    // Registry: store/lookup a shared instance.
    auto inst = std::make_shared<CalculatorTool>();
    reg.register_tool(inst);
    check(reg.lookup("calculator") != nullptr, "Registry lookup finds registered instance");

    // Alias normalization (Strategy: same interface, many names).
    reg.register_alias("calculate", "calculator");
    check(reg.normalize("calculate") == "calculator",
          "alias 'calculate' normalizes to 'calculator'");

    // Policy: deny a tool -> lookup returns nullptr.
    reg.deny("calculator");
    check(reg.lookup("calculator") == nullptr, "deny-list blocks lookup");
    check(reg.lookup("calculate") == nullptr, "deny-list also blocks alias");

    // Strategy demonstration: polymorphic execute via Tool base pointer.
    auto r = inst->execute("2+3");
    check(r.has_value() && *r == "5", "Tool interface (Strategy) executes correctly");
}

// ── 12. testCppFeatureMatrix ──────────────────────────────────────────────────
void testCppFeatureMatrix() {
    std::cout << "[TEST] testCppFeatureMatrix\n";

    // C++23 is the configured baseline.
    static_assert(__cplusplus >= 202302L, "C++23 baseline required");

    // C++20: std::ranges (compile-time feature probe).
#ifdef __cpp_lib_ranges
    static_assert(__cpp_lib_ranges >= 202202L, "C++20 std::ranges available");
    std::vector<int> v{3, 1, 2};
    int cnt = 0;
    for (int x : v | std::views::filter([](int n) { return n > 1; })) { (void)x; ++cnt; }
    check(cnt == 2, "C++20 std::ranges usable at runtime");
#else
    check(true, "C++20 std::ranges not probed (macro absent) — limitation recorded");
#endif

    // C++23: std::expected (compile-time feature probe).
#ifdef __cpp_lib_expected
    static_assert(__cpp_lib_expected >= 202211L, "C++23 std::expected available");
    std::expected<int, std::string> e = 42;
    check(e.has_value() && *e == 42, "C++23 std::expected usable at runtime");
#else
    check(true, "C++23 std::expected not probed (macro absent) — limitation recorded");
#endif

    // C++26: guarded std::inplace_vector with portable fallback.
#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L
    std::inplace_vector<int, 4> iv;
    iv.push_back(1);
    check(iv.size() == 1, "C++26 std::inplace_vector usable");
#else
    // Fallback path: std::vector with reserve — keeps building on older toolchains.
    std::vector<int> iv;
    iv.reserve(4);
    iv.push_back(1);
    check(iv.size() == 1, "C++26 inplace_vector absent -> portable std::vector fallback");
#endif
}

// ── 13. test_agent_loop_fallback_real_tools (deterministic pipeline evidence) ──
void test_agent_loop_fallback_real_tools() {
    std::cout << "[TEST] test_agent_loop_fallback_real_tools\n";

    auto client = std::make_shared<MockLLMClient>("Final Answer: from_llm");
    AgentLoop agent(client); // default registry with real calculator/write_file

    // Matches the task_005 fallback: 47*23 -> result.txt
    std::string result = agent.run("Compute 47*23 and save it to result.txt", 5);

    // The fallback completion message echoes the LAST tool result (the write
    // confirmation); the computed value is asserted via result.txt below.
    check(!result.empty() && result.find("wrote result.txt") != std::string::npos,
          "fallback plan completed via real tools");

    const std::string path = "result.txt";
    bool file_ok = fs::exists(path);
    std::string content;
    if (file_ok) {
        std::ifstream f(path);
        std::getline(f, content);
    }
    check(file_ok && content == "1081",
          "fallback wrote 1081 into result.txt via real write_file tool");

    std::error_code ec;
    fs::remove(path, ec);
}

// ── 14. test_native_environment ───────────────────────────────────────────────
void test_native_environment() {
    std::cout << "[TEST] test_native_environment\n";

    NativeEnvironment env;
    const std::string path = (fs::temp_directory_path() / "role_a_env_test.txt").string();

    check(env.writeFile(path, "hello agent").has_value(), "NativeEnvironment::writeFile succeeds");
    auto r = env.readFile(path);
    check(r.has_value() && *r == "hello agent", "readFile returns written content");
    check(env.exists(path), "exists() true after write");
    check(env.removeFile(path).has_value(), "removeFile succeeds");
    check(!env.exists(path), "exists() false after remove");

    const std::string a = (fs::temp_directory_path() / "role_a_a.txt").string();
    const std::string b = (fs::temp_directory_path() / "role_a_b.txt").string();
    env.writeFile(a, "A");
    env.writeFile(b, "B");
    check(env.cleanArtifacts({a, b}).has_value() && !env.exists(a) && !env.exists(b),
          "cleanArtifacts removes all listed paths");
}

// ── main ────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "=== RUNNING ROLE A (Systems/Core) FOCUSED TESTS — Tuần 10 ===\n\n";

    testClientErrorContract();          std::cout << "\n";
    testMultimodalInterface();          std::cout << "\n";
    testParserVariants();               std::cout << "\n";
    testMalformedToolIntentNotFinalAnswer(); std::cout << "\n";
    testMaxStepsAndHistoryGrowth();     std::cout << "\n";
    testSkillInjectionBeforeEachRun();  std::cout << "\n";
    testLoopDetectorUnit();             std::cout << "\n";
    testLoopAbortIntegration();         std::cout << "\n";
    testTemplateMethodSkeleton();       std::cout << "\n";
    testObserverHook();                 std::cout << "\n";
    testRegistryFactoryStrategy();      std::cout << "\n";
    testCppFeatureMatrix();             std::cout << "\n";
    test_agent_loop_fallback_real_tools(); std::cout << "\n";
    test_native_environment();          std::cout << "\n";

    if (g_failures == 0) {
        std::cout << "=== ALL ROLE A TESTS PASSED SUCCESSFULLY ===\n";
        return 0;
    }
    std::cerr << "=== ROLE A TESTS FAILED: " << g_failures << " check(s) failed ===\n";
    return 1;
}
