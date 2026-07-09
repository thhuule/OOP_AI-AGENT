#include "tools/WebSearchTool.h"
#include <iostream>

using namespace oop_agent;

int main()
{
    WebSearchTool tool;

    auto result = tool.execute("capital of Japan");

    if (result.has_value())
    {
        std::cout << "Success\n";
        std::cout << result.value() << '\n';
    }
    else
    {
        std::cout << "Error: " << static_cast<int>(result.error()) << '\n';
    }

    return 0;
}