#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace oop_agent
{

class Tool;

class ToolRegistry
{
public:

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

private:

    std::unordered_map<
        std::string,
        std::unique_ptr<Tool>
    > tools_;
};

}
