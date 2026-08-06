#pragma once
#include "evaluator.h"
#include "../tools/ExecTool.h"
#include <memory>

namespace oop_agent {

class FunctionalEvaluator : public Evaluator {
public:
    /// Constructor: inject ExecTool từ bên ngoài (Dependency Injection)
    explicit FunctionalEvaluator(std::shared_ptr<ExecTool> exec_tool)
        : exec_tool_(std::move(exec_tool)) {}

    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "functional_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& expected_output
    ) override;

private:
    std::shared_ptr<ExecTool> exec_tool_;
};

}
