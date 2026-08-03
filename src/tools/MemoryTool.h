#pragma once

#include "Tool.h"
#include <sqlite3.h>
#include <string>

namespace oop_agent
{

class MemoryTool final : public Tool
{
public:
    MemoryTool();
    ~MemoryTool() override;

    [[nodiscard]]
    std::string_view
    get_name() const noexcept override;

    [[nodiscard]]
    std::string_view
    get_description() const noexcept override;

    std::expected<std::string, ToolError>
    execute(const std::string& arguments) override;

private:
    sqlite3* db_;

    bool init_database();

    std::expected<std::string, ToolError>
    save_memory(const std::string& text);

    std::expected<std::string, ToolError>
    search_memory(const std::string& keyword);
};

}