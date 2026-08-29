#include "LLMEvaluator.h"

#include <algorithm>
#include <cctype>

namespace oop_agent {
namespace {

std::string trimLeft(std::string value) {
    value.erase(value.begin(), std::ranges::find_if(value, [](unsigned char c) {
                    return !std::isspace(c);
                }));
    return value;
}

bool startsWithPass(std::string value) {
    value = trimLeft(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return value.starts_with("PASS");
}

} // namespace

std::expected<EvalResult, EvalError> LLMEvaluator::evaluate(
    const std::string& agent_output,
    const std::string& expected_output) {
    if (!client_)
        return std::unexpected(EvalError::UnknownError);
    if (agent_output.empty() || expected_output.empty())
        return std::unexpected(EvalError::InvalidTaskSpec);

    const std::vector<Message> prompt = {{
        "user",
        "You are grading one benchmark task. Decide whether the agent output satisfies the expected result.\n"
        "Reply with PASS or FAIL first, then one short reason.\n\n"
        "Expected result:\n" + expected_output + "\n\n"
        "Agent output:\n" + agent_output
    }};

    const auto judged = client_->generate_chat(prompt, config_);
    if (!judged)
        return std::unexpected(EvalError::UnknownError);

    const bool passed = startsWithPass(*judged);
    return EvalResult{passed, passed ? 1.0f : 0.0f, *judged};
}

} // namespace oop_agent
