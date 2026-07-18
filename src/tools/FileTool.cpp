#include "FileTool.h"

#include <fstream>
#include <sstream>

namespace oop_agent {

// ── FileTool ────────────────────────────────────────────────────────

std::string_view FileTool::get_name() const noexcept {
    return "file";
}

std::string_view FileTool::get_description() const noexcept {
    return "Read or write files. Usage: read <path> | write <path> <content>";
}

std::expected<std::string, ToolError>
FileTool::execute(const std::string &arguments) {
    std::istringstream ss(arguments);
    std::string action;
    ss >> action;

    if (action == "read") {
        std::string path;
        ss >> path;
        std::ifstream file(path);
        if (!file) {
            return std::unexpected(ToolError::NotFound);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } else if (action == "write") {
        std::string path;
        ss >> path;
        std::string content;
        std::getline(ss, content);
        if (!content.empty() && content[0] == ' ') {
            content = content.substr(1);
        }
        std::ofstream file(path);
        if (!file) {
            return std::unexpected(ToolError::ExecutionFailed);
        }
        file << content;
        return "OK";
    }

    return std::unexpected(ToolError::InvalidArgument);
}

// ── FileReadTool ────────────────────────────────────────────────────

std::expected<std::string, ToolError>
FileReadTool::execute(const std::string &arguments) {
    std::ifstream file(arguments);
    if (!file) {
        return std::unexpected(ToolError::NotFound);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ── FileWriteTool ───────────────────────────────────────────────────

std::expected<std::string, ToolError>
FileWriteTool::execute(const std::string &arguments) {
    std::stringstream ss(arguments);
    std::string filename;
    ss >> filename;

    std::string content;
    std::getline(ss, content);
    if (!content.empty() && content[0] == ' ') {
        content = content.substr(1);
    }

    std::ofstream file(filename);
    if (!file) {
        return std::unexpected(ToolError::ExecutionFailed);
    }

    file << content;
    return "OK";
}

} // namespace oop_agent
