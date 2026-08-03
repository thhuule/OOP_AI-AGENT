#include "CalculatorTool.h"

#include <sstream>

namespace oop_agent {

std::string_view CalculatorTool::get_name() const noexcept
{
    return "calculator";
}

std::string_view CalculatorTool::get_description() const noexcept
{
    return "Evaluate a mathematical expression. Example: 2+3*5";
}

std::expected<std::string, ToolError>
CalculatorTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        std::stringstream ss(arguments);

        double lhs{};
        double rhs{};
        char op{};

        if (!(ss >> lhs >> op >> rhs))
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        double result{};

        switch (op)
        {
        case '+':
            result = lhs + rhs;
            break;

        case '-':
            result = lhs - rhs;
            break;

        case '*':
            result = lhs * rhs;
            break;

        case '/':
        {
            if (rhs == 0)
            {
                return std::unexpected(
                    ToolError::ExecutionFailed);
            }

            result = lhs / rhs;
            break;
        }

        default:
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        return std::to_string(result);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent
