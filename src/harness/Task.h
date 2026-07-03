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
    std::string category;           // Phân loại độ khó: "simple", "medium", "hard"
    std::string description;        // Mô tả ngắn gọn mục đích của bài test
    std::string input;              // Câu lệnh / prompt gửi cho Agent xử lý
    std::string expected_tool;      // Tool mà Agent được kỳ vọng sẽ gọi, ví dụ: "exec", "file_read"
    std::string expected_keywords;  // Các keyword kỳ vọng trong output, phân cách bằng dấu phẩy
    std::string evaluator_type;     // Loại evaluator sử dụng: "keyword", "functional", ...
};

/**
 * @brief Alias cho danh sách nhiều Task, tiện dùng khi load toàn bộ tasks.json.
 */
using TaskList = std::vector<Task>;

}
