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
    get_name() const noexcept override { return "file_read"; }

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override { return "Read file contents"; }

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;
};

class FileWriteTool final : public Tool
{
public:
    [[nodiscard]]
    std::string_view
    get_name() const noexcept override { return "file_write"; }

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override { return "Write content to a file"; }

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;
};

}

