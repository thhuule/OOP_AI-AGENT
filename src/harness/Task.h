#pragma once

#include <string>
#include <vector>

namespace oop_agent {

/**
 * @brief Cấu trúc mô tả một bài test (Task) được load từ file tasks.json.
 * Mỗi Task chứa đầy đủ thông tin để Agent thực hiện và Evaluator chấm điểm.
 */
struct Task {
    std::string id;                 // Mã định danh duy nhất, ví dụ: "task_001"
    std::string description;        // Mô tả ngắn gọn mục đích của bài test
    std::string instruction;        // Câu lệnh / prompt gửi cho Agent xử lý
    std::string eval_type;          // Loại evaluator sử dụng: "keyword", "functional", ...
    std::string expected_keywords;  // Các keyword kỳ vọng trong output (dùng cho eval_type="keyword")
    std::string eval_script;        // Script kiểm tra (dùng cho eval_type="functional")
    std::string category;           // Phân loại độ khó: "simple", "medium", "hard"
    bool requires_tool = false;      // Task có bắt buộc thực thi tool thật hay không
    std::vector<std::string> required_tools; // Ít nhất một tool trong danh sách phải chạy thành công
    std::vector<std::string> artifacts;      // File sinh ra trong benchmark, dùng để clean state
    int max_steps = 10;             // Số bước tối đa Agent được phép thực hiện
};

/**
 * @brief Alias cho danh sách nhiều Task, tiện dùng khi load toàn bộ tasks.json.
 */
using TaskList = std::vector<Task>;

}
