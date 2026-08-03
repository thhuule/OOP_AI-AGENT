#include "GitTool.h"

#include <cstdio>
#include <array>

namespace oop_agent
{

std::string_view
GitTool::get_name() const noexcept
{
    return "git";
}

std::string_view
GitTool::get_description() const noexcept
{
    return
        "Run git commands.\n"
        "Supported:\n"
        "status\n"
        "branch\n"
        "log\n"
        "diff";
}

std::expected<std::string, ToolError>
GitTool::run_command(
    const std::string& command)
{
    FILE* pipe =
        popen(command.c_str(), "r");

    if (!pipe)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }

    std::array<char,256> buffer;

    std::string output;

    while (fgets(
        buffer.data(),
        buffer.size(),
        pipe))
    {
        output += buffer.data();
    }

    pclose(pipe);

    return output;
}

std::expected<std::string, ToolError>
GitTool::execute(
    const std::string& arguments)
    {
        try
        {
            if (arguments.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            if (arguments == "status")
            {
                return run_command(
                    "git status");
            }

            if (arguments == "branch")
            {
                return run_command(
                    "git branch");
            }

            if (arguments == "log")
            {
                return run_command(
                    "git log --oneline -10");
            }

            if (arguments == "diff")
            {
                return run_command(
                    "git diff");
            }

            return std::unexpected(
                ToolError::InvalidArgument);
        }
        catch (...)
        {
            return std::unexpected(
                ToolError::ExecutionFailed);
        }
    }

}