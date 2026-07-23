#include <iostream>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include "client/gemini_client.h"

using json = nlohmann::json;

int main() {
    // 1. Đọc file config.json ở thư mục gốc
    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        std::cerr << "[ERROR] Khong tim thay file config.json!\n";
        return 1;
    }

    json config;
    config_file >> config;

    std::string api_key = config.value("api_key", "");
    std::string model = config.value("model", "gemini-2.5-flash");

    std::cout << "[INFO] Dang test GeminiClient voi model: " << model << "...\n";

    // 2. Khởi tạo GeminiClient
    auto client = std::make_unique<oop_agent::GeminiClient>(api_key, model);

    // 3. Tạo hội thoại test
    std::vector<oop_agent::Message> history = {
        {"system", "Ban la mot AI tro ly."},
        {"user", "Xin chao Gemini! 1 + 1 bang bao nhieu?"}
    };

    // 4. Gọi API
    auto result = client->generate_chat(history);

    if (result.has_value()) {
        std::cout << "\n>>> KET QUA GEMINI TRA VE:\n" << result.value() << "\n\n";
    } else {
        std::cout << "\n[ERROR] Loi khi goi Gemini API!\n";
    }

    return 0;
}