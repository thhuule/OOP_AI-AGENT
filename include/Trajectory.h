#pragma once
#include <string>
#include <vector>

// Một bước trong agent run
struct Step {
  int step_number;          // bước thứ mấy
  std::string thought;      // agent nghĩ gì
  std::string action;       // tool nào được gọi
  std::string action_input; // input truyền vào tool
  std::string result;       // output tool trả về
  double latency_ms;        // thời gian chạy (milliseconds)
  int tokens_used;          // số token dùng trong bước này
};

// Toàn bộ 1 agent run
struct Trajectory {
  std::string task_id;      // ID của task trong benchmark
  std::string task_input;   // nội dung task
  std::vector<Step> steps;  // danh sách các bước
  std::string final_answer; // kết quả cuối cùng
  bool success;             // PASS hay FAIL
  double total_latency_ms;  // tổng thời gian chạy
  int total_tokens;         // tổng token toàn run
};