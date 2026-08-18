# Project Report: Building an AI Agent System in C++

## 1. Introduction

This project implements an object-oriented C++ AI agent system. It connects to large language models, coordinates reasoning and tool-use steps, records execution trajectories, and evaluates results through a benchmark harness.

The agent can accept an instruction, select an action, invoke tools such as calculation, file operations, command execution, and web search, then evaluate the observable result.

## 2. Objectives

1. Build a clear layered agent architecture.
2. Apply abstraction, encapsulation, polymorphism, and separation of responsibility.
3. Implement a multi-step agent loop with explicit stopping conditions.
4. Provide reproducible focused tests and benchmark evaluation.
5. Demonstrate optional Vector Search and Multi-Agent extensions without weakening mandatory behaviour.

## 3. Architecture

The system is divided into five layers:

- **LLM client:** Gemini and Ollama request/response handling.
- **Agent core:** ReAct-style loop, parser, history, skills, and loop detection.
- **Tools:** calculator, file, command execution, web search, memory, screenshot, and guarded action contracts.
- **Harness:** task loading, evaluation, trajectory recording, artifact cleanup, and export.
- **Multi-agent extension:** worker threads, dispatcher, message queues, and coordinated shutdown.

Primary implementation sources are [AgentLoop](../../src/agent/agent_loop.cpp), [ToolRegistry](../../src/tools/ToolRegistry.cpp), [HarnessRunner](../../src/harness/HarnessRunner.cpp), and [run_eval](../../benchmark/run_eval.cpp).

## 4. Core Components

### 4.1 LLM Client

The client layer accepts conversation history and returns a typed response or `LLMError`. Provider configuration is supplied through `config.json`; the API key is never reported in documentation or benchmark evidence.

### 4.2 Agent Loop

`AgentLoop` builds a system prompt, receives a model response, parses either a tool call or final answer, executes allowed tools through the registry, records observations, and stops on final answer, max steps, or loop detection.

### 4.3 Tool Registry

`ToolRegistry` provides runtime registration, canonical-name lookup, aliases, and allow/deny policies. The agent depends on the `Tool` abstraction rather than concrete tool classes.

### 4.4 Harness and Evaluators

`HarnessRunner` loads tasks, clears approved artifacts before a batch, invokes the agent, evaluates results, and exports a summary plus one trajectory per task. Keyword and functional evaluators are selected through the Evaluator strategy interface.

### 4.5 Extensions

The Vector extension persists embeddings and ranks search results with cosine similarity. The Multi-Agent demo delegates calculator and research subtasks to separate workers and reports failure explicitly rather than fabricating a successful result.

## 5. Design Techniques

- **Strategy:** interchangeable evaluators and tool implementations.
- **Registry/Factory:** runtime tool creation and lookup.
- **Observer/Hook:** `StepHook` records agent steps without coupling `AgentLoop` to the harness.
- **Template Method:** the AgentLoop execution skeleton is fixed while primitive operations remain overridable for tests.

See [the OOP design report](report_oop_design.md) and [the class diagram](../diagrams/class_diagram.md) for source-level detail.

## 6. Verification

The benchmark suite defines ten tasks: four simple, four medium, and two hard. Evaluation separates evaluator success, action-level success, and final success; an action-level score alone is not treated as proof of the final task result.

Mandatory regression commands are:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

The current CTest suite contains five registered tests. A production `run_eval` run disables deterministic fallback; trajectories identify whether actions originate from the LLM or an explicitly opted-in test fixture.

## 7. Limitations

- Token usage is not yet collected from provider response metadata.
- Benchmark tasks share one working directory within a batch; cleanup is batch-level rather than per-task isolation.
- VLM/GUI execution remains a guarded contract and is not claimed as an end-to-end desktop automation feature.

## 8. Conclusion

The project demonstrates a maintainable C++ AI-agent architecture with clear module boundaries, typed error paths, tool-based execution, and reproducible focused testing. The optional extensions remain separately evidenced so they do not inflate the claims made for the mandatory agent and benchmark requirements.

## Related Documents

- [Evaluation and Benchmark Report](report_evaluation.md)
- [Tooling and Vector Report](report_tools.md)
- [OOP Design Report](report_oop_design.md)
- [Submission Checklist](../guides/submission_checklist.md)
