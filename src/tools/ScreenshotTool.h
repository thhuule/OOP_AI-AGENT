#pragma once

#include "Tool.h"

#include <expected>
#include <string>
#include <string_view>

namespace oop_agent
{

/**
 * @brief BNS-G-01 (phần B): contract `capture_screenshot`.
 *
 * Tool này chụp màn hình hiện tại và trả về ảnh dạng base64 để gửi cho VLM
 * qua cùng interface client. Quyền truy cập màn hình do môi trường quyết định;
 * nếu không thể chụp, trả ToolError ổn định (không throw/crash).
 *
 * An toàn: không thực thi hành động nào ngoài việc đọc pixel; không ghi file
 * người dùng. Phương thức chụp được tách virtual để test inject một capture
 * giả lập, tránh phụ thuộc môi trường desktop/display.
 */
class ScreenshotTool : public Tool
{
public:
    ScreenshotTool() = default;
    ~ScreenshotTool() override = default;

    [[nodiscard]]
    std::string_view get_name() const noexcept override;

    [[nodiscard]]
    std::string_view get_description() const noexcept override;

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;

    /// Mã hoá nhị phân → base64 (thuần C++, deterministic, không mạng).
    /// Public vì là tiện ích thuần và được dùng trong focused test.
    static std::string base64_encode(
        const std::string& bytes);

protected:
    /**
     * @brief Chụp ảnh màn hình, trả về dữ liệu PNG dạng nhị phân.
     *
     * Virtual để test offline inject capture giả lập; default dùng công cụ
     * hệ thống (macOS `screencapture`) khi có display.
     */
    virtual std::expected<std::string, ToolError>
    capture_png();
};

} // namespace oop_agent
