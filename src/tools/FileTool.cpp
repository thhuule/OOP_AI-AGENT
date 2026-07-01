#include "FileTool.h"

#include <fstream>
#include <sstream>

namespace oop_agent
{

std::string_view
FileTool::get_name() const noexcept
{
    return "file";
}

std::string_view
FileTool::get_description() const noexcept
{
    return "Read and write files";
}

std::expected<std::string, ToolError>
FileTool::execute(
    const std::string& arguments)
{
    std::stringstream ss(arguments);

    std::string command;
    ss >> command;

    if (command == "read")
    {
        std::string filename;
        ss >> filename;

        std::ifstream file(filename);

        if (!file)
        {
            return std::unexpected(
                ToolError::NotFound);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    if (command == "write")
    {
        std::string filename;
        ss >> filename;

        std::string content;
        std::getline(ss, content);

        std::ofstream file(filename);

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

}
