#include "agent/agent_loop.h"
#include "client/llm_client.h"
#include "tools/ToolRegistry.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include <variant>

using namespace oop_agent;

// Mock LLM Client that does nothing but satisfy AgentLoop dependency
class DummyLLMClient : public LLMClient {
public:
    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>&, const LLMConfig&) override {
        return "Final Answer: Done";
    }
};

// Subclass to override AgentLoop's primitive operations
class CustomTestAgentLoop : public AgentLoop {
public:
    explicit CustomTestAgentLoop(std::shared_ptr<LLMClient> llm)
        : AgentLoop(llm) {}

    std::vector<std::string> called_sequence;

protected:
    std::string build_system_prompt(const std::string& instruction) override {
        called_sequence.push_back("build_system_prompt");
        return AgentLoop::build_system_prompt(instruction);
    }

    std::variant<ToolCallAction, FinalAnswerAction> think_and_act(int step) override {
        called_sequence.push_back("think_and_act_" + std::to_string(step));
        if (step == 1) {
            return ToolCallAction{"calculator", "10+20"};
        }
        return FinalAnswerAction{"completed custom sequence"};
    }

    std::expected<std::string, std::string> execute_tool(const ToolCallAction& action) override {
        called_sequence.push_back("execute_tool_" + action.tool_name + "_" + action.args);
        return "30";
    }

    void observe(const std::string& text) override {
        called_sequence.push_back("observe_" + text);
        AgentLoop::observe(text);
    }

    void on_loop_detected() override {
        called_sequence.push_back("on_loop_detected");
        AgentLoop::on_loop_detected();
    }

    void on_max_steps_reached() override {
        called_sequence.push_back("on_max_steps_reached");
        AgentLoop::on_max_steps_reached();
    }
};

void test_template_method_execution_flow() {
    std::cout << "[TEST] Running test_template_method_execution_flow...\n";
    auto client = std::make_shared<DummyLLMClient>();
    CustomTestAgentLoop agent(client);

    // Disable fallback mechanism for this task instruction to ensure think_and_act is called
    std::string instruction = "Execute custom logic without matching fallbacks";
    std::string result = agent.run(instruction, 5);

    std::cout << "Result: " << result << "\n";
    assert(result == "completed custom sequence");

    // Print and verify execution sequence
    std::cout << "Sequence of calls:\n";
    for (const auto& step : agent.called_sequence) {
        std::cout << "  - " << step << "\n";
    }

    // Verify template method calling sequence:
    // 1. build_system_prompt
    // 2. think_and_act_1
    // 3. execute_tool_calculator_10+20
    // 4. observe_30
    // 5. think_and_act_2
    assert(!agent.called_sequence.empty());
    assert(agent.called_sequence[0] == "build_system_prompt");
    assert(agent.called_sequence[1] == "think_and_act_1");
    assert(agent.called_sequence[2] == "execute_tool_calculator_10+20");
    assert(agent.called_sequence[3] == "observe_30");
    assert(agent.called_sequence[4] == "think_and_act_2");

    std::cout << "  -> PASSED\n";
}

int main() {
    std::cout << "=== RUNNING ROLE A TEMPLATE METHOD FOCUSED TESTS ===\n";
    test_template_method_execution_flow();
    std::cout << "=== ALL ROLE A TESTS PASSED SUCCESSFULLY ===\n";
    return 0;
}
