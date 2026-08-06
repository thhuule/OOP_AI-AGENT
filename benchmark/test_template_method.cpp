#include "agent/agent_loop.h"
#include "client/llm_client.h"
#include "tools/Tool.h"
#include "tools/ToolRegistry.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

using namespace oop_agent;

// 1. Scripted LLM Client for testing think_and_act override or default behavior
class MockLLMClient final : public LLMClient {
public:
    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>&,
        const LLMConfig&) override {
        return "Mock LLM Answer";
    }
};

// 2. Custom Subclass overriding primitive operations of AgentLoop
class CustomAgentLoop : public AgentLoop {
public:
    CustomAgentLoop(std::shared_ptr<LLMClient> llm)
        : AgentLoop(std::move(llm)) {}

    bool build_prompt_called = false;
    bool think_act_called = false;
    int primitive_execution_order = 0;

protected:
    std::string build_system_prompt(const std::string& instruction) override {
        build_prompt_called = true;
        primitive_execution_order++;
        return "Custom System Prompt: " + instruction;
    }

    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int /*step*/) override {
        think_act_called = true;
        primitive_execution_order++;
        return FinalAnswerAction{"Custom Final Answer"};
    }
};

void test_template_method_primitive_overrides() {
    auto client = std::make_shared<MockLLMClient>();
    CustomAgentLoop custom_agent(client);

    // Run skeleton method run() with unique instruction so fallback plan is empty and think_and_act primitive is called
    std::string result = custom_agent.run("XYZ", 3);

    // Verify primitive overrides were called by run() skeleton
    assert(custom_agent.build_prompt_called);
    assert(custom_agent.think_act_called);
    assert(custom_agent.primitive_execution_order == 2);
    assert(result == "Custom Final Answer");
    std::cout << "[PASS] Template Method primitives overridden and executed correctly." << std::endl;
}

int main() {
    std::cout << "=== TEST TEMPLATE METHOD (ROLE A) ===" << std::endl;
    test_template_method_primitive_overrides();
    std::cout << "ALL TEMPLATE METHOD TESTS PASSED" << std::endl;
    return 0;
}
