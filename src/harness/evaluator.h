#pragma once

#include <string>
#include <string_view>
#include <expected>

namespace oop_agent {

/**
 * @brief Các mã lỗi có thể xảy ra trong quá trình Evaluator đánh giá kết quả.
 */
enum class EvalError {
    MissingTrajectory,  // Thiếu dữ liệu hành trình chạy (Trajectory) của Agent
    InvalidTaskSpec,    // Định dạng file cấu hình task/bài test không hợp lệ
    ExecutionTimeout,   // Quá thời gian chạy script kiểm tra (Functional Eval)
    UnknownError        // Lỗi không xác định khác
};

/**
 * @brief Cấu trúc lưu trữ kết quả chấm điểm chi tiết của một Task.
 */
struct EvalResult {
    bool is_passed = false;       // true nếu Agent hoàn thành đúng yêu cầu bài test
    float score = 0.0f;           // Thang điểm (ví dụ từ 0.0 đến 1.0 hoặc 0 đến 100)
    std::string feedback;         // Nhận xét chi tiết (ví dụ: "Thiếu file output.txt", "Sai từ khóa")
};

/**
 * @brief Interface thuần ảo của Bộ đánh giá (Evaluator) - Áp dụng Strategy Pattern.
 * Được dùng để tự động nghiệm thu xem đầu ra của Agent có đạt yêu cầu hay không.
 */
class Evaluator {
public:
    virtual ~Evaluator() = default;

    /**
     * @brief Lấy tên định danh của bộ đánh giá (Ví dụ: "keyword_eval", "functional_eval").
     */
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;

    /**
     * @brief Thực hiện đánh giá kết quả dựa trên đầu ra của Agent và dữ liệu mẫu.
     * @param agent_output Câu trả lời cuối cùng hoặc trạng thái kết quả của Agent Loop.
     * @param expected_output Đáp án mẫu hoặc tiêu chí kiểm tra lấy từ file tasks.json.
     * @return std::expected (C++23/26): Trả về cấu trúc EvalResult nếu chấm điểm thành công,
     * hoặc trả về mã lỗi EvalError nếu quá trình chấm điểm gặp sự cố kỹ thuật.
     */
    virtual std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output, 
        const std::string& expected_output
    ) = 0;
};

} 
