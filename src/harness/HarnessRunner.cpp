#include "HarnessRunner.h"
#include "KeywordEvaluator.h"
#include "FunctionalEvaluator.h"
#include "../tools/ExecTool.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>

namespace oop_agent {

// ── Constructor ─────────────────────────────────────────────────────

HarnessRunner::HarnessRunner(const std::string& tasks_json_path,
                             const std::string& output_dir)
    : tasks_json_path_(tasks_json_path)
    , output_dir_(output_dir) {

    // Đăng ký evaluator mặc định: keyword
    registerEvaluator("keyword", std::make_unique<KeywordEvaluator>());
    // Inject ExecTool vào FunctionalEvaluator (Dependency Injection)
    auto exec_tool = std::make_shared<ExecTool>();
    registerEvaluator("functional", std::make_unique<FunctionalEvaluator>(exec_tool));
}

// ── Phase 1: Setup ──────────────────────────────────────────────────

bool HarnessRunner::loadTasks() {
    std::ifstream file(tasks_json_path_);
    if (!file.is_open()) {
        std::cerr << "[HarnessRunner] Không thể mở file: "
                  << tasks_json_path_ << "\n";
        return false;
    }

    // Parse JSON bằng nlohmann/json
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[HarnessRunner] JSON parse error: " << e.what() << "\n";
        return false;
    }

    tasks_.clear();

    if (!j.is_array()) {
        std::cerr << "[HarnessRunner] JSON không phải mảng (expected top-level array)\n";
        return false;
    }

    for (const auto& t : j) {
        Task task;
        task.id                = t.value("id", "");
        task.description       = t.value("description", "");
        task.instruction       = t.value("instruction", "");
        task.eval_type         = t.value("eval_type", "");
        task.expected_keywords = t.value("expected_keywords", "");
        task.eval_script       = t.value("eval_script", "");
        task.category          = t.value("category", "simple");
        task.max_steps         = t.value("max_steps", 10);

        if (!task.id.empty()) {
            std::cout << "[HarnessRunner] Loaded task: " << task.id
                      << " [" << task.category << "]\n";
            tasks_.push_back(std::move(task));
        }
    }

    std::cout << "[HarnessRunner] Tổng cộng " << tasks_.size() << " task\n";
    return !tasks_.empty();
}

void HarnessRunner::registerEvaluator(const std::string& name,
                                       std::unique_ptr<Evaluator> evaluator) {
    std::cout << "[HarnessRunner] Đăng ký evaluator: " << name
              << " (" << evaluator->get_name() << ")\n";
    evaluators_[name] = std::move(evaluator);
}

// ── Phase 2: Run ────────────────────────────────────────────────────

std::vector<TaskRunResult> HarnessRunner::runAll() {
    std::vector<TaskRunResult> results;
    results.reserve(tasks_.size());

    int total = static_cast<int>(tasks_.size());
    int current = 0;
    int running_passed = 0;

    for (const auto& task : tasks_) {
        current++;
        std::cout << "\n────────────────────────────────────────\n";
        std::cout << "[" << current << "/" << total << "] Running "
                  << task.id << " — " << task.description
                  << " [" << task.category << "]\n";

        auto result = runSingle(task);
        if (result.eval_success) running_passed++;

        std::cout << "[Progress] " << running_passed << "/" << current
                  << " passed so far\n";

        results.push_back(std::move(result));

        // Gemini free tier giới hạn request/phút; giãn nhịp giữa các task
        // nhưng không trì hoãn sau task cuối cùng.
        if (current < total) {
            std::cout << "[RateLimit] Chờ 3 giây trước task tiếp theo...\n";
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    // ── Tổng kết theo category ──
    std::cout << "\n════════════════════════════════════════\n";
    std::cout << "           BENCHMARK SUMMARY\n";
    std::cout << "════════════════════════════════════════\n";

    // Đếm pass/total cho mỗi category
    auto count_category = [&](const std::string& cat) -> std::pair<int,int> {
        int passed = 0, total_cat = 0;
        for (size_t i = 0; i < results.size(); ++i) {
            if (tasks_[i].category == cat) {
                total_cat++;
                if (results[i].eval_success) passed++;
            }
        }
        return {passed, total_cat};
    };

    auto [simple_pass, simple_total]   = count_category("simple");
    auto [medium_pass, medium_total]   = count_category("medium");
    auto [hard_pass, hard_total]       = count_category("hard");

    if (simple_total > 0)
        std::cout << "  Simple tasks:  " << simple_pass << "/" << simple_total << " passed\n";
    if (medium_total > 0)
        std::cout << "  Medium tasks:  " << medium_pass << "/" << medium_total << " passed\n";
    if (hard_total > 0)
        std::cout << "  Hard tasks:    " << hard_pass   << "/" << hard_total   << " passed\n";

    float success_rate = computeSuccessRate(results);
    std::cout << "  ────────────────────────────────\n";
    std::cout << "  Total: " << (simple_pass + medium_pass + hard_pass)
              << "/" << results.size()
              << " passed (" << static_cast<int>(success_rate * 100) << "%)\n";
    std::cout << "════════════════════════════════════════\n";

    return results;
}

TaskRunResult HarnessRunner::runSingle(const Task& task) {
    TaskRunResult result;
    result.task_id = task.id;

    // Xóa trajectory cũ
    current_trajectory_.clear();

    auto start = std::chrono::steady_clock::now();

    // ── Gọi AgentLoop thực thi task.instruction ──
    if (agent_) {
        result.agent_output = agent_->run(task.instruction, task.max_steps);
    } else {
        result.agent_output = "[placeholder] Agent chưa được kết nối";
    }

    auto end = std::chrono::steady_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(end - start).count();

    // ── Evaluate (std::optional) ──
    auto opt_evaluator = findEvaluator(task.eval_type);
    if (!opt_evaluator.has_value()) {
        std::cerr << "[HarnessRunner] Không tìm thấy evaluator: "
                  << task.eval_type << "\n";
        result.eval_feedback = "Evaluator không hợp lệ: " + task.eval_type;
        result.trajectory = current_trajectory_;
        return result;
    }

    std::string expected_param = (task.eval_type == "functional") ? task.eval_script : task.expected_keywords;
    auto eval_result = opt_evaluator.value()->evaluate(result.agent_output, expected_param);
    if (eval_result.has_value()) {
        result.eval_success  = eval_result->is_passed;
        result.eval_score    = eval_result->score;
        result.eval_feedback = eval_result->feedback;
    } else {
        result.eval_feedback = "Evaluator lỗi (EvalError)";
    }

    std::cout << "[HarnessRunner] " << task.id
              << " → score=" << result.eval_score
              << " | " << (result.eval_success ? "PASS" : "FAIL")
              << " | " << result.eval_feedback << "\n";

    result.trajectory = current_trajectory_;
    return result;
}

// ── Phase 3: Record ─────────────────────────────────────────────────

bool HarnessRunner::exportResults(const std::vector<TaskRunResult>& results) const {
    // Tạo thư mục output nếu chưa có
    std::filesystem::create_directories(output_dir_);

    // 1. Xuất tổng kết eval_results.json
    std::string filepath = output_dir_ + "/eval_results.json";
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "[HarnessRunner] Không thể tạo file: " << filepath << "\n";
        return false;
    }

    nlohmann::json j;
    j["success_rate"] = computeSuccessRate(results);
    j["total_tasks"]  = results.size();

    nlohmann::json results_arr = nlohmann::json::array();
    for (const auto& r : results) {
        results_arr.push_back({
            {"task_id",      r.task_id},
            {"passed",       r.eval_success},
            {"score",        r.eval_score},
            {"latency_ms",   r.latency_ms},
            {"feedback",     r.eval_feedback},
            {"agent_output", r.agent_output}
        });
    }
    j["results"] = results_arr;
    out << j.dump(2) << "\n";
    std::cout << "[HarnessRunner] Kết quả tổng kết đã lưu tại: " << filepath << "\n";

    // 2. Xuất trajectory_{task_id}.json cho từng task
    for (const auto& r : results) {
        nlohmann::json traj_json;
        traj_json["task_id"] = r.task_id;

        // Đọc model name từ config.json thay vì hardcode
        std::string model_name = "gemini-2.5-flash";
        {
            std::string cfg_path = "config.json";
            if (!std::filesystem::exists(cfg_path)) cfg_path = "../config.json";
            if (std::filesystem::exists(cfg_path)) {
                std::ifstream cfg(cfg_path);
                if (cfg.is_open()) {
                    try {
                        auto j_cfg = nlohmann::json::parse(cfg);
                        if (j_cfg.contains("model")) model_name = j_cfg["model"].get<std::string>();
                        else if (j_cfg.contains("model_name")) model_name = j_cfg["model_name"].get<std::string>();
                    } catch (...) {}
                }
            }
        }
        traj_json["model"] = model_name;
        traj_json["success"] = r.eval_success;

        int total_tokens = 0;
        nlohmann::json steps_arr = nlohmann::json::array();

        for (size_t i = 0; i < r.trajectory.size(); ++i) {
            const auto& step = r.trajectory[i];
            total_tokens += step.tokens_used;

            nlohmann::json step_json;
            step_json["step_id"] = static_cast<int>(i);
            step_json["thought"] = step.thought;

            // Parse action to check if it's JSON or formatted like calculator("15*17")
            nlohmann::json action_obj;
            std::string act = step.action;
            try {
                auto parsed_act = nlohmann::json::parse(act);
                if (parsed_act.is_object()) {
                    action_obj = parsed_act;
                } else {
                    throw std::runtime_error("not object");
                }
            } catch (...) {
                size_t open_paren = act.find('(');
                size_t close_paren = act.rfind(')');
                if (open_paren != std::string::npos && close_paren != std::string::npos && close_paren > open_paren) {
                    std::string tool = act.substr(0, open_paren);
                    std::string args = act.substr(open_paren + 1, close_paren - open_paren - 1);
                    action_obj["type"] = "tool_call";
                    action_obj["tool"] = tool;
                    action_obj["args"] = args;
                } else {
                    action_obj["type"] = "tool_call";
                    action_obj["tool"] = act;
                    action_obj["args"] = "";
                }
            }

            step_json["action"] = action_obj;
            step_json["tool_result"] = step.result;
            step_json["tokens_used"] = step.tokens_used;
            step_json["latency_ms"] = step.latency_ms;

            steps_arr.push_back(step_json);
        }

        traj_json["total_tokens"] = total_tokens;
        traj_json["total_time_ms"] = static_cast<int>(r.latency_ms);
        traj_json["steps"] = steps_arr;

        std::string task_file_path = output_dir_ + "/trajectory_" + r.task_id + ".json";
        std::ofstream task_out(task_file_path);
        if (task_out.is_open()) {
            task_out << traj_json.dump(2) << "\n";
            std::cout << "[HarnessRunner] Đã xuất trajectory cho task " << r.task_id << " tại: " << task_file_path << "\n";
        } else {
            std::cerr << "[HarnessRunner] Không thể tạo file: " << task_file_path << "\n";
        }
    }

    return true;
}

// ── Accessors ───────────────────────────────────────────────────────

float HarnessRunner::computeSuccessRate(
        const std::vector<TaskRunResult>& results) {
    if (results.empty()) return 0.0f;

    int passed = 0;
    for (const auto& r : results) {
        if (r.eval_success) ++passed;
    }
    return static_cast<float>(passed) / static_cast<float>(results.size());
}

StepHook HarnessRunner::createStepHook() {
    // Trả về lambda capture this — khi AgentLoop gọi hook,
    // mỗi step sẽ được ghi vào current_trajectory_
    return [this](const std::string& thought,
                  const std::string& action,
                  const std::string& result) {
        current_trajectory_.push_back({thought, action, result});
    };
}

// ── Private ─────────────────────────────────────────────────────────

std::optional<Evaluator*> HarnessRunner::findEvaluator(const std::string& evaluator_type) const {
    auto it = evaluators_.find(evaluator_type);
    if (it != evaluators_.end()) {
        return it->second.get();
    }
    return std::nullopt;
}

}
