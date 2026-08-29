#pragma once

#include "../client/llm_client.h"
#include "evaluator.h"
#include <memory>

namespace oop_agent {

class LLMEvaluator : public Evaluator {
public:
    LLMEvaluator(std::shared_ptr<LLMClient> client, LLMConfig config)
        : client_(std::move(client)), config_(std::move(config)) {}

    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "llm_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& expected_output) override;

private:
    std::shared_ptr<LLMClient> client_;
    LLMConfig config_;
};

} // namespace oop_agent
