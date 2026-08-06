#pragma once
#include "Tool.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>   // C++17
#include <set>
#include <string>
#include "Registry.h"

namespace oop_agent {

/// ToolCreator: factory function that produces a fresh Tool instance.
using ToolCreator = std::function<std::unique_ptr<Tool>()>;

/// ToolRegistry — Registry + Factory pattern (bắt buộc theo đề).
///
/// Two responsibilities (kept in one class per project scope):
///   1. FACTORY  : register_creator() + create()  → makes new unique_ptr<Tool> by name
///   2. REGISTRY : register_tool()    + lookup()  → stores & retrieves existing instances
///
/// Alias resolution and allow/deny policy are applied before any lookup.
/// Alias normalize must happen BEFORE allow-list / deny-list check.
class ToolRegistry {
public:
    // ── Factory interface ─────────────────────────────────────────────────

    /// Register a creator function for a canonical tool name.
    /// Overwrites any existing creator for the same name.
    bool register_creator(const std::string& canonical_name, ToolCreator creator);

    /// Create a fresh tool instance by name (alias-resolved, policy-checked).
    /// Returns nullptr if name unknown or denied.
    std::unique_ptr<Tool> create(const std::string& name);

    // ── Registry interface ────────────────────────────────────────────────

    /// Register an already-constructed tool instance.
    bool register_tool(std::shared_ptr<Tool> tool);

    /// Register or override a tool instance by canonical name.
    void set_tool(std::shared_ptr<Tool> tool);

    /// Lookup a registered instance by canonical name.
    /// Returns nullptr if not found.
    Tool* lookup(const std::string& name);

    // ── Alias & policy ────────────────────────────────────────────────────

    /// Register alias → canonical mapping.
    bool register_alias(const std::string& alias, const std::string& canonical);

    /// Resolve alias to canonical name; returns original if no alias found.
    std::string normalize(const std::string& name) const;

    /// Returns true if canonical name is not in deny_list_ and
    /// (allow_list_ is empty OR name is in allow_list_).
    bool is_allowed(const std::string& canonical_name) const;

    /// Add to deny-list (takes canonical name).
    void deny(const std::string& canonical_name);

    /// Add to allow-list. If allow-list is non-empty, ONLY listed tools are permitted.
    void allow(const std::string& canonical_name);

    /// Register all built-in tools (creators + instances + aliases).
    void register_all_tools();

    // ── Inspection ────────────────────────────────────────────────────────
    bool has_creator(const std::string& name) const;
    bool has_instance(const std::string& name) const;

private:
    std::map<std::string, ToolCreator>            creators_;
    Registry<Tool> registry_;
    std::map<std::string, std::string>            aliases_;
    std::set<std::string>                         allow_list_;
    std::set<std::string>                         deny_list_;
};

} // namespace oop_agent