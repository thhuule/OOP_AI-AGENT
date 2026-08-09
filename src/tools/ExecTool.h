#pragma once

#include "Tool.h"

#include <chrono>
#include <expected>
#include <string>
#include <string_view>

namespace oop_agent
{

/**
 * @brief Tool thực thi lệnh shell.
 *
 * Ví dụ:
 *   ls -la
 *   pwd
 *   whoami
 */
class ExecTool : public Tool
{
public:
    ExecTool() = default;

    /**
     * @brief Tạo ExecTool với timeout tuỳ chỉnh (milliseconds).
     *        Constructor này phục vụ test offline: chạy lệnh nhanh, đảm bảo
     *        không phụ thuộc mạng và không phải chờ timeout mặc định 10s.
     */
    explicit ExecTool(std::chrono::milliseconds timeout);

    ~ExecTool() override = default;

    /**
     * @brief Tên định danh của Tool.
     */
    [[nodiscard]]
    std::string_view get_name() const noexcept override;

    /**
     * @brief Mô tả chức năng Tool.
     */
    [[nodiscard]]
    std::string_view get_description() const noexcept override;

    /**
     * @brief Thực thi lệnh shell.
     *
     * @param arguments Lệnh cần thực thi.
     * @return Kết quả stdout + stderr nếu thành công,
     *         hoặc ToolError nếu thất bại.
     */
    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;

private:
    /// Timeout tối đa cho một lệnh (mặc định 10 giây).
    std::chrono::milliseconds timeout_{10'000};
};

} // namespace oop_agent