
#include "ToolRegistry.h"
#include "Tool.h"

namespace oop_agent
{
ToolRegistry::ToolRegistry()
{
    aliases_["calculate"] = "calculator";
    aliases_["exec"] = "execute_shell";
    aliases_["google_search"] = "web_search";
    aliases_["create_file"] = "write_file";
}
void ToolRegistry::register_tool(
    std::unique_ptr<Tool> tool)
{
    if (!tool)
    {
        return;
    }

    tools_[std::string(tool->get_name())]
        = std::move(tool);
}

Tool* ToolRegistry::get_tool(
    std::string_view name) const
{
    std::string tool_name(name);

    auto alias = aliases_.find(tool_name);

    if (alias != aliases_.end())
    {
        tool_name = alias->second;
    }

    auto it = tools_.find(tool_name);

    if (it == tools_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

void ToolRegistry::set_allow_list(
    const std::vector<std::string>& names)
{
    allow_list_.clear();

    for (const auto& name : names)
    {
        allow_list_.insert(name);
    }
}

void ToolRegistry::set_deny_list(
    const std::vector<std::string>& names)
{
    deny_list_.clear();

    for (const auto& name : names)
    {
        deny_list_.insert(name);
    }
}

bool ToolRegistry::is_allowed(
    std::string_view name) const
{
    std::string tool_name(name);
    auto alias = aliases_.find(tool_name);

    if (alias != aliases_.end())
    {
        tool_name = alias->second;
    }
    // Nếu Allow List không rỗng thì chỉ Tool
    // nằm trong danh sách mới được phép chạy.
    if (!allow_list_.empty())
    {
        if (allow_list_.find(tool_name)
            == allow_list_.end())
        {
            return false;
        }
    }

    // Tool nằm trong Deny List sẽ bị chặn.
    if (deny_list_.find(tool_name)
        != deny_list_.end())
    {
        return false;
    }

    return true;
}

} // namespace oop_agent