#pragma once

#include "Tool.h"

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
};

} // namespace oop_agent