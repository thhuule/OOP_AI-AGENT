#pragma once

#include "Tool.h"

namespace oop_agent
{

class FileTool final : public Tool
{
public:

    [[nodiscard]]
    std::string_view
    get_name() const noexcept override;

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override;

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;
};

class FileReadTool final : public Tool
{
public:
    [[nodiscard]]
    std::string_view
    get_name() const noexcept override { return "read_file"; }

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override { return "Read a file. Example: notes.txt"; }

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;
};

class FileWriteTool final : public Tool
{
public:
    [[nodiscard]]
    std::string_view
    get_name() const noexcept override { return "write_file"; }

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override { return "Write a file. Example: hello.txt Hello world"; }

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;
};

}

