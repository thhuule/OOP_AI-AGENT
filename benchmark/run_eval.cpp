#include "agent/agent_loop.h"
#include "client/ollama_client.h"
#include "tools/CalculatorTool.h"
#include "agent/SkillLoader.h"
#include <iostream>
#include <memory>

int main() {
    // 1. Khởi tạo Client Ollama thật với model cụ thể
    // 1. Khởi tạo Client Ollama thật với địa chỉ URL và model
auto ollama_client = std::make_shared<oop_agent::OllamaClient>(
    "http://oihnt-35-233-204-204.free.pinggy.net",
    "gemma4:e4b"
);

    // 2. Khởi tạo Agent
    oop_agent::AgentLoop agent(ollama_client);

    // 3. Nạp SkillLoader thật từ bạn C
    agent.set_skill_loader(std::make_shared<SkillLoader>("src/skills"));

    // 4. Đăng ký các công cụ thật
    agent.register_tool(std::make_shared<oop_agent::CalculatorTool>());

    // 5. Chạy eval với câu lệnh thật
    std::string final_res = agent.run("Tính tích của 15 và 17 bằng công cụ calculator", 5);

    std::cout << "=> KẾT QUẢ EVALUATE: " << final_res << std::endl;
    return 0;
}