
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace oop_agent
{

class Tool;

class ToolRegistry
{
public:

    ToolRegistry();
    /**
     * @brief Đăng ký Tool vào hệ thống.
     */
    void register_tool(
        std::unique_ptr<Tool> tool);

    /**
     * @brief Tra cứu Tool theo tên.
     *
     * @return nullptr nếu không tìm thấy.
     */
    [[nodiscard]]
    Tool* get_tool(
        std::string_view name) const;

    /**
     * @brief Thiết lập danh sách Tool được phép sử dụng.
     */
    void set_allow_list(
        const std::vector<std::string>& names);

    /**
     * @brief Thiết lập danh sách Tool bị cấm.
     */
    void set_deny_list(
        const std::vector<std::string>& names);

    /**
     * @brief Kiểm tra Tool có được phép thực thi hay không.
     */
    [[nodiscard]]
    bool is_allowed(
        std::string_view name) const;

private:

    std::unordered_map<
        std::string,
        std::unique_ptr<Tool>
    > tools_;

    std::unordered_set<std::string> allow_list_;
    std::unordered_map<std::string, std::string> aliases_;
    std::unordered_set<std::string> deny_list_;
};

} // namespace oop_agent
