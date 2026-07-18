#include "agent/SkillLoader.h"
#include "agent/agent_loop.h"
#include "client/llm_client.h"
#include "tools/Tool.h"
#include <iostream>
#include <memory>
#include <optional>


// 1. Giả lập một công cụ (Tool) của bạn B để kiểm tra luồng Execute
class MockCalculatorTool : public oop_agent::Tool {
public:
  std::string_view get_name() const noexcept override { return "calculator"; }
  std::string_view get_description() const noexcept override {
    return "Tính toán số học.";
  }
  std::expected<std::string, oop_agent::ToolError>
  execute(const std::string &args) override {
    if (args == "15*17")
      return "255";
    return "0";
  }
};

// 2. Giả lập LLMClient để test thuật toán ReAct mà không cần gọi API thật
class MockLLMClient : public oop_agent::LLMClient {
private:
  int call_count = 0;

public:
  std::expected<std::string, oop_agent::LLMError>
  generate_chat(const std::vector<oop_agent::Message> &messages,
                const oop_agent::LLMConfig &config) override {
    call_count++;
    if (call_count == 1) {
      // Lần lặp 1: Trả về chuỗi JSON ReAct yêu cầu gọi Tool
      return R"(Tôi cần thực hiện phép tính này trước. ```json {"tool": "calculator", "args": "15*17"} ```)";
    } else if (call_count == 2) {
      // Lần lặp 2: Đưa ra câu trả lời cuối cùng sau khi có Observation
      return R"({"tool": "final_answer", "args": "Kết quả của phép tính là 255."})";
    }
    return R"({"tool": "final_answer", "args": "Hết kịch bản test."})";
  }
};

int main() {
  std::cout << "=== BẮT ĐẦU KIỂM THỬ LOCAL PIPELINE (CORE A) ===" << std::endl;

  // Khởi tạo các thành phần
  auto mock_client = std::make_shared<MockLLMClient>();
  auto actual_loader = std::make_shared<oop_agent::SkillLoader>("../src/skills");
  actual_loader->loadAll(); // Load tất cả skill .md từ thư mục

  // Khởi tạo AgentLoop của bạn A
  oop_agent::AgentLoop agent(mock_client);

  // Cài đặt các thành phần kết nối liên tầng
  agent.set_skill_loader(actual_loader);

  // Giả lập một StepHook của bạn C để in Trajectory ra màn hình
  agent.set_step_hook([](const std::string &llm_text,
                         const std::string &tool_name,
                         const std::string &tool_args) {
    std::cout << "[StepHook Trình Báo] -> Tool phát hiện: " << tool_name
              << " | Tham số: " << tool_args << std::endl;
  });

  // Đăng ký công cụ giả lập của bạn B
  agent.register_tool(std::make_shared<MockCalculatorTool>());

  // Tiến hành kích hoạt luồng chạy thử
  std::string final_res = agent.run("Tính giúp tôi phép nhân 15 với 17", 5);

  std::cout << "\n==========================================" << std::endl;
  std::cout << "=> KẾT QUẢ CUỐI CÙNG AGENT TRẢ VỀ: " << final_res << std::endl;
  std::cout << "==========================================" << std::endl;

  return 0;
}
