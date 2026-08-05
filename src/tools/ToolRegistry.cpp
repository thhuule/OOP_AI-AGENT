#include "ToolRegistry.h"

// Include all concrete tools
#include "CalculatorTool.h"
#include "FileTool.h"
#include "ExecTool.h"
#include "WebSearchTool.h"
#include "MemoryTool.h"
#include "TimeTool.h"
#include "JsonTool.h"
#include "GitTool.h"

#include <print>    // C++23

namespace oop_agent {

// ── Factory ───────────────────────────────────────────────────────────────────

bool ToolRegistry::register_creator(
    const std::string& canonical_name,
    ToolCreator creator)
{
    if (creators_.find(canonical_name) != creators_.end())
        return false;

    creators_[canonical_name] = std::move(creator);
    return true;
}

std::unique_ptr<Tool> ToolRegistry::create(const std::string& name) {
    const std::string canonical = normalize(name);
    if (!is_allowed(canonical)) {
        std::println("[ToolRegistry] create() denied: {}", canonical);
        return nullptr;
    }
    auto it = creators_.find(canonical);
    if (it == creators_.end()) {
        std::println("[ToolRegistry] create() unknown tool: {}", canonical);
        return nullptr;
    }
    return it->second();    // call creator → fresh unique_ptr<Tool>
}

// ── Registry ──────────────────────────────────────────────────────────────────

bool ToolRegistry::register_tool(std::shared_ptr<Tool> tool)
{
    if (!tool)
        return false;

    return registry_.register_item(
        std::string(tool->get_name()),
        std::move(tool));
}

Tool* ToolRegistry::lookup(const std::string& name)
{
    auto canonical = normalize(name);

    if (!is_allowed(canonical))
        return nullptr;

    return registry_.get(canonical);
}

// ── Alias & policy ────────────────────────────────────────────────────────────

bool ToolRegistry::register_alias(
    const std::string& alias,
    const std::string& canonical)
{
    if (aliases_.contains(alias))
        return false;

    aliases_[alias] = canonical;
    return true;
}

std::string ToolRegistry::normalize(const std::string& name) const {
    auto it = aliases_.find(name);
    return (it != aliases_.end()) ? it->second : name;
}

bool ToolRegistry::is_allowed(const std::string& canonical_name) const {
    if (deny_list_.count(canonical_name)) return false;
    if (!allow_list_.empty() && !allow_list_.count(canonical_name)) return false;
    return true;
}

void ToolRegistry::deny(const std::string& canonical_name) {
    deny_list_.insert(canonical_name);
}

void ToolRegistry::allow(const std::string& canonical_name) {
    allow_list_.insert(canonical_name);
}

bool ToolRegistry::has_creator(const std::string& name) const {
    return creators_.count(normalize(name)) > 0;
}

bool ToolRegistry::has_instance(const std::string& name) const
{
    return registry_.contains(normalize(name));
}

// ── register_all_tools ────────────────────────────────────────────────────────

void ToolRegistry::register_all_tools() {
    // ── Creators (Factory) ────────────────────────────────────────────────
    register_creator("calculator",    [] { return std::make_unique<CalculatorTool>(); });
    register_creator("file",          [] { return std::make_unique<FileTool>(); });
    register_creator("read_file",     [] { return std::make_unique<FileReadTool>(); });
    register_creator("write_file",    [] { return std::make_unique<FileWriteTool>(); });
    register_creator("append_file",   [] { return std::make_unique<FileAppendTool>(); });
    register_creator("execute_shell", [] { return std::make_unique<ExecTool>(); });
    register_creator("web_search",    [] { return std::make_unique<WebSearchTool>(); });
    register_creator("memory",        [] { return std::make_unique<MemoryTool>(); });
    register_creator("time",          [] { return std::make_unique<TimeTool>(); });
    register_creator("json",          [] { return std::make_unique<JsonTool>(); });
    register_creator("git",           [] { return std::make_unique<GitTool>(); });

    // ── Instances (Registry) ─────────────────────────────────────────────
    // Pre-instantiate all tools so lookup() works without create()
    register_tool(std::make_shared<CalculatorTool>());
    register_tool(std::make_shared<FileTool>());
    register_tool(std::make_shared<FileReadTool>());    // "read_file"
    register_tool(std::make_shared<FileWriteTool>());   // "write_file"
    register_tool(std::make_shared<FileAppendTool>());  // "append_file"
    register_tool(std::make_shared<ExecTool>());
    register_tool(std::make_shared<WebSearchTool>());
    register_tool(std::make_shared<MemoryTool>());
    register_tool(std::make_shared<TimeTool>());
    register_tool(std::make_shared<JsonTool>());
    register_tool(std::make_shared<GitTool>());

    // ── Aliases (normalize before lookup or policy check) ─────────────────
    // These aliases point to the dedicated specialized tools,
    // not the generic FileTool, so they work correctly.
    register_alias("calculate",     "calculator");
    register_alias("exec",          "execute_shell");
    register_alias("google_search", "web_search");
    register_alias("create_file",   "write_file");   // → FileWriteTool
    // "read_file", "write_file", "append_file" are canonical names now —
    // no alias needed (they are registered directly above).
}

} // namespace oop_agent