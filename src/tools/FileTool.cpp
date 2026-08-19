#include "FileTool.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string_view>

namespace oop_agent {
namespace {

struct FileArguments {
    std::string filename;
    std::string content;
    bool append = false;
};

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    value = value.substr(first, last - first + 1);
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::optional<std::string> parseReadPath(const std::string& arguments) {
    try {
        const auto json = nlohmann::json::parse(arguments);
        if (json.is_object()) {
            for (const char* key : {"filename", "path", "file"}) {
                if (json.contains(key) && json[key].is_string()) {
                    auto path = trim(json[key].get<std::string>());
                    if (!path.empty())
                        return path;
                }
            }
            return std::nullopt;
        }
        if (json.is_string()) {
            auto path = trim(json.get<std::string>());
            return path.empty() ? std::nullopt
                                : std::optional<std::string>{std::move(path)};
        }
    } catch (const nlohmann::json::exception&) {
    }

    auto path = trim(arguments);
    return path.empty() ? std::nullopt
                        : std::optional<std::string>{std::move(path)};
}

std::optional<FileArguments> parseWriteArguments(const std::string& arguments,
                                                  bool force_append) {
    FileArguments parsed;
    parsed.append = force_append;

    // Detect JSON intent: if the trimmed input starts with '{', treat the
    // entire input as JSON.  A parse failure here must NOT fall through to
    // the text/comma parser — return nullopt so the caller surfaces
    // ToolError::InvalidArgument, matching the contract expected by tests.
    const auto first_non_ws = arguments.find_first_not_of(" \t\r\n");
    const bool looks_like_json =
        (first_non_ws != std::string::npos && arguments[first_non_ws] == '{');

    try {
        const auto json = nlohmann::json::parse(arguments);
        if (json.is_object()) {
            for (const char* key : {"filename", "path", "file"}) {
                if (json.contains(key) && json[key].is_string()) {
                    parsed.filename = trim(json[key].get<std::string>());
                    break;
                }
            }
            if (json.contains("content") && json["content"].is_string())
                parsed.content = json["content"].get<std::string>();
            else if (json.contains("text") && json["text"].is_string())
                parsed.content = json["text"].get<std::string>();
            else
                return std::nullopt;

            parsed.append = parsed.append || json.value("append", false) ||
                json.value("mode", std::string{}) == "append";
            if (parsed.filename.empty())
                return std::nullopt;
            return parsed;
        }
    } catch (const nlohmann::json::exception&) {
        // If the input looked like JSON but failed to parse → invalid JSON
        // argument; do not fall through to the text/comma parser.
        if (looks_like_json)
            return std::nullopt;
    }

    // Only reach here for non-JSON inputs (plain text / comma protocol).
    // Canonical text protocol: split only at the first comma so content may
    // contain spaces and additional commas without changing the filename.
    const auto comma = arguments.find(',');
    if (comma != std::string::npos) {
        parsed.filename = trim(arguments.substr(0, comma));
        parsed.content = arguments.substr(comma + 1);
    } else {
        std::istringstream input(arguments);
        input >> parsed.filename;
        std::getline(input, parsed.content);
        if (!parsed.content.empty() && parsed.content.front() == ' ')
            parsed.content.erase(0, 1);
        parsed.filename = trim(parsed.filename);
    }

    if (parsed.filename.empty())
        return std::nullopt;
    return parsed;
}

std::expected<std::string, ToolError>
writeFile(const std::string& arguments, bool append) {
    const auto parsed = parseWriteArguments(arguments, append);
    if (!parsed)
        return std::unexpected(ToolError::InvalidArgument);

    const auto mode = std::ios::out |
        (parsed->append ? std::ios::app : std::ios::trunc);
    std::ofstream file(parsed->filename, mode);
    if (!file)
        return std::unexpected(ToolError::ExecutionFailed);
    file << parsed->content;
    if (!file)
        return std::unexpected(ToolError::ExecutionFailed);

    return std::string("OK: ") + (parsed->append ? "appended " : "wrote ") +
           parsed->filename;
}

} // namespace

// ── FileTool ────────────────────────────────────────────────────────

std::string_view FileTool::get_name() const noexcept
{
    return "file";
}

std::string_view FileTool::get_description() const noexcept
{
    return "Read or write files. Usage: read <path> or write <path> <content>. "
           "Example: write result.txt 1081";
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
        const auto path = parseReadPath(arguments);
        if (!path)
            return std::unexpected(ToolError::InvalidArgument);

        std::ifstream file(*path);

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
        return writeFile(arguments, false);
    }
    catch (...)
    {
        return std::unexpected(
            ToolError::ExecutionFailed);
    }
}

std::expected<std::string, ToolError>
FileAppendTool::execute(const std::string& arguments)
{
    try
    {
        return writeFile(arguments, true);
    }
    catch (...)
    {
        return std::unexpected(ToolError::ExecutionFailed);
    }
}

} // namespace oop_agent
