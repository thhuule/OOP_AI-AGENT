#include "../src/multiagent/MultiAgentRunner.h"
#include "../src/tools/CalculatorTool.h"
#include "../src/tools/WebSearchTool.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <string_view>

using namespace oop_agent;

int main() {
    std::cout << "=====================================================\n";
    std::cout << "   DEMO SCENARIO MULTI-AGENT RUNNER (WEEK 7 - C)     \n";
    std::cout << "=====================================================\n\n";

    MultiAgentRunner runner;

    // ── AGENT 1: Math Calculator Agent ───────────────────────────────
    // Vai trò: Thực hiện tính toán 47 * 23 và gửi kết quả về Main
    runner.registerAgent("agent_calc", "Agent Tính Toán", [](MessageQueue& in, MessageQueue& out) {
        auto msg = in.pop(3000); // Chờ tối đa 3 giây
        if (msg) {
            std::cout << "[SubAgent: Calc] Nhận yêu cầu: " << msg->content << "\n";
            
            CalculatorTool calc;
            auto calc_result = calc.execute("47 * 23");

            if (!calc_result) {
                std::cerr << "[SubAgent: Calc] CalculatorTool failed.\n";
                return;
            }

            std::cout << "[SubAgent: Calc] Kết quả tính toán: " << calc_result.value() << "\n";
            out.push(AgentMessage("agent_calc", "main", "CALC_RESULT: 47 * 23 = " + calc_result.value()));
        }
    });

    // ── AGENT 2: Information Search Agent ────────────────────────────
    // Vai trò: Tra cứu thông tin thủ đô Nhật Bản và gửi kết quả về Main
    runner.registerAgent("agent_search", "Agent Tra Cứu Thông Tin", [](MessageQueue& in, MessageQueue& out) {
        auto msg = in.pop(3000);
        if (msg) {
            std::cout << "[SubAgent: Search] Nhận yêu cầu: " << msg->content << "\n";

            // Giãn nhịp để demo vẫn an toàn khi thay tool bằng một agent gọi Gemini.
            std::this_thread::sleep_for(std::chrono::seconds(3));

            WebSearchTool search;
            auto search_res = search.execute("capital of Japan Tokyo");
            std::string search_result = search_res.value_or("Tokyo");
            
            std::cout << "[SubAgent: Search] Kết quả tra cứu: " << search_result << "\n";
            out.push(
                AgentMessage(
                    "agent_search",
                    "main",
                    "SEARCH_RESULT: Thủ đô của Nhật Bản là " + search_result
                )
            );
        }
    });

    // ── 1. KHỞI CHẠY TẤT CẢ AGENT THREADS ────────────────────────────
    runner.startAll();

    // ── 2. MAIN PUSH YÊU CẦU CHO TỪNG AGENT ─────────────────────────
    runner.sendMessage(AgentMessage("main", "agent_calc", "Tính 47 * 23"));
    runner.sendMessage(AgentMessage("main", "agent_search", "Tìm thủ đô của Nhật Bản"));

    // ── 3. MAIN THREAD THU THẬP KẾT QUẢ TỪ MESSAGE BUS ─────────────
    std::string calc_data = "";
    std::string search_data = "";

    // Đợi nhận đúng 2 kết quả do dispatcher chuyển tới endpoint "main".
    for (int received = 0; received < 2; ++received) {
        auto result = runner.receiveMessage("main", 6000);
        if (!result) {
            std::cerr << "[Main] Hết thời gian chờ kết quả từ Sub-Agent.\n";
            break;
        }

        if (result->content.starts_with("CALC_RESULT:")) {
            calc_data = result->content;
        } else if (result->content.starts_with("SEARCH_RESULT:")) {
            search_data = result->content;
        }
    }

    // Gửi tín hiệu dừng an toàn cho các thread
    runner.stopAndJoinAll();

    // ── 4. GỘP KẾT QUẢ VÀ XUẤT OUT REPORT.TXT ────────────────────────
    std::string report_path = "report.txt";
    if (calc_data.empty() || search_data.empty()) {
        std::cerr << "[Main] Không nhận đủ kết quả để tạo báo cáo.\n";
        return 1;
    }

    std::ofstream report_file(report_path);
    if (report_file.is_open()) {
        report_file << "========================================\n";
        report_file << "        MULTI-AGENT REPORT SUMMARY      \n";
        report_file << "========================================\n";
        report_file << "1. " << calc_data << "\n";
        report_file << "2. " << search_data << "\n";
        report_file << "========================================\n";
        report_file.close();
        std::cout << "\n[Main] Đã tạo thành công file báo cáo gộp: " << report_path << "\n";
    } else {
        std::cerr << "[Main] Lỗi không thể tạo file " << report_path << "\n";
        return 1;
    }

    std::cout << "\n=====================================================\n";
    std::cout << "       DEMO MULTI-AGENT HOÀN THÀNH THÀNH CÔNG        \n";
    std::cout << "=====================================================\n";

    return 0;
}
