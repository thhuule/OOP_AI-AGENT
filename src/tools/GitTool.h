#pragma once

#include "Tool.h"

namespace oop_agent
{

class GitTool final : public Tool
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

private:

    std::expected<std::string, ToolError>
    run_command(const std::string& command);
};

}