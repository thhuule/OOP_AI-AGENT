#include "JsonTool.h"

#include <nlohmann/json.hpp>

namespace oop_agent
{

using json = nlohmann::json;

std::string_view
JsonTool::get_name() const noexcept
{
    return "json";
}

std::string_view
JsonTool::get_description() const noexcept
{
    return "Parse and pretty print JSON.";
}

std::expected<std::string, ToolError>
JsonTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        json j = json::parse(arguments);

        return j.dump(4);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

}