#include "FunctionalEvaluator.h"
#include <iostream>

namespace oop_agent {

std::expected<EvalResult, EvalError> FunctionalEvaluator::evaluate(
    const std::string& agent_output,
    const std::string& expected_output
) {
    (void)agent_output; // functional eval chạy script độc lập, không dùng agent_output

    if (expected_output.empty()) {
        return std::unexpected(EvalError::InvalidTaskSpec);
    }

    // Sử dụng ExecTool đã được inject để thực thi lệnh
    auto exec_result = exec_tool_->execute(expected_output);

    if (!exec_result.has_value()) {
        // ExecTool trả về lỗi (timeout, execution failed, ...)
        ToolError err = exec_result.error();
        std::cerr << "[FunctionalEvaluator] ExecTool thất bại cho lệnh: "
                  << expected_output
                  << " (ToolError: " << static_cast<int>(err) << ")"
                  << std::endl;
        return std::unexpected(EvalError::ExecutionTimeout);
    }

    // Phân tích output từ ExecTool
    const std::string& cmd_output = exec_result.value();

    EvalResult result;
    if (cmd_output.find("PASS") != std::string::npos) {
        result.is_passed = true;
        result.score = 1.0f;
        result.feedback = "Đánh giá chức năng thành công (Tìm thấy 'PASS')";
    } else {
        result.is_passed = false;
        result.score = 0.0f;
        result.feedback = "Đánh giá chức năng thất bại (Không tìm thấy 'PASS' trong output)";
    }

    return result;
}

}
