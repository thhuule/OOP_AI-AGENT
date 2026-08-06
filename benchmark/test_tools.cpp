#include <cassert>
#include <iostream>
#include "tools/ToolRegistry.h"
#include "tools/CalculatorTool.h"
#include "tools/FileTool.h"
#include "tools/ExecTool.h"
#include "tools/WebSearchTool.h"
#include "tools/MemoryTool.h"
#include "tools/TimeTool.h"
#include "tools/JsonTool.h"
#include "tools/GitTool.h"

using namespace oop_agent;

void test_registry_instance_registration() {
    std::cout << "[TEST] Running test_registry_instance_registration...\n";
    ToolRegistry reg;
    
    // Test null registration
    assert(!reg.register_tool(nullptr));
    
    // Test valid registration
    auto calc = std::make_shared<CalculatorTool>();
    assert(reg.register_tool(calc));
    assert(reg.has_instance("calculator"));
    
    // Lookup returns pointer to registered instance
    assert(reg.lookup("calculator") != nullptr);
    assert(reg.lookup("calculator")->get_name() == "calculator");
    
    // Duplicate instance registration returns false
    auto calc2 = std::make_shared<CalculatorTool>();
    assert(!reg.register_tool(calc2));
    
    std::cout << "  -> PASSED\n";
}

void test_factory_creation() {
    std::cout << "[TEST] Running test_factory_creation...\n";
    ToolRegistry reg;
    
    assert(reg.register_creator("calculator", []() { return std::make_unique<CalculatorTool>(); }));
    assert(reg.has_creator("calculator"));
    
    // Create produces fresh distinct instances
    auto t1 = reg.create("calculator");
    auto t2 = reg.create("calculator");
    assert(t1 != nullptr);
    assert(t2 != nullptr);
    assert(t1.get() != t2.get());
    
    // Unknown creator returns nullptr
    auto unknown = reg.create("unknown_tool");
    assert(unknown == nullptr);
    
    std::cout << "  -> PASSED\n";
}

void test_aliases_and_normalization() {
    std::cout << "[TEST] Running test_aliases_and_normalization...\n";
    ToolRegistry reg;
    
    reg.register_creator("execute_shell", []() { return std::make_unique<ExecTool>(); });
    reg.register_alias("exec", "execute_shell");
    
    assert(reg.normalize("exec") == "execute_shell");
    assert(reg.normalize("unknown") == "unknown");
    assert(reg.has_creator("exec"));
    
    auto t = reg.create("exec");
    assert(t != nullptr);
    assert(t->get_name() == "execute_shell");
    
    std::cout << "  -> PASSED\n";
}

void test_allow_deny_policies() {
    std::cout << "[TEST] Running test_allow_deny_policies...\n";
    ToolRegistry reg;
    reg.register_all_tools();
    
    // Deny policy
    reg.deny("execute_shell");
    assert(!reg.is_allowed("execute_shell"));
    assert(reg.lookup("exec") == nullptr);
    assert(reg.create("exec") == nullptr);
    
    // Allow policy (whitelist)
    ToolRegistry reg_allow;
    reg_allow.register_all_tools();
    reg_allow.allow("calculator");
    assert(reg_allow.is_allowed("calculator"));
    assert(!reg_allow.is_allowed("web_search"));
    assert(reg_allow.lookup("calculator") != nullptr);
    assert(reg_allow.lookup("web_search") == nullptr);
    
    std::cout << "  -> PASSED\n";
}

/// B-9.5-02: Focused test — duplicate creator overwrite semantics.
///
/// Theo contract trong ToolRegistry.h:
///   register_creator() ALWAYS overwrites the existing creator for the same name.
/// Điều này khác với register_tool() (instance registry) vốn returns false khi trùng.
///
/// Test matrix:
///   1. Register creator A → create() trả instance của A.
///   2. Register creator B (cùng tên) → creator B overwrite creator A.
///   3. create() sau overwrite trả instance của B, không phải A.
///   4. Hai lần create() liên tiếp trả hai object khác nhau (fresh instance).
///   5. register_creator() returns true cả hai lần (không reject duplicate).
void test_duplicate_creator_overwrite() {
    std::cout << "[TEST] Running test_duplicate_creator_overwrite...\n";
    ToolRegistry reg;

    // --- Round 1: register CalculatorTool creator for "dual_test" ---
    assert(reg.register_creator("dual_test",
        []() { return std::make_unique<CalculatorTool>(); }) == true);
    assert(reg.has_creator("dual_test"));

    auto first = reg.create("dual_test");
    assert(first != nullptr);
    // CalculatorTool reports name "calculator"
    assert(first->get_name() == "calculator");

    // --- Round 2: overwrite with TimeTool creator (same canonical name) ---
    assert(reg.register_creator("dual_test",
        []() { return std::make_unique<TimeTool>(); }) == true);   // overwrite always returns true
    assert(reg.has_creator("dual_test"));

    auto after_overwrite = reg.create("dual_test");
    assert(after_overwrite != nullptr);
    // After overwrite, creator produces TimeTool, not CalculatorTool
    assert(after_overwrite->get_name() == "time");

    // --- Round 3: two consecutive creates → distinct fresh objects ---
    auto obj_a = reg.create("dual_test");
    auto obj_b = reg.create("dual_test");
    assert(obj_a != nullptr);
    assert(obj_b != nullptr);
    assert(obj_a.get() != obj_b.get());  // must be different heap objects

    std::cout << "  -> PASSED\n";
}

void test_register_all_tools() {
    std::cout << "[TEST] Running test_register_all_tools...\n";
    ToolRegistry reg;
    reg.register_all_tools();
    
    // Verify instances
    assert(reg.has_instance("calculator"));
    assert(reg.has_instance("file"));
    assert(reg.has_instance("read_file"));
    assert(reg.has_instance("write_file"));
    assert(reg.has_instance("append_file"));
    assert(reg.has_instance("execute_shell"));
    assert(reg.has_instance("web_search"));
    assert(reg.has_instance("memory"));
    assert(reg.has_instance("time"));
    assert(reg.has_instance("json"));
    assert(reg.has_instance("git"));
    
    // Verify aliases
    assert(reg.lookup("calculate") != nullptr);
    assert(reg.lookup("exec") != nullptr);
    assert(reg.lookup("google_search") != nullptr);
    assert(reg.lookup("create_file") != nullptr);
    
    // Verify tool execution returns valid result
    Tool* calc = reg.lookup("calculator");
    auto res = calc->execute("2 + 3");
    assert(res.has_value());
    assert(res.value().find("5") != std::string::npos);
    
    std::cout << "  -> PASSED\n";
}

int main() {
    std::cout << "=== RUNNING ROLE B TOOL REGISTRY & FACTORY FOCUSED TESTS ===\n";
    test_registry_instance_registration();
    test_factory_creation();
    test_aliases_and_normalization();
    test_allow_deny_policies();
    test_duplicate_creator_overwrite();
    test_register_all_tools();
    std::cout << "=== ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY ===\n";
    return 0;
}
