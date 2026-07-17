#include "agent/agent_loop.h"

#include "client/ollama_client.h"

#include "tools/CalculatorTool.h"
#include "tools/FileTool.h"
#include "tools/ExecTool.h"
#include "tools/WebSearchTool.h"
#include "tools/MemoryTool.h"
#include "tools/TimeTool.h"
#include "tools/JsonTool.h"
#include "tools/GitTool.h"

#include <iostream>
#include <memory>
#include <string>

using namespace oop_agent;

int main()
{
    auto client = std::make_shared<OllamaClient>();

    AgentLoop agent(client);

    // Register tools
    agent.register_tool(std::make_shared<CalculatorTool>());
    agent.register_tool(std::make_shared<FileTool>());
    agent.register_tool(std::make_shared<ExecTool>());
    agent.register_tool(std::make_shared<WebSearchTool>());
    agent.register_tool(std::make_shared<MemoryTool>());
    agent.register_tool(std::make_shared<TimeTool>());
    agent.register_tool(std::make_shared<JsonTool>());
    agent.register_tool(std::make_shared<GitTool>());

    std::cout << "========================================\n";
    std::cout << "        OOP AI Agent - Week 5\n";
    std::cout << "========================================\n";
    std::cout << "Type 'exit' to quit.\n";

    while (true)
    {
        std::string instruction;

        std::cout << "\nUser> ";
        std::getline(std::cin, instruction);

        if (instruction == "exit")
        {
            break;
        }

        std::string answer = agent.run(instruction);

        std::cout << "\nAgent> "
                  << answer
                  << "\n";
    }

    return 0;
}