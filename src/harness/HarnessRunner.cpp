#include "HarnessRunner.h"
#include "KeywordEvaluator.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace oop_agent {

// ── Constructor ─────────────────────────────────────────────────────

HarnessRunner::HarnessRunner(const std::string& tasks_json_path,
                             const std::string& output_dir)
    : tasks_json_path_(tasks_json_path)
    , output_dir_(output_dir) {

    // Đăng ký evaluator mặc định: keyword
    registerEvaluator("keyword", std::make_unique<KeywordEvaluator>());
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
        task.max_steps         = t.value("max_steps", 10);

        if (!task.id.empty()) {
            std::cout << "[HarnessRunner] Loaded task: " << task.id << "\n";
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

    for (const auto& task : tasks_) {
        std::cout << "\n────────────────────────────────────────\n";
        std::cout << "[HarnessRunner] Chạy task: " << task.id
                  << " — " << task.description << "\n";

        auto result = runSingle(task);
        results.push_back(std::move(result));
    }

    // In tổng kết
    float success_rate = computeSuccessRate(results);
    std::cout << "\n════════════════════════════════════════\n";
    std::cout << "[HarnessRunner] Success rate: "
              << static_cast<int>(success_rate * 100) << "% ("
              << static_cast<int>(success_rate * results.size()) << "/"
              << results.size() << ")\n";

    return results;
}

TaskRunResult HarnessRunner::runSingle(const Task& task) {
    TaskRunResult result;
    result.task_id = task.id;

    // Xóa trajectory cũ
    current_trajectory_.clear();

    auto start = std::chrono::steady_clock::now();

    // ── TODO: Gọi AgentLoop thực thi task.instruction ──
    // Hiện tại placeholder — sẽ thay bằng AgentLoop::run(task.instruction) khi A hoàn thành
    result.agent_output = "[placeholder] Agent chưa được kết nối";

    auto end = std::chrono::steady_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(end - start).count();

    // ── Evaluate (std::optional) ──
    auto opt_evaluator = findEvaluator(task.eval_type);
    if (!opt_evaluator.has_value()) {
        std::cerr << "[HarnessRunner] Không tìm thấy evaluator: "
                  << task.eval_type << "\n";
        result.eval_feedback = "Evaluator không hợp lệ: " + task.eval_type;
        return result;
    }

    auto eval_result = opt_evaluator.value()->evaluate(result.agent_output, task.expected_keywords);
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

    return result;
}

// ── Phase 3: Record ─────────────────────────────────────────────────

bool HarnessRunner::exportResults(const std::vector<TaskRunResult>& results) const {
    // Tạo thư mục output nếu chưa có
    std::filesystem::create_directories(output_dir_);

    std::string filepath = output_dir_ + "/eval_results.json";
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "[HarnessRunner] Không thể tạo file: " << filepath << "\n";
        return false;
    }

    // Xuất JSON bằng nlohmann/json
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

    std::cout << "[HarnessRunner] Kết quả đã lưu tại: " << filepath << "\n";
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
