#pragma once

#include "evaluator.h"

namespace oop_agent {

class VLMEvaluator : public Evaluator {
public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "vlm_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& expected_output
    ) override;
};

} // namespace oop_agent