#include "tools/MemoryTool.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== BẮT ĐẦU KIỂM THỬ VỚI OLLAMA TỪ XA (PINGGY) ===" << std::endl;

int main()
{
    MemoryTool memory;

    auto result = memory.execute("save I love AI");

    if (result)
        std::cout << *result << '\n';
    else
        std::cout << "Error\n";

    auto search = memory.execute("search AI");

    if (search)
        std::cout << *search << '\n';
}
