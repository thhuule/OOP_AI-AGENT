#include "tools/WebSearchTool.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string query = "thủ đô của pháp";
    if (argc > 1) {
        query = "";
        for (int i = 1; i < argc; ++i) {
            if (i > 1) query += " ";
            query += argv[i];
        }
    }

    std::cout << "=====================================================\n";
    std::cout << "[OOP AI AGENT] Running WebSearchTool\n";
    std::cout << "Query: " << query << "\n";
    std::cout << "=====================================================\n\n";

    oop_agent::WebSearchTool tool;
    auto result = tool.execute(query);

    if (result.has_value()) {
        std::cout << ">>> KẾT QUẢ TÌM KIẾM (DuckDuckGo API):\n" << result.value() << "\n";
    } else {
        std::cerr << "[ERROR] WebSearchTool thất bại (mã lỗi: " 
                  << static_cast<int>(result.error()) << ")\n";
        return 1;
    }

    return 0;
}
