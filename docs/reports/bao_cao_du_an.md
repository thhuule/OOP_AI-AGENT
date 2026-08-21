# Technical Report - AI-AGENT Project

## 1. User's Requirement

The project requires an object-oriented C++ AI Agent framework with clear layers, not a simple API caller. The system must include an LLM client, Tool registry, Skill loader, Agent loop, and Harness evaluation. Each layer must have a separate responsibility and verifiable source/test evidence.

Mandatory requirements:

| Requirement group | Main requirement | Status |
|---|---|---|
| LLM and Agent | Gemini/Ollama client, text/image message, ReAct loop, history, parser, max-step stop | DONE |
| Tools | Shell, file, web search, SQLite memory, calculator, and at least three additional tools | DONE |
| Skill and loop safety | Markdown skill loading, keyword selection, generic and ping-pong loop detection | DONE |
| OOP design | Four design patterns, layer boundaries, UML/Mermaid diagrams, Modern C++ feature matrix | DONE |
| Harness and benchmark | 10 tasks with 4 simple / 4 medium / 2 hard split, evaluator, trajectory, success rate | DONE |
| Bonus | Vector memory and multi-agent demo | PARTIALLY DONE - implemented, pending independent review |
| Git contribution gate | Commit count passes, but line balance and commit spacing fail the current audit | BLOCKED / requires instructor disposition |

Main sources of truth: [project specification](../../planning/reference/OOP%20Project%202026%20AI%20Agent.docx%20%281%29.md) and [final requirement traceability](../evidence/requirement_traceability_final_2026-08-20.md).

## 2. System Features

| Feature | Observable behavior | Status | Evidence |
|---|---|---|---|
| LLM client | Calls Gemini or Ollama through the shared `LLMClient` interface and returns a response or `LLMError` | DONE | [README](../../README.md), [OOP report](report_oop_design.md) |
| ReAct AgentLoop | Accepts an instruction, calls the LLM, parses tool/final actions, executes tools, and records observations | DONE | `test_role_a`, `test_template_method` |
| Tool registry | Runtime registration, aliases, allow/deny policy, and factory creation by name | DONE | [Tools report](report_tools.md), `test_tools` |
| Required tools | `calculator`, `execute_shell`, `read_file`, `write_file`, `append_file`, `web_search`, `memory` | DONE | [Tools report](report_tools.md) |
| Extra tools | `time`, `json`, `git` | DONE | [README](../../README.md) |
| Skill loading | Injects `task_planner`, `step_verifier`, and `error_recovery` before each run | DONE | [OOP report](report_oop_design.md) |
| Loop detection | Detects repeated actions and ping-pong loops, then stops safely after threshold | DONE | [Evaluation report](report_evaluation.md) |
| Harness/Evaluator | Loads tasks, cleans artifacts, runs the agent, scores results, exports summary and trajectory | DONE | [Evaluation report](report_evaluation.md) |
| Environment | Uses `NativeEnvironment` in production and `SandboxEnvironment` in focused tests | DONE | `test_harness` |
| Vector memory | SQLite + Ollama embeddings + cosine similarity; `HashEmbedder` is only used in offline tests | PARTIALLY DONE | [Tools report](report_tools.md) |
| Multi-agent | Two worker threads communicate through queues, with timeout and explicit failure reporting | PARTIALLY DONE | `test_multi_agent`, `demo_multi_agent` |
| GUI/VLM | Screenshot/action contracts and `VLMEvaluator` skeleton exist; no end-to-end desktop agent is claimed | PARTIALLY DONE | [Tools report](report_tools.md), [Evaluation report](report_evaluation.md) |

## 3. Tech Solutions

| Need | Solution | File/Module | Verification |
|---|---|---|---|
| Call model APIs | `LLMClient` abstract interface, `GeminiClient`, `OllamaClient`, timeout, usage metadata | `src/client/` | `test_role_a` |
| Parse and exchange structured data | `nlohmann::json`, typed action/final-answer path, JSON trajectory export | `src/agent/`, `src/harness/` | `test_role_a`, `test_harness` |
| HTTP/web search | `libcurl` with request/connect timeout and an offline transport seam for tests | `src/tools/WebSearchTool.cpp` | `test_websearch_offline_fixture` |
| Persistent memory | Local SQLite DB, embedding BLOB, C++ cosine similarity | `src/tools/MemoryTool.cpp`, `src/tools/Embedding.cpp` | `test_memory_vector_search_ranking` |
| File and artifact safety | `std::filesystem`, rejection of `..` and absolute artifact paths in harness cleanup | `src/environment/`, `src/harness/` | `test_harness` |
| Ownership and cleanup | RAII, `unique_ptr`, `shared_ptr`, virtual destructors on interfaces | `src/agent/`, `src/tools/`, `src/harness/` | CTest |
| Multi-threading | Worker threads, message queues, `stopAndJoinAll()` | `src/multiagent/` | `test_multi_agent` |
| Reproducible build | CMake C++26 targets, vendored `nlohmann/json` | `CMakeLists.txt` | `cmake --build build` |

`config.json` is an ignored local file and may contain an API key. It must not be committed or copied into reports.

## 4. Architecture & OOP Design

The main layers are:

```text
EntryPoints
    -> AgentCore
        -> LLMClient
        -> SkillLoader
        -> ToolRegistry -> Tool implementations
    -> HarnessLayer -> EnvironmentLayer
    -> MultiAgentLayer
```

Important dependency rules:

- `AgentLoop` does not depend on `HarnessRunner`.
- Tools do not depend on Agent or Harness classes.
- Harness observes the Agent through `StepHook` and does not read Agent internals.
- Production uses `NativeEnvironment`; tests may inject `SandboxEnvironment`.

Required design patterns:

| Pattern | Location | Purpose | Evidence |
|---|---|---|---|
| Strategy | `Tool`, `Evaluator`, `Embedder` | Swap algorithms/implementations through shared interfaces | `test_tools`, `test_harness` |
| Template Method | `AgentLoop::run()` | Keep the ReAct skeleton fixed while allowing primitive operations to be overridden in tests | `test_template_method` |
| Registry/Factory | `ToolRegistry` | Register and create tools at runtime through canonical names and aliases | `test_tools` |
| Observer/Hook | `StepHook` | Let Harness record trajectories without reverse dependency | `test_harness` |

Class details, ownership, and the Modern C++ matrix are documented in the [OOP design report](report_oop_design.md) and [class diagram](../diagrams/class_diagram.md).

## 5. Detailed Logic & AI Integration

Agent execution flow:

```text
User instruction
  -> AgentLoop builds system prompt + injects selected skills
  -> LLMClient generates response
  -> parser returns final answer or JSON tool call
  -> ToolRegistry normalizes, looks up, or creates the tool
  -> Tool executes args
  -> observation is appended back to history
  -> StepHook records trajectory
  -> stop on final answer, max steps, fatal tool path, or loop detection
```

Benchmark harness flow:

```text
run_eval
  -> read config and create provider client
  -> register tools and load skills
  -> HarnessRunner loads benchmark/tasks.json
  -> clean artifacts through Environment
  -> run 10 tasks sequentially
  -> score through KeywordEvaluator/FunctionalEvaluator
  -> export eval_results.json, benchmark_summary.txt, trajectory_task_XXX.json
```

The report keeps these distinctions explicit:

- Real LLM runs versus fake/mock LLMs in focused tests.
- Historical deterministic fallback versus real model reasoning; current `run_eval` does not enable fallback to improve scores.
- Tool success versus final task success; action-level score is not final success.
- `tokens_used = 0` means the provider did not return metadata, not that the model used zero tokens.

## 6. Implementation & Code Structure

| Module | Main paths | Owner | Notes |
|---|---|---|---|
| Systems/Core | `src/client/`, `src/agent/`, `src/skills/` | Role A | LLM, AgentLoop, parser, skills, loop detector |
| Tools/Data | `src/tools/` | Role B | Tool interface, Registry/Factory, concrete tools, vector memory |
| Evaluation/Infra | `src/harness/`, `benchmark/` | Role C | Task loader, evaluators, scoring, trajectory, `run_eval` |
| Environment | `src/environment/` | A/C | Artifact-operation interface plus sandbox/native implementations |
| Multi-agent | `src/multiagent/` | Role C | Worker threads, queue, dispatcher, demo report |
| Docs/Evidence | `docs/`, `planning/` | A/B/C | Reports, diagrams, guide, traceability, weekly evidence |

Related documents:

- [OOP design report](report_oop_design.md)
- [Tools report](report_tools.md)
- [Evaluation report](report_evaluation.md)
- [Documentation guide](../guides/DOCUMENTATION_GUIDE.md)
- [Final requirement traceability](../evidence/requirement_traceability_final_2026-08-20.md)

## 7. Verification & Testing

Offline gate from repository root:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Focused test executables:

```bash
./build/test_harness
./build/test_multi_agent
./build/test_tools
./build/test_template_method
./build/test_role_a
```

Current evidence status:

| Verification | Coverage | Result |
|---|---|---|
| Build | CMake configure/build with C++26 targets | PASS according to README/evidence |
| `test_role_a` | LLM config, parser, skills, loop detector, C++ feature matrix | PASS |
| `test_tools` | Registry, tool args/error paths, vector memory, screenshot/action contracts | PASS |
| `test_harness` | Task loading, evaluator strategy, cleanup, scoring, trajectory export | PASS |
| `test_multi_agent` | Queue/thread lifecycle and injected failure | PASS |
| `test_template_method` | AgentLoop skeleton and primitive-operation override | PASS |
| CTest | Five registered tests | `5/5` PASS |

Build success is not treated as runtime success. A real-provider benchmark should only be run when key, quota, model access, and network are stable.

## 8. Evaluation & Benchmark

`benchmark/tasks.json` contains 10 tasks:

| Category | Count | Coverage |
|---|---:|---|
| Simple | 4 | Calculator, file write, file read, local time |
| Medium | 4 | Multi-step tool/file/memory workflows |
| Hard | 2 | Longer workflows or recovery behavior |

The harness records three score layers:

| Metric | Meaning |
|---|---|
| Evaluator score | Whether the keyword/functional evaluator passes |
| Action-level score | Whether a required tool step exists and executes successfully |
| Final success | Evaluator pass and action-level pass |

Current real-provider benchmark:

| Field | Value |
|---|---|
| Run ID | `run_20260820_002933_100` |
| Provider/model | Gemini `gemma-4-31b-it` |
| Final success | `7/10` (`0.7`) |
| Evaluator score | `0.7` |
| Action-level score | `0.9` |
| Action provenance | `source: llm`, no fixture action |
| Failed tasks | `task_004`, `task_005`, `task_009` |

This result satisfies the requirement to report the selected model's measured success rate, but it is not a `10/10` model-quality claim. Historical fallback-assisted runs are only pipeline evidence and do not replace the current real benchmark. Detailed analysis is in the [Evaluation report](report_evaluation.md).

## 9. Limitations & Future Work

Blockers or submission-sensitive items:

- The Git contribution gate currently fails line balance and commit spacing according to the [requirement traceability audit](../evidence/requirement_traceability_final_2026-08-20.md). This is an evidence/process issue and cannot be honestly fixed with runtime code.
- Vector memory and multi-agent are implemented and tested, but traceability still marks them as technically passed with independent review pending.

Honest limitations:

- `VLMEvaluator` and GUI agent execution are not complete end-to-end; only screenshot/action contracts and a skeleton evaluator exist.
- Token usage depends on provider metadata; `0` means not measured.
- Benchmark cleanup is batch-level and does not create a separate workspace per task.
- Web search, Gemini, Ollama, and live vector acceptance depend on network, quota, and model availability.
- Action-level score is heuristic and must not be used alone as final task success.

Reasonable future work:

- Add isolated workspaces per benchmark task if task-order dependencies become a problem.
- Extend focused tests for failure taxonomy cases such as rate limits, timeouts, and loop detection.
- Complete GUI/VLM end-to-end only when a stable desktop environment and vision model are available.
- Re-run real-provider benchmark only when the team needs a newer model-quality baseline.
