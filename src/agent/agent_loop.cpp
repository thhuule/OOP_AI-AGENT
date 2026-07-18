#include "agent_loop.h"
#include <iostream>
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
std::shared_ptr<Tool>
AgentLoop::find_tool(std::string_view name)
{
    for (auto& tool : tools_)
    {
        if (tool->get_name() == name)
        {
            return tool;
        }
    }

    return nullptr;
}

std::string AgentLoop::run(const std::string& instruction, int max_steps)
{
    std::cout << "[AgentLoop] Bắt đầu xử lý task: \""
              << instruction << "\"\n";

    memory_.clear();

    memory_.push_back(
        Message{
            "system",
    R"(Bạn là AI Agent.

    Available tools:
    - calculator
    - memory
    - time
    - json
    - git
    - exec
    - file
    - websearch

    Nếu cần sử dụng Tool hãy CHỈ trả về JSON:

    {
        "tool":"<tool_name>",
        "arguments":"<arguments>"
    }

    Sau khi nhận được Observation từ Tool,
    hãy trả lời người dùng bằng ngôn ngữ tự nhiên.

    Không gọi Tool lần thứ hai nếu đã có Observation.)",
            {}
        });

    memory_.push_back(
        Message{
            "user",
            instruction,
            {}
        });

    int current_step = 0;
    std::string final_answer = "Không có phản hồi.";

    while (current_step < max_steps)
    {
        current_step++;

        std::cout
            << "\n===== Step "
            << current_step
            << " =====\n";

        LLMConfig config;
        config.model_name = "gemma4:e4b";
        config.api_url =
            "http://vcvou-34-26-174-246.run.pinggy-free.link/api/chat";

        auto response =
            client_->generate_chat(memory_, config);

        if (!response)
        {
            std::cerr
                << "[AgentLoop] LLM Error\n";

            final_answer =
                "LLM connection failed.";

            break;
        }

        std::string llm_text =
            response.value();

        std::cout
            << "[LLM]\n"
            << llm_text
            << '\n';

        //----------------------------------------------------
        // Thử parse JSON xem có phải Tool Call không
        //----------------------------------------------------

        try
        {
            json j =
                json::parse(llm_text);

            if (!j.contains("tool") ||
                !j.contains("arguments"))
            {
                final_answer =
                    "Invalid Tool JSON.";

                break;
            }

            if (!j["tool"].is_string() ||
                !j["arguments"].is_string())
            {
                final_answer =
                    "Invalid Tool JSON.";

                break;
            }

            std::string tool_name =
                j["tool"].get<std::string>();

            std::string arguments =
                j["arguments"].get<std::string>();

            auto tool =
                find_tool(tool_name);

            if (!tool)
            {
                final_answer =
                    "Tool not found: " +
                    tool_name;

                break;
            }

            std::cout
                << "[Agent] Execute Tool: "
                << tool_name
                << '\n';

            auto result =
                tool->execute(arguments);

            if (!result)
            {
                final_answer =
                    "Tool '" +
                    tool_name +
                    "' execution failed.";

                break;
            }

            std::cout
                << "[Observation]\n"
                << result.value()
                << '\n';

            memory_.push_back(
            Message{
                "tool",
                "Observation: " + result.value(),
                {}
            });

            // Tiếp tục vòng lặp để LLM đọc Observation
            continue;
        }
        catch (const json::exception&)
        {
            //------------------------------------------------
            // Không phải JSON -> coi như câu trả lời cuối
            //------------------------------------------------

            memory_.push_back(
                Message{
                    "assistant",
                    llm_text,
                    {}
                });

            final_answer = llm_text;

            break;
        }
    }

    std::cout
        << "[AgentLoop] Hoàn thành.\n";

    return final_answer;
}
} // namespace oop_agent