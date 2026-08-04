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

void ToolRegistry::register_creator(const std::string& canonical_name,
                                    ToolCreator creator) {
    creators_[canonical_name] = std::move(creator);
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

void ToolRegistry::register_tool(std::shared_ptr<Tool> tool) {
    if (!tool) return;
    const std::string name = std::string(tool->get_name());
    instances_[name] = std::move(tool);
}

Tool* ToolRegistry::lookup(const std::string& name) {
    const std::string canonical = normalize(name);
    if (!is_allowed(canonical)) {
        return nullptr;
    }
    auto it = instances_.find(canonical);
    if (it == instances_.end()) return nullptr;
    return it->second.get();
}

// ── Alias & policy ────────────────────────────────────────────────────────────

void ToolRegistry::register_alias(const std::string& alias,
                                  const std::string& canonical) {
    aliases_[alias] = canonical;
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

bool ToolRegistry::has_instance(const std::string& name) const {
    return instances_.count(normalize(name)) > 0;
}

// ── register_all_tools ────────────────────────────────────────────────────────

void ToolRegistry::register_all_tools() {
    // ── Creators (Factory) ────────────────────────────────────────────────
    register_creator("calculator",    [] { return std::make_unique<CalculatorTool>(); });
    register_creator("file",          [] { return std::make_unique<FileTool>(); });
    register_creator("execute_shell", [] { return std::make_unique<ExecTool>(); });
    register_creator("web_search",    [] { return std::make_unique<WebSearchTool>(); });
    register_creator("memory",        [] { return std::make_unique<MemoryTool>(); });
    register_creator("time",          [] { return std::make_unique<TimeTool>(); });
    register_creator("json",          [] { return std::make_unique<JsonTool>(); });
    register_creator("git",           [] { return std::make_unique<GitTool>(); });

    // ── Instances (Registry) ─────────────────────────────────────────────
    // Pre-instantiate tools used frequently so lookup() works without create()
    register_tool(std::make_unique<CalculatorTool>());
    register_tool(std::make_unique<FileTool>());
    register_tool(std::make_unique<ExecTool>());
    register_tool(std::make_unique<WebSearchTool>());
    register_tool(std::make_unique<MemoryTool>());
    register_tool(std::make_unique<TimeTool>());
    register_tool(std::make_unique<JsonTool>());
    register_tool(std::make_unique<GitTool>());

    // ── Aliases (normalize before lookup or policy check) ─────────────────
    register_alias("calculate",     "calculator");
    register_alias("exec",          "execute_shell");
    register_alias("google_search", "web_search");
    register_alias("create_file",   "file");
    register_alias("write_file",    "file");
    register_alias("read_file",     "file");
    register_alias("append_file",   "file");
}

} // namespace oop_agent