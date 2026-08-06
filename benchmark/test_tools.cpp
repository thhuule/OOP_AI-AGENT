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
    Tool* t = reg.lookup("calculator");
    assert(t != nullptr);
    assert(t->get_name() == "calculator");
    
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
    test_register_all_tools();
    std::cout << "=== ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY ===\n";
    return 0;
}
