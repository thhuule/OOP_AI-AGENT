#pragma once

#include "Tool.h"

#include <expected>
#include <string>
#include <string_view>

namespace oop_agent
{

/**
 * @brief BNS-G-01 (phần B): action-tool an toàn cho GUI agent.
 *
 * Chỉ cho phép một tập hành động có giới hạn (allow-list):
 *   - `click <x> <y>`          — click tại toạ độ (x,y)
 *   - `type_text <text>`       — gõ chuỗi văn bản
 *   - `key_press <key>`        — nhấn phím trong danh sách an toàn
 *
 * Toạ độ/input được validate trước khi thực thi; không cho thực thi lệnh
 * shell/browser tuỳ ý. Hành động thật được tách qua virtual perform_action()
 * để test mock (invalid action path) và demo có kiểm soát không gây side
 * effect ngoài ý muốn.
 */
class ActionTool : public Tool
{
public:
    ActionTool() = default;
    ~ActionTool() override = default;

    [[nodiscard]]
    std::string_view get_name() const noexcept override;

    [[nodiscard]]
    std::string_view get_description() const noexcept override;

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;

    // ── Allow-list (expose để test / GUI layer kiểm tra) ──────────────────
    static constexpr int kMaxCoordinate = 100'000;
    static constexpr std::size_t kMaxTextLength = 512;

    static bool is_allowed_action(const std::string& action);

    static bool is_allowed_key(const std::string& key);

protected:
    /**
     * @brief Thực thi một hành động đã được validate.
     *
     * Virtual để test mock và demo có kiểm soát; default trả NotFound vì
     * không triển khai thao tác desktop thật (chỉ contract + validation).
     */
    virtual std::expected<std::string, ToolError>
    perform_action(
        const std::string& action,
        const std::string& payload);
};

} // namespace oop_agent
