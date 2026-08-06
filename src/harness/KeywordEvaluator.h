#pragma once
#include "evaluator.h"

namespace oop_agent {

// Evaluator đơn giản: check agent_output có chứa đủ keyword
// được liệt kê trong expected_output (phân cách bởi dấu phẩy)
class KeywordEvaluator : public Evaluator {
public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "keyword_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& expected_output
    ) override;
};

}