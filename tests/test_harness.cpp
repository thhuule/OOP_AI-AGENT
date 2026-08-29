#include "agent/agent_loop.h"
#include "client/llm_client.h"
#include "environment/SandboxEnvironment.h"
#include "harness/HarnessRunner.h"
#include "harness/LLMEvaluator.h"
#include "harness/evaluator.h"
#include "tools/FileTool.h"
#include "tools/Tool.h"

#include <chrono>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using nlohmann::json;
using namespace oop_agent;

namespace {

class TempDirectory {
public:
    TempDirectory() {
        const auto unique = std::chrono::steady_clock::now()
                                .time_since_epoch()
                                .count();
        path_ = fs::temp_directory_path() /
                ("oop_agent_harness_test_" + std::to_string(unique));
        if (!fs::create_directories(path_))
            throw std::runtime_error("Could not create temporary directory");
    }

    ~TempDirectory() {
        std::error_code error;
        fs::remove_all(path_, error);
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    [[nodiscard]] const fs::path& path() const noexcept { return path_; }

private:
    fs::path path_;
};

class ScopedCurrentPath {
public:
    explicit ScopedCurrentPath(const fs::path& path)
        : previous_(fs::current_path()) {
        fs::current_path(path);
    }

    ~ScopedCurrentPath() {
        std::error_code error;
        fs::current_path(previous_, error);
    }

    ScopedCurrentPath(const ScopedCurrentPath&) = delete;
    ScopedCurrentPath& operator=(const ScopedCurrentPath&) = delete;

private:
    fs::path previous_;
};

void require(bool condition, std::string_view message) {
    if (!condition)
        throw std::runtime_error(std::string(message));
}

void writeJson(const fs::path& path, const json& value) {
    std::ofstream output(path);
    if (!output.is_open())
        throw std::runtime_error("Could not write JSON fixture");
    output << value.dump(2) << '\n';
}

json validKeywordTask(std::string id = "task_test") {
    return {
        {"id", std::move(id)},
        {"description", "Harness fixture"},
        {"instruction", "Return a fixture result"},
        {"eval_type", "keyword"},
        {"expected_keywords", "Agent is not connected"},
        {"category", "simple"},
        {"requires_tool", false},
        {"required_tools", json::array()},
        {"artifacts", json::array()},
        {"max_steps", 3}
    };
}

class RecordingEvaluator final : public Evaluator {
public:
    explicit RecordingEvaluator(bool& called) : called_(called) {}

    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "recording_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& expected_output) override {
        called_ = true;
        require(agent_output == "Agent is not connected",
                "custom evaluator received unexpected agent output");
        require(expected_output == "fixture expected",
                "custom evaluator received unexpected expected value");
        return EvalResult{true, 0.75f, "recording evaluator selected"};
    }

private:
    bool& called_;
};

class ScriptedLLMClient final : public LLMClient {
public:
    explicit ScriptedLLMClient(std::vector<std::string> responses,
                               LLMUsage usage = {})
        : responses_(std::move(responses)), usage_(usage) {}

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>&,
        const LLMConfig&) override {
        if (next_ >= responses_.size())
            return std::unexpected(LLMError::UnknownError);
        return responses_[next_++];
    }

    [[nodiscard]] LLMUsage last_usage() const noexcept override { return usage_; }

private:
    std::vector<std::string> responses_;
    std::size_t next_ = 0;
    LLMUsage usage_;
};

class EchoCalculatorTool final : public Tool {
public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "calculator";
    }

    [[nodiscard]] std::string_view get_description() const noexcept override {
        return "Echo calculator fixture";
    }

    std::expected<std::string, ToolError> execute(
        const std::string& arguments) override {
        if (arguments.empty())
            return std::unexpected(ToolError::InvalidArgument);
        return arguments;
    }
};

class ErrorCalculatorTool final : public Tool {
public:
    explicit ErrorCalculatorTool(ToolError error) : error_(error) {}

    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "calculator";
    }

    [[nodiscard]] std::string_view get_description() const noexcept override {
        return "Failing calculator fixture";
    }

    std::expected<std::string, ToolError> execute(
        const std::string&) override {
        return std::unexpected(error_);
    }

private:
    ToolError error_;
};

class ErrorEvaluator final : public Evaluator {
public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "error_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string&,
        const std::string&) override {
        return std::unexpected(EvalError::UnknownError);
    }
};

class CleanupFailingEnvironment final : public Environment {
public:
    std::expected<std::string, EnvError> readFile(
        const std::string&) override {
        return std::unexpected(EnvError::FileNotFound);
    }

    std::expected<void, EnvError> writeFile(
        const std::string&, const std::string&) override {
        return {};
    }

    std::expected<void, EnvError> removeFile(const std::string&) override {
        return {};
    }

    bool exists(const std::string&) const override { return false; }

    std::expected<void, EnvError> cleanArtifacts(
        const std::vector<std::string>&) override {
        return std::unexpected(EnvError::AccessDenied);
    }
};

void testLoadTasksValidation() {
    TempDirectory temp;
    const auto tasks_path = temp.path() / "tasks.json";

    writeJson(tasks_path, json::array({validKeywordTask()}));
    HarnessRunner valid(tasks_path.string(), (temp.path() / "out").string());
    require(valid.loadTasks(), "valid task fixture was rejected");
    require(valid.getTasks().size() == 1, "valid task was not loaded");

    auto unsafe = validKeywordTask("unsafe");
    unsafe["artifacts"] = json::array({"../escape.txt"});
    writeJson(tasks_path, json::array({unsafe}));
    HarnessRunner unsafe_harness(tasks_path.string());
    require(!unsafe_harness.loadTasks(), "unsafe artifact path was accepted");
    require(unsafe_harness.getTasks().empty(),
            "invalid load left partially loaded tasks");

    const auto duplicate = validKeywordTask("duplicate");
    writeJson(tasks_path, json::array({duplicate, duplicate}));
    HarnessRunner duplicate_harness(tasks_path.string());
    require(!duplicate_harness.loadTasks(), "duplicate task ID was accepted");
    require(duplicate_harness.getTasks().empty(),
            "duplicate load left partially loaded tasks");

    auto llm = validKeywordTask("llm_eval");
    llm["eval_type"] = "llm";
    writeJson(tasks_path, json::array({llm}));
    HarnessRunner llm_harness(tasks_path.string());
    require(llm_harness.loadTasks(), "llm evaluator task was rejected");
}

void testLLMEvaluator() {
    auto client = std::make_shared<ScriptedLLMClient>(
        std::vector<std::string>{"PASS: answer satisfies the rubric"});
    LLMEvaluator evaluator(client, LLMConfig{});
    const auto result = evaluator.evaluate("51", "answer should be 51");
    require(result.has_value(), "LLM evaluator returned an error");
    require(result->is_passed && result->score == 1.0f,
            "LLM evaluator did not accept PASS judgement");
}

void testStrategySelection() {
    TempDirectory temp;
    HarnessRunner harness((temp.path() / "unused.json").string());
    bool called = false;
    harness.registerEvaluator(
        "recording", std::make_unique<RecordingEvaluator>(called));

    Task task;
    task.id = "strategy";
    task.description = "Select a custom evaluator";
    task.instruction = "unused";
    task.eval_type = "recording";
    task.expected_keywords = "fixture expected";
    task.category = "simple";
    task.requires_tool = false;
    task.max_steps = 1;

    const auto result = harness.runSingle(task);
    require(called, "registered evaluator strategy was not called");
    require(result.evaluator_success, "custom evaluator result was lost");
    require(result.evaluator_score == 0.75f,
            "custom evaluator score was not preserved");
    require(result.success, "successful evaluator did not produce final success");
    require(result.failure_reason == "NONE",
            "successful strategy was assigned a failure reason");
}

void testStepHookPreservesActionArguments() {
    TempDirectory temp;
    auto client = std::make_shared<ScriptedLLMClient>(
        std::vector<std::string>{
            R"({"tool":"calculator","args":"123"})",
            "done"
        }, LLMUsage{2, 3});
    AgentLoop agent(client);
    agent.register_tool(std::make_unique<EchoCalculatorTool>());

    HarnessRunner harness((temp.path() / "unused.json").string());
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);

    Task task;
    task.id = "trajectory";
    task.description = "Record a tool call";
    task.instruction = "Use the calculator fixture";
    task.eval_type = "keyword";
    task.expected_keywords = "done";
    task.category = "simple";
    task.requires_tool = true;
    task.required_tools = {"calculator"};
    task.max_steps = 3;

    const auto result = harness.runSingle(task);
    require(result.success, "scripted tool task did not pass");
    require(result.tool_steps_count == 1,
            "trajectory did not contain exactly one tool step");
    require(result.trajectory.size() == 2 &&
            result.trajectory.back().tool_name == "final_answer",
            "final answer was not recorded as a non-tool trajectory step");
    require(result.total_tokens == 10,
            "token total did not include tool-decision and final-answer calls");

    const auto& step = result.trajectory.front();
    const auto action = json::parse(step.action);
    require(action.at("type") == "tool_call", "action type was not recorded");
    require(action.at("tool") == "calculator", "tool name was not recorded");
    require(action.at("args") == "123", "tool args were not preserved");
    require(step.result == "123", "tool result was not recorded");
    require(step.latency_ms >= 0.0, "step latency was negative");
    require(step.tokens_used == 5,
            "provider token usage was not preserved on the tool step");
}

void testExportIncludesFinalAnswerAndTokens() {
    TempDirectory temp;
    const auto output = temp.path() / "results";
    HarnessRunner harness((temp.path() / "unused.json").string(), output.string());

    TaskRunResult result;
    result.task_id = "task_export";
    result.category = "simple";
    result.agent_output = "done";
    result.success = true;
    result.tool_steps_count = 1;
    result.total_tokens = 12;
    TrajectoryStep tool;
    tool.step = 1;
    tool.action = R"({"type":"tool_call","tool":"calculator","args":"1+1"})";
    tool.tool_name = "calculator";
    tool.result = "2";
    tool.success = true;
    tool.tokens_used = 5;
    TrajectoryStep final;
    final.step = 2;
    final.action = "final_answer";
    final.tool_name = "final_answer";
    final.result = "done";
    final.success = true;
    final.tokens_used = 7;
    result.trajectory = {tool, final};

    require(harness.exportResults({result}), "trajectory export failed");
    fs::path run_dir;
    for (const auto& entry : fs::directory_iterator(output)) {
        if (entry.is_directory()) {
            run_dir = entry.path();
            break;
        }
    }
    require(!run_dir.empty(), "export did not create a run directory");
    std::ifstream file(run_dir / "trajectory_task_export.json");
    const json exported = json::parse(file);
    require(exported.at("final_answer") == "done",
            "export omitted final_answer");
    require(exported.at("total_tokens") == 12,
            "exported total_tokens is incorrect");
    require(exported.at("tool_steps_count") == 1 && exported.at("steps").size() == 2,
            "final answer incorrectly changed tool step count");
}

void testInstructionFallbackUsesToolWhenLLMOmitsToolCall() {
    TempDirectory temp;
    auto client = std::make_shared<ScriptedLLMClient>(
        std::vector<std::string>{"I cannot decide yet"});
    AgentLoop agent(client);
    agent.set_fallback_enabled(true);
    agent.register_tool(std::make_shared<FileWriteTool>());

    HarnessRunner harness((temp.path() / "unused.json").string());
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);

    Task task;
    task.id = "instruction_fallback";
    task.description = "Instruction fallback fixture";
    task.instruction = "Tạo file notes.txt với nội dung 'Agent test run'";
    task.eval_type = "functional";
    task.eval_script = "test -f notes.txt && grep -F 'Agent test run' notes.txt && echo PASS";
    task.category = "simple";
    task.requires_tool = true;
    task.required_tools = {"write_file"};
    task.artifacts = {"notes.txt"};
    task.max_steps = 2;

    const auto result = harness.runSingle(task);
    require(result.success, "instruction fallback did not complete the write task");
}

void testCountFallbackReturnsNumericResult() {
    TempDirectory temp;
    ScopedCurrentPath current_path(temp.path());

    {
        std::ofstream notes("notes.txt");
        notes << "Agent test run\n";
    }

    auto client = std::make_shared<ScriptedLLMClient>(
        std::vector<std::string>{"I cannot decide yet"});
    AgentLoop agent(client);
    agent.set_fallback_enabled(true);

    HarnessRunner harness((temp.path() / "unused.json").string());
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);

    Task task;
    task.id = "count_fallback";
    task.description = "Count fallback fixture";
    task.instruction = "Đọc file notes.txt, đếm số từ trong đó rồi in ra kết quả";
    task.eval_type = "keyword";
    task.expected_keywords = "3";
    task.category = "medium";
    task.requires_tool = true;
    task.required_tools = {"read_file", "execute_shell"};
    task.max_steps = 3;

    const auto result = harness.runSingle(task);
    require(result.success, "count fallback did not return the word count");
}

void testBatchCleanupRemovesStaleArtifacts() {
    TempDirectory temp;
    ScopedCurrentPath current_path(temp.path());

    {
        std::ofstream stale("notes.txt");
        stale << "stale benchmark data\n";
    }
    require(fs::exists("notes.txt"), "stale fixture was not created");

    const fs::path tasks_path = temp.path() / "tasks.json";
    writeJson(tasks_path, json::array({validKeywordTask("cleanup")}));
    HarnessRunner harness(tasks_path.string(), (temp.path() / "out").string());
    require(harness.loadTasks(), "cleanup task could not be loaded");

    const auto results = harness.runAll();
    require(!fs::exists("notes.txt"), "batch cleanup kept a stale artifact");
    require(results.size() == 1 && results.front().success,
            "cleanup batch did not complete its fixture task");
}

void testHarnessUsesSandboxEnvironmentForCleanup() {
    TempDirectory temp;
    const fs::path tasks_path = temp.path() / "tasks.json";
    writeJson(tasks_path, json::array({validKeywordTask("sandbox_cleanup")}));

    auto environment = std::make_shared<SandboxEnvironment>();
    require(environment->writeFile("notes.txt", "stale data").has_value(),
            "sandbox stale fixture was not created");

    HarnessRunner harness(tasks_path.string(),
                          (temp.path() / "out").string(),
                          environment);
    require(harness.loadTasks(), "sandbox cleanup task could not be loaded");

    const auto results = harness.runAll();
    require(!environment->exists("notes.txt"),
            "Harness did not clean through the injected Environment");
    require(results.size() == 1 && results.front().success,
            "sandbox cleanup batch did not complete");
}

void testCleanupFailureHasSpecificReason() {
    TempDirectory temp;
    const fs::path tasks_path = temp.path() / "tasks.json";
    writeJson(tasks_path, json::array({validKeywordTask("cleanup_failure")}));

    auto environment = std::make_shared<CleanupFailingEnvironment>();
    HarnessRunner harness(tasks_path.string(),
                          (temp.path() / "out").string(),
                          environment);
    require(harness.loadTasks(), "cleanup failure task could not be loaded");

    const auto results = harness.runAll();
    require(results.size() == 1,
            "cleanup failure did not return one result per task");
    require(results.front().failure_reason == "ARTIFACT_CLEANUP_FAILED",
            "cleanup failure received a generic failure reason");
}

void testArtifactFailureTaxonomy() {
    TempDirectory temp;
    ScopedCurrentPath current_path(temp.path());
    HarnessRunner harness((temp.path() / "unused.json").string());

    Task missing;
    missing.id = "artifact_missing";
    missing.description = "Missing artifact";
    missing.instruction = "unused";
    missing.eval_type = "functional";
    missing.eval_script = "test -f missing.txt && echo PASS";
    missing.category = "simple";
    missing.requires_tool = false;
    missing.artifacts = {"missing.txt"};
    missing.max_steps = 1;

    const auto missing_result = harness.runSingle(missing);
    require(missing_result.failure_reason == "ARTIFACT_MISSING",
            "missing artifact was classified incorrectly");

    {
        std::ofstream mismatch("mismatch.txt");
        mismatch << "wrong content\n";
    }

    Task content = missing;
    content.id = "artifact_content_mismatch";
    content.description = "Artifact content mismatch";
    content.eval_script = "grep -F expected mismatch.txt && echo PASS";
    content.artifacts = {"mismatch.txt"};

    const auto mismatch_result = harness.runSingle(content);
    require(mismatch_result.failure_reason == "ARTIFACT_CONTENT_MISMATCH",
            "content mismatch was classified incorrectly");
}

Task toolFailureTask(std::string id) {
    Task task;
    task.id = std::move(id);
    task.description = "Tool failure taxonomy fixture";
    task.instruction = "Call the calculator";
    task.eval_type = "keyword";
    task.expected_keywords = "never produced";
    task.category = "simple";
    task.requires_tool = true;
    task.required_tools = {"calculator"};
    task.max_steps = 1;
    return task;
}

TaskRunResult runToolFailureFixture(ToolError error) {
    auto client = std::make_shared<ScriptedLLMClient>(
        std::vector<std::string>{
            R"({"tool":"calculator","args":"bad"})"
        });
    AgentLoop agent(client);
    agent.register_tool(std::make_unique<ErrorCalculatorTool>(error));

    TempDirectory temp;
    HarnessRunner harness((temp.path() / "unused.json").string());
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);
    return harness.runSingle(toolFailureTask("tool_failure"));
}

void testSpecificFailureTaxonomy() {
    const auto invalid = runToolFailureFixture(ToolError::InvalidArgument);
    require(invalid.failure_reason == "INVALID_ARGS",
            "ToolError::InvalidArgument was not classified as INVALID_ARGS");
    require(!invalid.action_level_success,
            "invalid tool result was counted as an action-level success");

    const auto execution = runToolFailureFixture(ToolError::ExecutionFailed);
    require(execution.failure_reason == "TOOL_EXECUTION_FAILED",
            "ToolError::ExecutionFailed was not classified specifically");
    require(!execution.action_level_success,
            "failed tool result was counted as an action-level success");

    TempDirectory temp;
    HarnessRunner harness((temp.path() / "unused.json").string());
    harness.registerEvaluator("error", std::make_unique<ErrorEvaluator>());

    Task evaluator_error;
    evaluator_error.id = "evaluator_error";
    evaluator_error.description = "Evaluator error taxonomy fixture";
    evaluator_error.instruction = "unused";
    evaluator_error.eval_type = "error";
    evaluator_error.expected_keywords = "unused";
    evaluator_error.category = "simple";
    evaluator_error.requires_tool = false;
    evaluator_error.max_steps = 1;

    const auto result = harness.runSingle(evaluator_error);
    require(result.failure_reason == "EVALUATOR_ERROR",
            "Evaluator failure was not classified as EVALUATOR_ERROR");
}

void testRequiredToolCannotBeSkipped() {
    TempDirectory temp;
    auto client = std::make_shared<ScriptedLLMClient>(
        std::vector<std::string>{"final without a tool"});
    AgentLoop agent(client);

    HarnessRunner harness((temp.path() / "unused.json").string());
    agent.set_step_hook(harness.createStepHook());
    harness.set_agent(&agent);

    Task task;
    task.id = "requires_tool";
    task.description = "Final answer cannot replace a tool call";
    task.instruction = "Use a calculator";
    task.eval_type = "keyword";
    task.expected_keywords = "final without a tool";
    task.category = "simple";
    task.requires_tool = true;
    task.required_tools = {"calculator"};
    task.max_steps = 1;

    const auto result = harness.runSingle(task);
    require(result.evaluator_success,
            "fixture final answer should pass the keyword evaluator");
    require(!result.action_level_success,
            "final answer without a tool passed action-level evaluation");
    require(!result.success, "task without a required tool passed overall");
    require(result.failure_reason == "NO_TOOL_EXECUTION",
            "missing required tool received the wrong failure reason");
}

void testAggregateScores() {
    TaskRunResult passed;
    passed.success = true;
    passed.evaluator_success = true;
    passed.action_level_success = true;

    TaskRunResult failed;
    const std::vector<TaskRunResult> results = {passed, failed};

    require(HarnessRunner::computeSuccessRate(results) == 0.5f,
            "final success rate aggregation is incorrect");
    require(HarnessRunner::computeEvaluatorScore(results) == 0.5f,
            "evaluator score aggregation is incorrect");
    require(HarnessRunner::computeActionLevelScore(results) == 0.5f,
            "action-level score aggregation is incorrect");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string_view, std::function<void()>>> tests = {
        {"load task validation", testLoadTasksValidation},
        {"LLM evaluator", testLLMEvaluator},
        {"evaluator strategy selection", testStrategySelection},
        {"StepHook action arguments", testStepHookPreservesActionArguments},
        {"trajectory final answer and tokens export", testExportIncludesFinalAnswerAndTokens},
        {"instruction fallback", testInstructionFallbackUsesToolWhenLLMOmitsToolCall},
        {"count fallback", testCountFallbackReturnsNumericResult},
        {"batch artifact cleanup", testBatchCleanupRemovesStaleArtifacts},
        {"sandbox Environment cleanup", testHarnessUsesSandboxEnvironmentForCleanup},
        {"cleanup failure taxonomy", testCleanupFailureHasSpecificReason},
        {"artifact failure taxonomy", testArtifactFailureTaxonomy},
        {"specific failure taxonomy", testSpecificFailureTaxonomy},
        {"required tool cannot be skipped", testRequiredToolCannotBeSkipped},
        {"aggregate scores", testAggregateScores}
    };

    int failed = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
            std::cout << "[PASS] " << name << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }

    if (failed != 0) {
        std::cerr << failed << " harness test(s) failed\n";
        return 1;
    }

    std::cout << "ALL HARNESS TESTS PASSED\n";
    return 0;
}
