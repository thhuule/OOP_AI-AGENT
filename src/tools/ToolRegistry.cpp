#include "ToolRegistry.h"
#include "Tool.h"

namespace oop_agent
{

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
    auto it = tools_.find(std::string(name));

    if (it == tools_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

}
