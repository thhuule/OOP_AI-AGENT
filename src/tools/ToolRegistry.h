#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace oop_agent
{
    class Tool;

    /**
     * @brief Quản lý tập trung toàn bộ Tool trong hệ thống.
     *
     * Mục đích:
     * - Đăng ký Tool vào hệ thống.
     * - Tra cứu Tool theo tên.
     * - Tách Agent khỏi implementation cụ thể của từng Tool.
     *
     * Ví dụ:
     *   "calculator" -> CalculatorTool
     *   "file"       -> FileTool
     */
    class ToolRegistry
    {
    public:

        /**
         * @brief Đăng ký Tool mới.
         *
         * Nếu Tool có cùng tên đã tồn tại
         * thì Tool mới sẽ ghi đè Tool cũ.
         */
        void register_tool(std::unique_ptr<Tool> tool);

        /**
         * @brief Tìm Tool theo tên.
         *
         * Trường hợp tìm thấy:
         *   Trả về con trỏ tới Tool.
         *
         * Trường hợp không tìm thấy:
         *   Trả về nullptr.
         */
        [[nodiscard]]
        Tool* get_tool(std::string_view name) const;

    private:

        std::unordered_map<
            std::string,
            std::unique_ptr<Tool>
        > tools_;
    };

}
