#pragma once

#include "../agent/agent_loop.h"
#include "Task.h"
#include "evaluator.h"
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace oop_agent {

// Forward declarations
class ToolRegistry;

struct TrajectoryStep {
  std::string thought;
  std::string action;
  std::string result;
  int tokens_used = 0;
  double latency_ms = 0.0;
};

/**
 * @brief Kết quả chạy một Task đơn lẻ, bao gồm cả thông tin eval.
 HarnessRunner là post-mortem analyzer, chạy xong xuất ra file json
 */
struct TaskRunResult {
  std::string task_id;                    // ID của task đã chạy
  std::string category;                   // Nhóm simple/medium/hard
  std::string agent_output;               // Output thực tế từ Agent
  bool evaluator_success = false;         // Kết quả thô từ Evaluator
  float evaluator_score = 0.0f;           // Điểm thô từ Evaluator (0.0 - 1.0)
  bool action_level_success = false;       // Agent có thực thi tool liên quan hay không
  float action_level_score = 0.0f;         // Điểm hành động (0.0 hoặc 1.0)
  bool success = false;                    // PASS cuối: evaluator và action-level đều đạt
  bool requires_tool = false;
  std::size_t tool_steps_count = 0;
  std::string failure_reason = "NONE";
  std::string eval_feedback;              // Nhận xét chi tiết từ Evaluator
  double latency_ms = 0.0;                // Thời gian chạy task (milliseconds)
  std::vector<TrajectoryStep> trajectory; // Trajectory của task này
};

/**
 * @brief Callback Observer — được gọi sau mỗi step trong AgentLoop.
 * Dùng để Harness ghi lại trajectory mà không cần AgentLoop biết Harness tồn
 * tại. (Observer Pattern)
 */
using StepHook =
    std::function<void(const std::string &thought, const std::string &action,
                       const std::string &result)>;

/**
 * @brief Bộ điều phối chạy benchmark: load task → chạy Agent → evaluate → ghi
 * kết quả.
 *
 * Luồng chính:
 *   1. loadTasks()       — Đọc tasks.json, parse thành TaskList
 *   2. runAll()          — Duyệt từng Task, gọi Agent, evaluate, lưu kết quả
 *   3. exportResults()   — Xuất kết quả ra file JSON (trajectory + score)
 *
 * Áp dụng:
 *   - Strategy Pattern: chọn Evaluator (Keyword / Functional) theo
 * evaluator_type
 *   - Observer Pattern: inject step_hook vào AgentLoop để ghi trajectory
 */
class HarnessRunner {
public:
  /**
   * @brief Khởi tạo HarnessRunner.
   * @param tasks_json_path Đường dẫn tới file tasks.json
   * @param output_dir      Thư mục xuất kết quả JSON
   */
  explicit HarnessRunner(const std::string &tasks_json_path,
                         const std::string &output_dir = "benchmark/results");

  ~HarnessRunner() = default;

  // Không cho copy, chỉ cho move
  HarnessRunner(const HarnessRunner &) = delete;
  HarnessRunner &operator=(const HarnessRunner &) = delete;
  HarnessRunner(HarnessRunner &&) = default;
  HarnessRunner &operator=(HarnessRunner &&) = default;

  // ── Phase 1: Setup ──────────────────────────────────────────────

  /**
   * @brief Load danh sách Task từ file tasks.json.
   * @return true nếu parse thành công, false nếu file lỗi hoặc không tồn tại.
   */
  bool loadTasks();

  /**
   * @brief Đăng ký một Evaluator strategy (keyword, functional, ...).
   * @param name Tên evaluator khớp với field "eval_type" trong tasks.json
   * @param evaluator Unique pointer tới Evaluator cụ thể
   */
  void registerEvaluator(const std::string &name,
                         std::unique_ptr<Evaluator> evaluator);

  // ── Phase 2: Run ────────────────────────────────────────────────

  /**
   * @brief Chạy toàn bộ danh sách task, evaluate từng cái.
   * @return Vector kết quả cho tất cả task.
   */
  std::vector<TaskRunResult> runAll();

  /**
   * @brief Chạy một Task đơn lẻ.
   * @param task Task cần chạy
   * @return Kết quả chạy + evaluate
   */
  TaskRunResult runSingle(const Task &task);

  // ── Phase 3: Record ─────────────────────────────────────────────

  /**
   * @brief Xuất toàn bộ kết quả ra file JSON trong output_dir.
   * @param results Vector kết quả từ runAll()
   * @return true nếu ghi file thành công.
   */
  bool exportResults(const std::vector<TaskRunResult> &results) const;

  /**
   * @brief Đăng ký Agent vào Harness.
   */
  void set_agent(AgentLoop *agent) noexcept { agent_ = agent; }

  // ── Accessors ───────────────────────────────────────────────────

  /**
   * @brief Lấy danh sách task đã load.
   */
  [[nodiscard]] const TaskList &getTasks() const noexcept { return tasks_; }

  /**
   * @brief Tính success rate tổng quát.
   * @param results Kết quả benchmark
   * @return Tỉ lệ pass (0.0 – 1.0)
   */
  [[nodiscard]] static float
  computeSuccessRate(const std::vector<TaskRunResult> &results);

  [[nodiscard]] static float
  computeEvaluatorScore(const std::vector<TaskRunResult> &results);

  [[nodiscard]] static float
  computeActionLevelScore(const std::vector<TaskRunResult> &results);

  /**
   * @brief Tạo step_hook để inject vào AgentLoop (Observer Pattern).
   * Harness sẽ ghi lại mọi step (thought/action/result) vào trajectory nội bộ.
   */
  [[nodiscard]] StepHook createStepHook();

private:
  /**
   * @brief Tìm Evaluator phù hợp theo evaluator_type.
   * @return std::optional chứa pointer tới Evaluator, hoặc std::nullopt nếu
   * không tìm thấy.
   */
  [[nodiscard]] std::optional<Evaluator *>
  findEvaluator(const std::string &evaluator_type) const;

  bool cleanBenchmarkArtifacts() const;
  [[nodiscard]] bool hasRelevantSuccessfulToolStep(const Task &task) const;
  [[nodiscard]] static std::string
  classifyFailure(const Task &task, const TaskRunResult &result);

  AgentLoop *agent_ = nullptr; // Con trỏ tới Agent để chạy task

  std::string tasks_json_path_; // Đường dẫn tới tasks.json
  std::string output_dir_;      // Thư mục xuất kết quả

  TaskList tasks_; // Danh sách task đã load

  // Map evaluator_type → Evaluator instance (Strategy Pattern)
  std::unordered_map<std::string, std::unique_ptr<Evaluator>> evaluators_;

  std::vector<TrajectoryStep> current_trajectory_;
  std::chrono::steady_clock::time_point last_step_time_;
};

} // namespace oop_agent
