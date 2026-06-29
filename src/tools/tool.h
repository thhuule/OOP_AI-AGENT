#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <expected>
#include <map>

namespace oop_agent {

/**
 * @brief Định nghĩa các mã lỗi có thể xảy ra trong quá trình thực thi Tool.
 */
enum class ToolError {
    InvalidArgument,   // Tham số truyền vào Tool không hợp lệ (sai định dạng, thiếu trường...)
    ExecutionFailed,   // Thực thi thất bại (lỗi logic tính toán, lệnh shell lỗi...)
    AccessDenied,      // Bị từ chối quyền truy cập (vi phạm Tool Policy)
    NotFound,          // Không tìm thấy tài nguyên hệ thống cần thiết
    UnknownError       // Các lỗi phát sinh không xác định khác
};

/**
 * @brief Lớp trừu tượng cơ sở (Abstract Interface) cho tất cả các Công cụ (Tools).
 * Áp dụng mẫu thiết kế Strategy Pattern.
 */
class Tool {
public:
    virtual ~Tool() = default;

    /**
     * @brief Lấy tên định danh duy nhất của Tool (Ví dụ: "calculator", "exec").
     * @return std::string_view (C++17/20/23/26) tối ưu bộ nhớ, tránh sao chép chuỗi.
     */
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;

    /**
     * @brief Lấy chuỗi mô tả chức năng và hướng dẫn LLM cách dùng Tool (JSON Schema).
     */
    [[nodiscard]] virtual std::string_view get_description() const noexcept = 0;

    /**
     * @brief Hàm thực thi logic cốt lõi của Tool.
     * @param arguments Chuỗi chứa các tham số đầu vào (thường là định dạng JSON do LLM trích xuất).
     * @return std::expected (C++23/26): Trả về kết quả dạng chuỗi văn bản (string) nếu chạy thành công,
     * hoặc trả về mã lỗi ToolError nếu thất bại.
     */
    virtual std::expected<std::string, ToolError> execute(const std::string& arguments) = 0;
};

} // namespace oop_agent
