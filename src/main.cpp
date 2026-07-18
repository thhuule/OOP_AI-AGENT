#include "agent/SkillLoader.h"
#include "agent/agent_loop.h"
#include "client/ollama_client.h"
#include "tools/CalculatorTool.h"
#include "tools/ToolRegistry.h"
#include "agent/SkillLoader.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "=== BẮT ĐẦU KIỂM THỬ VỚI OLLAMA TỪ XA (PINGGY) ===" << std::endl;

    // 1. Khởi tạo Client Ollama
    auto ollama_client = std::make_shared<oop_agent::OllamaClient>();
    
    // 2. Khởi tạo Agent
    oop_agent::AgentLoop agent(ollama_client);
    
    // 3. Nạp SkillLoader (đảm bảo thư mục "skills" có tồn tại)
    agent.set_skill_loader(std::make_shared<SkillLoader>("skills"));
    
    // 4. Đăng ký các công cụ
    agent.register_tool(std::make_shared<oop_agent::CalculatorTool>());

    // 5. Chạy tác vụ đánh giá
    // Đây là nơi thực thi logic mà bạn đã kiểm thử thành công ở tuần 5
    std::string final_res = agent.run("Tính tích của 15 và 17 bằng công cụ calculator", 5);
    
    std::cout << "=> KẾT QUẢ ĐÁNH GIÁ: " << final_res << std::endl;
    
    return 0;
}
