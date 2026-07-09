#include "WebSearchTool.h"
#include <iostream>

using namespace oop_agent;

int main()
{
    WebSearchTool web;

    auto result = web.execute("capital of Japan");

    if (result.has_value())
    {
        std::cout << "Result:\n";
        std::cout << result.value() << '\n';
    }
    else
    {
        std::cout << "Error: " << static_cast<int>(result.error()) << '\n';
    }

    return 0;
}
