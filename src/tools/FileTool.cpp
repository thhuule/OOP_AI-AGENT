#include "FileTool.h"

#include <fstream>
#include <sstream>

namespace oop_agent {

// ── FileTool ────────────────────────────────────────────────────────

std::string_view FileTool::get_name() const noexcept
{
    return "file";
}

std::string_view FileTool::get_description() const noexcept
{
    return "Read or write files. Usage: read <path> | write <path> <content>";
}

std::expected<std::string, ToolError>
FileTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        std::istringstream ss(arguments);

        std::string action;
        ss >> action;

        if (action == "read")
        {
            std::string path;
            ss >> path;

            if (path.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            std::ifstream file(path);

            if (!file)
            {
                return std::unexpected(
                    ToolError::NotFound);
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            return buffer.str();
        }

        if (action == "write")
        {
            std::string path;
            ss >> path;

            if (path.empty())
            {
                return std::unexpected(
                    ToolError::InvalidArgument);
            }

            std::string content;
            std::getline(ss, content);

            if (!content.empty() &&
                content.front() == ' ')
            {
                content.erase(0, 1);
            }

            std::ofstream file(path);

            if (!file)
            {
                return std::unexpected(
                    ToolError::ExecutionFailed);
            }

            file << content;

            return "OK";
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

// ── FileReadTool ────────────────────────────────────────────────────

std::expected<std::string, ToolError>
FileReadTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        std::ifstream file(arguments);

        if (!file)
        {
            return std::unexpected(
                ToolError::NotFound);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

// ── FileWriteTool ───────────────────────────────────────────────────

std::expected<std::string, ToolError>
FileWriteTool::execute(const std::string& arguments)
{
    try
    {
        if (arguments.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        std::stringstream ss(arguments);

        std::string filename;
        ss >> filename;

        if (filename.empty())
        {
            return std::unexpected(
                ToolError::InvalidArgument);
        }

        std::string content;
        std::getline(ss, content);

        if (!content.empty() &&
            content.front() == ' ')
        {
            content.erase(0, 1);
        }

        std::ofstream file(filename);

        if (!file)
        {
            return std::unexpected(
                ToolError::ExecutionFailed);
        }

        file << content;

        return "OK";
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent