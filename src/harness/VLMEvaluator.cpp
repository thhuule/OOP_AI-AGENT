#include "VLMEvaluator.h"

namespace oop_agent {

std::expected<EvalResult, EvalError> VLMEvaluator::evaluate(
    const std::string& agent_output,
    const std::string& expected_output
) {
    (void)agent_output;
    (void)expected_output;

    // TODO: Sử dụng Vision Language Model để đánh giá output có ảnh.

    EvalResult result;
    result.is_passed = false;
    result.score = 0.0f;
    result.feedback = "VLM evaluator is not implemented yet.";

    return result;
}

} // namespace oop_agent