#include "agent_loop.h"
#include <iostream>

namespace oop_agent {

void AgentLoop::register_tool(std::shared_ptr<Tool> tool) {
    if (tool) {
        tools_.push_back(tool);
        std::cout << "[AgentLoop] Đã nạp công cụ thành công: " << tool->get_name() << std::endl;
    }
}

std::string AgentLoop::run(const std::string& instruction, int max_steps) {
    std::cout << "[AgentLoop] Bắt đầu xử lý task: \"" << instruction << "\"" << std::endl;
    
    // Khởi tạo bộ nhớ cho ca làm việc mới
    memory_.clear();
    memory_.push_back(Message{"system", "Bạn là một AI Agent thông minh có khả năng sử dụng công cụ qua tư duy ReAct.", {}});
    memory_.push_back(Message{"user", instruction, {}});
    int current_step = 0;
    bool task_completed = false;
    std::string final_answer = "Chưa tìm thấy câu trả lời.";

    while (current_step < max_steps && !task_completed) {
        current_step++;
        std::cout << "\n--- [Vòng lặp ReAct] Bước " << current_step << " / " << max_steps << " ---" << std::endl;

        // Gọi LLM Client để lấy "Tư duy (Thought) / Hành động (Action)"
        LLMConfig config;
        config.model_name = "gemma4:e4b"; // Đặt đúng tag model trong colab oop.py của bạn
        
        // Tạm thời cấu hình URL cục bộ hoặc URL từ Pinggy của nhóm để test
        config.api_url = "http://vcvou-34-26-174-246.run.pinggy-free.link/api/chat"; 

        auto response = client_->generate_chat(memory_, config);

        if (!response.has_value()) {
            std::cerr << "[AgentLoop] Lỗi kết nối LLM trong quá trình suy luận." << std::endl;
            break;
        }

        std::string llm_text = response.value();
        std::cout << "[LLM Response]:\n" << llm_text << std::endl;

        // Lưu phản hồi của LLM vào bộ nhớ lịch sử hội thoại
        memory_.push_back(Message{"assistant", llm_text, {}});
        // KỊCH BẢN TUẦN 4: Mockup dừng lại sau bước đầu tiên hoặc kết thúc khi LLM trả chuỗi text thông thường.
        // Sang tuần 5 bạn sẽ làm bộ Parser lọc chuỗi JSON để tự động kích hoạt `tool->execute()`.
        
        task_completed = true; // Chốt chặn thử nghiệm tuần 4 để không bị lặp vô hạn
        final_answer = llm_text;
    }

    std::cout << "[AgentLoop] Hoàn thành tác vụ." << std::endl;
    return final_answer;
}

} // namespace oop_agent