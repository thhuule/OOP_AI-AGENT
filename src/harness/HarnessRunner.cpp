#include "HarnessRunner.h"
#include "FunctionalEvaluator.h"
#include "KeywordEvaluator.h"
#include "../environment/NativeEnvironment.h"
#include "../multiagent/MultiAgentRunner.h"
#include "../tools/CalculatorTool.h"
#include "../tools/ExecTool.h"
#include "../tools/WebSearchTool.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <thread>

namespace oop_agent {
namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool containsAny(const std::string& value,
                 const std::initializer_list<std::string_view> needles) {
    const std::string lower = toLower(value);
    return std::ranges::any_of(needles, [&](std::string_view needle) {
        return lower.find(needle) != std::string::npos;
    });
}

bool isSafeArtifactPath(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute())
        return false;

    for (const auto& component : path) {
        if (component == "..")
            return false;
    }
    return true;
}

std::string makeRunId() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()) %
                        1000;
    std::tm local_time{};
#ifdef _WIN32
    localtime_s(&local_time, &time);
#else
    localtime_r(&time, &local_time);
#endif
    std::ostringstream out;
    out << std::put_time(&local_time, "run_%Y%m%d_%H%M%S")
        << '_' << std::setw(3) << std::setfill('0') << millis.count();
    return out.str();
}

std::string loadModelName() {
    std::filesystem::path config_path = "config.json";
    if (!std::filesystem::exists(config_path))
        config_path = "../config.json";
    if (!std::filesystem::exists(config_path))
        return "unknown";

    std::ifstream config(config_path);
    if (!config.is_open())
        return "unknown";

    try {
        const auto json = nlohmann::json::parse(config);
        return json.value("model", json.value("model_name", "unknown"));
    } catch (const nlohmann::json::exception&) {
        return "unknown";
    }
}

nlohmann::json parseAction(const std::string& action) {
    try {
        auto parsed = nlohmann::json::parse(action);
        if (parsed.is_object())
            return parsed;
    } catch (const nlohmann::json::exception&) {
    }

    nlohmann::json parsed = {
        {"type", "tool_call"},
        {"tool", action},
        {"args", ""}
    };
    const auto open_paren = action.find('(');
    const auto close_paren = action.rfind(')');
    if (open_paren != std::string::npos &&
        close_paren != std::string::npos &&
        close_paren > open_paren) {
        parsed["tool"] = action.substr(0, open_paren);
        parsed["args"] =
            action.substr(open_paren + 1, close_paren - open_paren - 1);
    }
    return parsed;
}

std::string actionToolName(const std::string& action) {
    const auto parsed = parseAction(action);
    if (parsed.contains("tool") && parsed["tool"].is_string())
        return toLower(parsed["tool"].get<std::string>());
    return {};
}

std::string toolErrorName(ToolError error) {
    switch (error) {
    case ToolError::InvalidArgument: return "InvalidArgument";
    case ToolError::ExecutionFailed: return "ExecutionFailed";
    case ToolError::AccessDenied: return "AccessDenied";
    case ToolError::NotFound: return "NotFound";
    case ToolError::UnknownError: return "UnknownError";
    }
    return "UnknownError";
}

} // namespace

HarnessRunner::HarnessRunner(const std::string& tasks_json_path,
                             const std::string& output_dir,
                             std::shared_ptr<Environment> environment)
    : tasks_json_path_(tasks_json_path),
      output_dir_(output_dir),
      environment_(environment ? std::move(environment)
                               : std::make_shared<NativeEnvironment>()) {
    registerEvaluator("keyword", std::make_unique<KeywordEvaluator>());
    auto exec_tool = std::make_shared<ExecTool>();
    registerEvaluator("functional",
                      std::make_unique<FunctionalEvaluator>(exec_tool));
}

bool HarnessRunner::loadTasks() {
    std::ifstream file(tasks_json_path_);
    if (!file.is_open()) {
        std::cerr << "[HarnessRunner] Cannot open file: "
                  << tasks_json_path_ << "\n";
        return false;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(file);
    } catch (const nlohmann::json::exception& error) {
        std::cerr << "[HarnessRunner] JSON parse error: "
                  << error.what() << "\n";
        return false;
    }

    if (!json.is_array()) {
        std::cerr << "[HarnessRunner] tasks.json must be a JSON array\n";
        return false;
    }

    tasks_.clear();
    std::set<std::string> task_ids;
    for (std::size_t index = 0; index < json.size(); ++index) {
        const auto& item = json[index];
        if (!item.is_object()) {
            std::cerr << "[HarnessRunner] Task #" << index
                       << " is not a JSON object\n";
            return false;
        }

        Task task;
        task.id = item.value("id", "");
        task.description = item.value("description", "");
        task.instruction = item.value("instruction", "");
        task.eval_type = item.value("eval_type", "");
        task.expected_keywords = item.value("expected_keywords", "");
        task.eval_script = item.value("eval_script", "");
        task.category = item.value("category", "");
        task.requires_tool = item.value("requires_tool", false);
        task.max_steps = item.value("max_steps", 0);

        if (item.contains("required_tools") && item["required_tools"].is_array())
            task.required_tools =
                item["required_tools"].get<std::vector<std::string>>();
        if (item.contains("artifacts") && item["artifacts"].is_array())
            task.artifacts = item["artifacts"].get<std::vector<std::string>>();

        const bool evaluator_spec_valid =
            (task.eval_type == "keyword" && !task.expected_keywords.empty()) ||
            (task.eval_type == "functional" && !task.eval_script.empty()) ||
            (task.eval_type == "llm" &&
             (!task.expected_keywords.empty() || !task.eval_script.empty()));
        const bool required_fields_valid =
            !task.id.empty() && !task.description.empty() &&
            !task.instruction.empty() && !task.category.empty() &&
            task.max_steps > 0 && item.contains("requires_tool");
        const bool tool_spec_valid =
            !task.requires_tool || !task.required_tools.empty();
        const bool artifacts_safe =
            std::ranges::all_of(task.artifacts, [](const std::string& path) {
                return isSafeArtifactPath(path);
            });

        if (!required_fields_valid || !evaluator_spec_valid ||
            !tool_spec_valid || !artifacts_safe ||
            !task_ids.insert(task.id).second) {
            std::cerr << "[HarnessRunner] Invalid task specification at index "
                      << index << " (id='" << task.id << "')\n";
            tasks_.clear();
            return false;
        }

        std::cout << "[HarnessRunner] Loaded task: " << task.id
                  << " [" << task.category << "]"
                  << (task.requires_tool ? " [requires tool]" : "") << "\n";
        tasks_.push_back(std::move(task));
    }

    std::cout << "[HarnessRunner] Loaded " << tasks_.size() << " tasks\n";
    return !tasks_.empty();
}

void HarnessRunner::registerEvaluator(const std::string& name,
                                      std::unique_ptr<Evaluator> evaluator) {
    if (!evaluator)
        return;
    std::cout << "[HarnessRunner] Registered evaluator: " << name
              << " (" << evaluator->get_name() << ")\n";
    evaluators_[name] = std::move(evaluator);
}

std::vector<TaskRunResult> HarnessRunner::runAll() {
    std::vector<TaskRunResult> results;
    results.reserve(tasks_.size());

    if (!cleanBenchmarkArtifacts()) {
        std::cerr << "[HarnessRunner] Cannot clean benchmark artifacts; "
                     "stopping to prevent a false positive.\n";
        for (const auto& task : tasks_) {
            TaskRunResult result;
            result.task_id = task.id;
            result.category = task.category;
            result.requires_tool = task.requires_tool;
            result.failure_reason = "ARTIFACT_CLEANUP_FAILED";
            result.eval_feedback = "Benchmark artifact cleanup failed";
            results.push_back(std::move(result));
        }
        return results;
    }

    const int total = static_cast<int>(tasks_.size());
    int final_passed = 0;
    for (int index = 0; index < total; ++index) {
        const auto& task = tasks_[static_cast<std::size_t>(index)];
        std::cout << "\n[" << index + 1 << "/" << total << "] Running "
                  << task.id << " - " << task.description
                  << " [" << task.category << "]\n";

        auto result = runSingle(task);
        if (result.success)
            ++final_passed;
        std::cout << "[Progress] " << final_passed << "/" << index + 1
                  << " final PASS\n";
        results.push_back(std::move(result));

        if (index + 1 < total) {
            std::cout << "[RateLimit] Cho 3 giay truoc task tiep theo...\n";
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    const auto print_category = [&](std::string_view category) {
        int passed = 0;
        int count = 0;
        for (const auto& result : results) {
            if (result.category == category) {
                ++count;
                if (result.success)
                    ++passed;
            }
        }
        std::cout << "  " << category << ": " << passed << "/" << count
                  << "\n";
    };

    std::cout << "\n========== BENCHMARK SUMMARY ==========\n";
    print_category("simple");
    print_category("medium");
    print_category("hard");
    std::cout << "  evaluator_score: "
              << computeEvaluatorScore(results) * 100.0f << "%\n";
    std::cout << "  action_level_score: "
              << computeActionLevelScore(results) * 100.0f << "%\n";
    std::cout << "  final_success_rate: "
              << computeSuccessRate(results) * 100.0f << "%\n";
    std::cout << "=======================================\n";
    return results;
}

bool HarnessRunner::runMultiAgentDemo(const std::string& report_path) {
    return runMultiAgentDemo(MultiAgentDemoInput{}, report_path);
}

bool HarnessRunner::runMultiAgentDemo(const MultiAgentDemoInput& input,
                                      const std::string& report_path) {
    MultiAgentRunner runner;

    runner.registerAgent(
        "calculator", "Compute 47 * 23",
        [](MessageQueue& in, MessageQueue& out) {
            const auto request = in.pop(1000);
            if (!request) {
                out.push({"calculator", "main", "ERROR=calculator:timeout"});
                return;
            }
            CalculatorTool tool;
            const auto result = tool.execute(request->content);
            out.push(AgentMessage{
                "calculator", "main",
                result ? "CALC=" + *result
                       : "ERROR=calculator:" + toolErrorName(result.error())});
        });
    runner.registerAgent(
        "researcher", "Find Japan capital",
        [](MessageQueue& in, MessageQueue& out) {
            const auto request = in.pop(1000);
            if (!request) {
                out.push({"researcher", "main", "ERROR=researcher:timeout"});
                return;
            }
            WebSearchTool tool;
            const auto result = tool.execute(request->content);
            out.push(AgentMessage{
                "researcher", "main",
                result ? "CAPITAL=" + *result
                       : "ERROR=researcher:" + toolErrorName(result.error())});
        });

    runner.startAll();
    runner.sendMessage(AgentMessage{"harness", "calculator", input.calculation});
    runner.sendMessage(AgentMessage{"harness", "researcher", input.research_query});

    std::string calc;
    std::string capital;
    std::vector<std::string> errors;
    for (int received = 0; received < 2; ++received) {
        const auto result = runner.receiveMessage("main", 6000);
        if (!result) {
            errors.push_back("ERROR=main:worker_timeout");
            break;
        }
        if (result->content.starts_with("CALC=")) calc = result->content;
        else if (result->content.starts_with("CAPITAL=")) capital = result->content;
        else errors.push_back(result->content);
    }
    runner.stopAndJoinAll();

    if (calc.empty()) errors.push_back("ERROR=calculator:missing_result");
    if (capital.empty()) errors.push_back("ERROR=researcher:missing_result");

    const std::filesystem::path path(report_path);
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) return false;

    std::ofstream report(path);
    if (!report) return false;
    const bool success = errors.empty();
    report << "MULTI-AGENT REPORT\nSTATUS=" << (success ? "PASS" : "FAIL") << '\n';
    if (!calc.empty()) report << calc << '\n';
    if (!capital.empty()) report << capital << '\n';
    for (const auto& error_message : errors) report << error_message << '\n';
    return success;
}

TaskRunResult HarnessRunner::runSingle(const Task& task) {
    TaskRunResult result;
    result.task_id = task.id;
    result.category = task.category;
    result.requires_tool = task.requires_tool;

    current_trajectory_.clear();
    last_step_time_ = std::chrono::steady_clock::now();
    const auto start = last_step_time_;

    if (agent_)
        result.agent_output = agent_->run(task.instruction, task.max_steps);
    else
        result.agent_output = "Agent is not connected";

    const auto end = std::chrono::steady_clock::now();
    result.latency_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    result.trajectory = current_trajectory_;
    result.tool_steps_count = static_cast<std::size_t>(std::ranges::count_if(
        result.trajectory, [](const TrajectoryStep& step) {
            return !step.tool_name.empty() && step.tool_name != "final_answer";
        }));
    for (const auto& step : result.trajectory)
        result.total_tokens += step.tokens_used;

    const auto evaluator = findEvaluator(task.eval_type);
    if (!evaluator) {
        result.eval_feedback = "Evaluator khong hop le: " + task.eval_type;
    } else {
        const std::string& expected =
            task.eval_type == "functional" || task.expected_keywords.empty()
                ? task.eval_script
                : task.expected_keywords;
        const auto evaluated = evaluator.value()->evaluate(result.agent_output,
                                                           expected);
        if (evaluated) {
            result.evaluator_success = evaluated->is_passed;
            result.evaluator_score = evaluated->score;
            result.eval_feedback = evaluated->feedback;
        } else {
            result.eval_feedback = "Evaluator error: " +
                                   std::to_string(
                                       static_cast<int>(evaluated.error()));
        }
    }

    result.action_level_success =
        !task.requires_tool || hasRelevantSuccessfulToolStep(task);
    result.action_level_score = result.action_level_success ? 1.0f : 0.0f;
    result.success =
        result.evaluator_success && result.action_level_success;
    result.failure_reason = classifyFailure(task, result);

    std::cout << "[HarnessRunner] " << task.id
              << " | evaluator="
              << (result.evaluator_success ? "PASS" : "FAIL")
              << " | action="
              << (result.action_level_success ? "PASS" : "FAIL")
              << " | final=" << (result.success ? "PASS" : "FAIL")
              << " | reason=" << result.failure_reason << "\n";
    return result;
}

bool HarnessRunner::exportResults(
    const std::vector<TaskRunResult>& results) const {
    const std::filesystem::path run_dir =
        std::filesystem::path(output_dir_) / makeRunId();
    std::error_code error;
    std::filesystem::create_directories(run_dir, error);
    if (error) {
        std::cerr << "[HarnessRunner] Cannot create output directory: "
                  << run_dir.string() << "\n";
        return false;
    }

    nlohmann::json summary;
    summary["total_tasks"] = results.size();
    summary["evaluator_score"] = computeEvaluatorScore(results);
    summary["action_level_score"] = computeActionLevelScore(results);
    summary["final_success_rate"] = computeSuccessRate(results);

    nlohmann::json category_scores;
    for (const std::string category : {"simple", "medium", "hard"}) {
        int passed = 0;
        int total = 0;
        for (const auto& result : results) {
            if (result.category == category) {
                ++total;
                if (result.success)
                    ++passed;
            }
        }
        category_scores[category] = {{"passed", passed}, {"total", total}};
    }
    summary["category_scores"] = std::move(category_scores);

    nlohmann::json result_array = nlohmann::json::array();
    for (const auto& result : results) {
        result_array.push_back({
            {"task_id", result.task_id},
            {"category", result.category},
            {"success", result.success},
            {"requires_tool", result.requires_tool},
            {"tool_steps_count", result.tool_steps_count},
            {"total_tokens", result.total_tokens},
            {"failure_reason", result.failure_reason},
            {"evaluator_success", result.evaluator_success},
            {"evaluator_score", result.evaluator_score},
            {"action_level_success", result.action_level_success},
            {"action_level_score", result.action_level_score},
            {"total_time_ms", result.latency_ms},
            {"feedback", result.eval_feedback},
            {"agent_output", result.agent_output}
        });
    }
    summary["results"] = std::move(result_array);

    const auto summary_path = run_dir / "eval_results.json";
    std::ofstream summary_file(summary_path);
    if (!summary_file.is_open())
        return false;
    summary_file << summary.dump(2) << "\n";

    const std::string model = loadModelName();
    for (const auto& result : results) {
        nlohmann::json trajectory = {
            {"task_id", result.task_id},
            {"model", model},
            {"success", result.success},
            {"requires_tool", result.requires_tool},
            {"tool_steps_count", result.tool_steps_count},
            {"final_answer", result.agent_output},
            {"failure_reason", result.failure_reason},
            {"evaluator_score", result.evaluator_score},
            {"action_level_score", result.action_level_score},
            {"total_time_ms", result.latency_ms}
        };

        nlohmann::json steps = nlohmann::json::array();
        for (std::size_t index = 0;
             index < result.trajectory.size(); ++index) {
            const auto& step = result.trajectory[index];
            nlohmann::json step_json = {
                {"step_id", index + 1},
                {"source", step.source.empty() ? "llm" : step.source},
                {"thought", step.thought},
                {"action", parseAction(step.action)},
                {"tool_result", step.result},
                {"latency_ms", step.latency_ms},
                {"tokens_used", step.tokens_used}
            };
            steps.push_back(std::move(step_json));
        }
        trajectory["total_tokens"] = result.total_tokens;
        trajectory["steps"] = std::move(steps);

        const auto path =
            run_dir / ("trajectory_" + result.task_id + ".json");
        std::ofstream trajectory_file(path);
        if (!trajectory_file.is_open())
            return false;
        trajectory_file << trajectory.dump(2) << "\n";
    }

    const auto report_path = run_dir / "benchmark_summary.txt";
    std::ofstream report(report_path);
    if (!report.is_open())
        return false;
    report << "ROLE C BENCHMARK SUMMARY\n"
           << "========================\n"
           << "Evaluator score: "
           << computeEvaluatorScore(results) << "\n"
           << "Action-level score: "
           << computeActionLevelScore(results) << "\n"
           << "Final success rate: "
           << computeSuccessRate(results) << "\n\n";
    for (const auto& result : results) {
        report << result.task_id << " [" << result.category << "] "
               << (result.success ? "PASS" : "FAIL")
               << " | tool_steps=" << result.tool_steps_count
               << " | reason=" << result.failure_reason << "\n";
    }

    std::cout << "[HarnessRunner] Da luu ket qua run tai: "
              << run_dir.string() << "\n";
    return true;
}

float HarnessRunner::computeSuccessRate(
    const std::vector<TaskRunResult>& results) {
    if (results.empty())
        return 0.0f;
    const auto passed = std::ranges::count_if(
        results, [](const TaskRunResult& result) { return result.success; });
    return static_cast<float>(passed) /
           static_cast<float>(results.size());
}

float HarnessRunner::computeEvaluatorScore(
    const std::vector<TaskRunResult>& results) {
    if (results.empty())
        return 0.0f;
    const auto score = std::ranges::count_if(
        results,
        [](const TaskRunResult& result) { return result.evaluator_success; });
    return static_cast<float>(score) /
           static_cast<float>(results.size());
}

float HarnessRunner::computeActionLevelScore(
    const std::vector<TaskRunResult>& results) {
    if (results.empty())
        return 0.0f;
    const auto score = std::ranges::count_if(
        results,
        [](const TaskRunResult& result) {
            return result.action_level_success;
        });
    return static_cast<float>(score) /
           static_cast<float>(results.size());
}

StepHook HarnessRunner::createStepHook() {
    return [this](const TrajectoryStep& step) {
        const auto now = std::chrono::steady_clock::now();
        const double latency_ms =
            std::chrono::duration<double, std::milli>(
                now - last_step_time_).count();
        TrajectoryStep recorded = step;
        recorded.latency_ms = latency_ms;
        current_trajectory_.push_back(std::move(recorded));
        last_step_time_ = now;
    };
}

std::optional<Evaluator*>
HarnessRunner::findEvaluator(const std::string& evaluator_type) const {
    const auto evaluator = evaluators_.find(evaluator_type);
    if (evaluator == evaluators_.end())
        return std::nullopt;
    return evaluator->second.get();
}

bool HarnessRunner::cleanBenchmarkArtifacts() const {
    std::set<std::string> artifacts = {
        "notes.txt", "result.txt", "capital.txt",
        "output.txt", "calc.txt", "data.txt"
    };
    for (const auto& task : tasks_) {
        for (const auto& artifact : task.artifacts)
            artifacts.emplace(artifact);
    }

    std::vector<std::string> artifact_paths;
    artifact_paths.reserve(artifacts.size());
    for (const auto& artifact : artifacts) {
        if (!isSafeArtifactPath(std::filesystem::path(artifact)))
            return false;
        if (environment_->exists(artifact))
            std::cout << "[HarnessRunner] Removed stale artifact: "
                      << artifact << "\n";
        artifact_paths.push_back(artifact);
    }

    const auto cleaned = environment_->cleanArtifacts(artifact_paths);
    if (!cleaned) {
        std::cerr << "[HarnessRunner] Environment could not clean artifacts"
                  << " (EnvError=" << static_cast<int>(cleaned.error())
                  << ")\n";
        return false;
    }
    return true;
}

bool HarnessRunner::hasRelevantSuccessfulToolStep(const Task& task) const {
    const auto canonical_tool = [](std::string tool) {
        tool = toLower(std::move(tool));
        if (tool == "exec")
            return std::string{"execute_shell"};
        if (tool == "create_file")
            return std::string{"write_file"};
        if (tool == "calculate")
            return std::string{"calculator"};
        if (tool == "google_search")
            return std::string{"web_search"};
        return tool;
    };

    for (const auto& step : current_trajectory_) {
        // TrajectoryStep::success is the authoritative tool execution status.
        // Do not infer failure from the returned text: a perfectly valid file
        // listing can contain names such as `skills/error_recovery.md`.
        if (!step.success)
            continue;

        const std::string action = canonical_tool(actionToolName(step.action));
        const bool relevant = task.required_tools.empty() ||
            std::ranges::any_of(
                task.required_tools,
                [&](const std::string& tool) {
                    return action == canonical_tool(tool);
                });
        if (relevant)
            return true;
    }
    return false;
}

std::string HarnessRunner::classifyFailure(
    const Task& task, const TaskRunResult& result) const {
    if (result.success)
        return "NONE";

    std::string evidence =
        result.agent_output + " " + result.eval_feedback;
    for (const auto& step : result.trajectory)
        evidence += " " + step.result;

    if (containsAny(evidence, {"rate limit", "429", "resource exhausted"}))
        return "RATE_LIMIT";
    if (containsAny(evidence, {"timeout", "timed out"}))
        return "TIMEOUT";
    if (containsAny(evidence, {"tool not found", "unknown tool"}))
        return "TOOL_NOT_FOUND";
    if (containsAny(evidence,
                    {"invalid argument", "invalid args", "invalidargument",
                     "tool error: invalidargument"}))
        return "INVALID_ARGS";
    if (containsAny(evidence, {"infinite loop", "loop detected"}))
        return "LOOP_DETECTED";
    if (containsAny(evidence, {"evaluator error"}))
        return "EVALUATOR_ERROR";
    if (containsAny(evidence,
                    {"executionfailed", "accessdenied", "unknownerror",
                     "tool error: executionfailed", "tool error: accessdenied",
                     "tool error: unknownerror"}))
        return "TOOL_EXECUTION_FAILED";
    if (result.requires_tool && result.tool_steps_count == 0)
        return "NO_TOOL_EXECUTION";
    if (!result.action_level_success)
        return "NO_TOOL_EXECUTION";
    if (!result.evaluator_success) {
        for (const auto& artifact : task.artifacts) {
            if (!environment_->exists(artifact))
                return "ARTIFACT_MISSING";
        }
        if (!task.artifacts.empty())
            return "ARTIFACT_CONTENT_MISMATCH";
        if (containsAny(result.agent_output,
                        {"maximum step", "stopped", "could not", "unable"}))
            return "INCOMPLETE_TASK";
        return "POST_CONDITION_FAIL";
    }
    return "PARSER_FAIL";
}

} // namespace oop_agent
