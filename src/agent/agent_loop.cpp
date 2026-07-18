#include "agent_loop.h"
#include <iostream>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace oop_agent {

void AgentLoop::register_tool(std::shared_ptr<Tool> tool) {
    if (tool) {
        tools_.push_back(tool);
        std::cout << "[AgentLoop] Đã nạp công cụ: " << tool->get_name() << std::endl;
    }
}

bool AgentLoop::parse_tool_call(const std::string& llm_text, std::string& tool_name, std::string& tool_args) {
    try {
        // Tìm block JSON trong phản hồi của LLM[cite: 15]
        std::regex json_block_regex(R"(```(?:json)?\s*([\s\S]*?)\s*```)");
        std::smatch match;
        std::string json_str = llm_text;
        
        if (std::regex_search(llm_text, match, json_block_regex)) {
            json_str = match[1].str();
        }

        auto j = json::parse(json_str);
        if (j.contains("tool") && j.contains("args")) {
            tool_name = j["tool"].get<std::string>();
            if (j["args"].is_string()) {
                tool_args = j["args"].get<std::string>();
            } else {
                tool_args = j["args"].dump();
            }
            return true;
        }
    } catch (...) {
        // Tự động chuyển sang phương án Fallback Regex nếu chuỗi JSON lỗi định dạng[cite: 15]
    }

    // Fallback sang Regex quét mẫu {"tool": "...", "args": "..."}[cite: 15]
    std::regex tool_rgx(R"delim("tool"\s*:\s*"([^"]+)")delim");
    std::regex args_rgx(R"delim("args"\s*:\s*"([^"]+)")delim");
    std::smatch m;
    if (std::regex_search(llm_text, m, tool_rgx)) tool_name = m[1].str();
    if (std::regex_search(llm_text, m, args_rgx)) tool_args = m[1].str();

    return !tool_name.empty();
}

std::string AgentLoop::run(const std::string& instruction, int max_steps) {
    std::cout << "[AgentLoop] Chạy tác vụ: \"" << instruction << "\"" << std::endl;
    
    memory_.clear();
    
    // 1. Inject nội dung kỹ năng (Skill) từ SkillLoader vào System Prompt[cite: 15]
    std::string system_prompt = "Bạn là một AI Agent hoạt động theo chuẩn ReAct loop (Observe -> Think -> Act).\n";
    if (skill_loader_) {
        system_prompt += "Dưới đây là các kỹ năng hệ thống bạn có thể áp dụng:\n" + skill_loader_->getSystemPrompt();
    }
    system_prompt += "\nỞ mỗi bước, bạn PHẢI phản hồi bằng duy nhất một khối JSON theo định dạng sau:\n"
                     "{\"tool\": \"tên_công_cụ\", \"args\": \"tham_số_chuỗi\"}\n"
                     "Nếu đã có câu trả lời cuối cùng, hãy ghi nhận:\n"
                     "{\"tool\": \"final_answer\", \"args\": \"nội_dung_câu_trả_lời_của_bạn\"}";

    memory_.push_back(Message{"system", system_prompt, {}});
    
    // 2. Add instruction ban đầu vào bộ nhớ thoại[cite: 15]
    memory_.push_back(Message{"user", instruction, {}});

    int current_step = 0;
    std::string final_answer = "Max steps reached"; // Trạng thái mặc định nếu vượt quá số bước[cite: 15]

    // 3. Vòng lặp ReAct giới hạn bởi max_steps[cite: 15]
    while (current_step < max_steps) {
        current_step++;
        std::cout << "\n--- [ReAct Loop] Bước " << current_step << " / " << max_steps << " ---" << std::endl;

        LLMConfig config;
        config.model_name = "gemma4:e4b";
        config.api_url = "http://vcvou-34-26-174-246.run.pinggy-free.link/api/chat";

        // a. Gọi mô hình LLM[cite: 15]
        auto response = client_->generate_chat(memory_, config);
        if (!response.has_value()) {
            std::cerr << "[AgentLoop] Lỗi kết nối LLM." << std::endl;
            break;
        }

        std::string llm_text = response.value();
        std::cout << "[LLM]: " << llm_text << std::endl;
        memory_.push_back(Message{"assistant", llm_text, {}});

        // b. Tiến hành bóc tách lệnh gọi Tool[cite: 15]
        std::string tool_name, tool_args;
        bool parse_success = parse_tool_call(llm_text, tool_name, tool_args);

        // e. Gọi StepHook báo cáo tình trạng bước về cho bạn C (nếu có)[cite: 15]
        if (step_hook_) {
            step_hook_(llm_text, tool_name, tool_args);
        }

        if (parse_success) {
            // d. Nếu công cụ là câu trả lời cuối cùng -> Kết thúc vòng lặp[cite: 15]
            if (tool_name == "final_answer") {
                final_answer = tool_args;
                break;
            }

            // c. Khởi chạy thực thi tool[cite: 15]
            std::shared_ptr<Tool> target_tool = nullptr;
            for (const auto& t : tools_) {
                if (t->get_name() == tool_name) { target_tool = t; break; }
            }

            std::string observation;
            if (target_tool) {
                auto result = target_tool->execute(tool_args);
                observation = result.value_or("Lỗi thực thi công cụ");
            } else {
                observation = "Lỗi: Không tìm thấy công cụ mang tên '" + tool_name + "'";
            }

            std::cout << "[Observation]: " << observation << std::endl;
            memory_.push_back(Message{"user", "Observation: " + observation, {}});
        } else {
            // Đưa thông tin nhắc nhở định dạng vào bộ nhớ để LLM sửa sai ở bước kế tiếp
            memory_.push_back(Message{"user", "Observation: Lỗi định dạng JSON ReAct. Vui lòng gửi lại cấu trúc dạng {\"tool\": \"...\", \"args\": \"...\"}"});
        }
    }

    return final_answer;
}

} // namespace oop_agent