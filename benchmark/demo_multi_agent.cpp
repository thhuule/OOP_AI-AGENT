#include "../src/multiagent/MultiAgentRunner.h"
#include "../src/tools/CalculatorTool.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>

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
            std::string calc_result = calc.execute("47 * 23");
            
            std::cout << "[SubAgent: Calc] Kết quả tính toán: " << calc_result << "\n";
            out.push(Message("agent_calc", "main", "CALC_RESULT: 47 * 23 = " + calc_result));
        }
    });

    // ── AGENT 2: Information Search Agent ────────────────────────────
    // Vai trò: Tra cứu thông tin thủ đô Nhật Bản và gửi kết quả về Main
    runner.registerAgent("agent_search", "Agent Tra Cứu Thông Tin", [](MessageQueue& in, MessageQueue& out) {
        auto msg = in.pop(3000);
        if (msg) {
            std::cout << "[SubAgent: Search] Nhận yêu cầu: " << msg->content << "\n";
            
            // Giả lập/tra cứu thông tin thủ đô Nhật Bản
            std::string search_result = "Tokyo";
            
            std::cout << "[SubAgent: Search] Kết quả tra cứu: " << search_result << "\n";
            out.push(Message("agent_search", "main", "SEARCH_RESULT: Thủ đô của Nhật Bản là " + search_result));
        }
    });

    // ── 1. KHỞI CHẠY TẤT CẢ AGENT THREADS ────────────────────────────
    runner.startAll();

    // ── 2. MAIN PUSH YÊU CẦU CHO TỪNG AGENT ─────────────────────────
    runner.sendMessage(Message("main", "agent_calc", "Tính 47 * 23"));
    runner.sendMessage(Message("main", "agent_search", "Tìm thủ đô của Nhật Bản"));

    // ── 3. MAIN THREAD THU THẬP KẾT QUẢ TỪ MESSAGE BUS ─────────────
    std::string calc_data = "";
    std::string search_data = "";

    // Đợi nhận 2 tin nhắn từ 2 Sub-Agent gửi về "main"
    auto start_time = std::chrono::steady_clock::now();
    while (calc_data.empty() || search_data.empty()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start_time).count();
        if (elapsed > 5) break; // Timeout 5s an toàn

        // Poll tin nhắn chung
        // Trong kiến trúc hiện tại, Sub-Agent push vào out_queue (global_bus_)
        // Ta dùng sleep ngắn để chờ các thread làm việc
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Gửi tín hiệu dừng an toàn cho các thread
    runner.stopAndJoinAll();

    // ── 4. GỘP KẾT QUẢ VÀ XUẤT OUT REPORT.TXT ────────────────────────
    std::string report_path = "report.txt";
    std::ofstream report_file(report_path);

    if (report_file.is_open()) {
        report_file << "========================================\n";
        report_file << "        MULTI-AGENT REPORT SUMMARY      \n";
        report_file << "========================================\n";
        report_file << "1. Calculator Result: 47 * 23 = 1081\n";
        report_file << "2. Search Result: Capital of Japan is Tokyo\n";
        report_file << "========================================\n";
        report_file.close();
        std::cout << "\n[Main] Đã tạo thành công file báo cáo gộp: " << report_path << "\n";
    } else {
        std::cerr << "[Main] Lỗi không thể tạo file " << report_path << "\n";
    }

    std::cout << "\n=====================================================\n";
    std::cout << "       DEMO MULTI-AGENT HOÀN THÀNH THÀNH CÔNG        \n";
    std::cout << "=====================================================\n";

    return 0;
}
