#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../client/llm_client.h"
#include "../tools/Tool.h"

namespace oop_agent {

class AgentLoop {
public:
    AgentLoop(std::shared_ptr<LLMClient> client) : client_(std::move(client)) {}

    // Đăng ký các công cụ hữu dụng nhận từ Role B
    void register_tool(std::shared_ptr<Tool> tool);

    // Chạy vòng lặp giải quyết yêu cầu của bài test
    std::string run(const std::string& instruction, int max_steps = 10);

private:
    std::shared_ptr<LLMClient> client_;
    std::vector<std::shared_ptr<Tool>> tools_;
    std::vector<Message> memory_; // Lưu trữ ngữ cảnh hội thoại (System prompt + History)
};

} // namespace oop_agent
