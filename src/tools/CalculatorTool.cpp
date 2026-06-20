#include "CalculatorTool.h"

#include <sstream>

namespace oop_agent
{

std::string_view
CalculatorTool::get_name() const noexcept
{
    return "calculator";
}

std::string_view
CalculatorTool::get_description() const noexcept
{
    return "Basic arithmetic calculator";
}

std::expected<std::string, ToolError>
CalculatorTool::execute(
    const std::string& arguments)
{
    std::stringstream ss(arguments);

    double lhs {};
    double rhs {};
    char op {};

    if (!(ss >> lhs >> op >> rhs))
    {
        return std::unexpected(
            ToolError::InvalidArguments);
    }

    double result {};

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
                ToolError::InvalidArguments);
    }

    return std::to_string(result);
}

}
