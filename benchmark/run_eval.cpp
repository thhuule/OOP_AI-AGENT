#include "../src/client/ollama_client.h"
#include "../src/agent/agent_loop.h"
#include <iostream>

int main() {
    std::cout << "=== THỬ NGHIỆM AGENT CORE LOOP (ROLE A) ===" << std::endl;

    // Khởi tạo Client và Agent Engine
    auto client = std::make_shared<oop_agent::OllamaClient>();
    oop_agent::AgentLoop agent(client);

    // Chạy thử một câu lệnh đơn giản gửi tới Ollama
    std::string response = agent.run("Hãy tự giới thiệu ngắn gọn bạn là ai bằng 1 câu văn.");
    
    std::cout << "\n[Kết quả cuối cùng của Agent]: " << response << std::endl;
    return 0;
}