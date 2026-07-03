#include "KeywordEvaluator.h"
#include <sstream>

namespace oop_agent {

// Tách chuỗi expected_output theo dấu phẩy thành danh sách keyword
static std::vector<std::string> splitKeywords(const std::string& expected_output) {
    std::vector<std::string> keywords;
    std::stringstream ss(expected_output);
    std::string keyword;

    while (std::getline(ss, keyword, ',')) {
        // Bỏ khoảng trắng đầu/cuối
        size_t start = keyword.find_first_not_of(" \t");
        size_t end = keyword.find_last_not_of(" \t");
        if (start != std::string::npos) {
            keywords.push_back(keyword.substr(start, end - start + 1));
        }
    }
    return keywords;
}

std::expected<EvalResult, EvalError> KeywordEvaluator::evaluate(
    const std::string& agent_output,
    const std::string& expected_output
) {
    if (agent_output.empty()) {
        return std::unexpected(EvalError::MissingTrajectory);
    }

    auto keywords = splitKeywords(expected_output);
    if (keywords.empty()) {
        return std::unexpected(EvalError::InvalidTaskSpec);
    }

    int matched = 0;
    std::string missing_list;

    for (const auto& keyword : keywords) {
        if (agent_output.find(keyword) != std::string::npos) {
            matched++;
        } else {
            if (!missing_list.empty()) missing_list += ", ";
            missing_list += keyword;
        }
    }

    EvalResult result;
    result.score = static_cast<float>(matched) / static_cast<float>(keywords.size());
    result.is_passed = (matched == static_cast<int>(keywords.size()));
    result.feedback = result.is_passed
        ? "Tất cả keyword khớp"
        : "Thiếu keyword: " + missing_list;

    return result;
}

}