# Evaluation and Benchmark Report

## 1. Scope

This document describes the current Evaluation/Infrastructure implementation: task sources, the benchmark lifecycle, evaluators, trajectories, scoring, historical run evidence, multi-agent support, and unresolved limitations.

Sources of truth:

- Tasks: [`../benchmark/tasks.json`](../benchmark/tasks.json)
- Harness: [`../src/harness/HarnessRunner.h`](../src/harness/HarnessRunner.h), [`../src/harness/HarnessRunner.cpp`](../src/harness/HarnessRunner.cpp)
- Evaluator interface: [`../src/harness/evaluator.h`](../src/harness/evaluator.h)
- Entrypoint: [`../benchmark/run_eval.cpp`](../benchmark/run_eval.cpp)
- Historical failed/passing comparison: `../benchmark/results/run_20260801_212302_253/` and `../benchmark/results/run_20260801_220549_361/`
- Latest stored pipeline runs: `../benchmark/results/run_20260805_032212_365/` and `../benchmark/results/run_20260805_034207_664/`

The figures below are historical evidence stored in the repository. They do not replace a final verification run from a clean state of the current revision.

## 2. Benchmark Suite

`benchmark/tasks.json` currently contains 10 tasks:

| Category | Count | Tasks |
|---|---:|---|
| Simple | 4 | `task_001`–`task_004` |
| Medium | 4 | `task_005`–`task_008` |
| Hard | 2 | `task_009`–`task_010` |

Each task declares an `id`, instruction, evaluator type, category, `requires_tool`, accepted tools, artifacts, and `max_steps`. `HarnessRunner::loadTasks()` rejects tasks with missing required fields, invalid evaluator specifications, empty required-tool specifications, unsafe artifact paths, or duplicate IDs.

Two complete evaluators are currently used:

- `KeywordEvaluator`: splits comma-separated keywords, calculates the match ratio, and passes only when every keyword is present.
- `FunctionalEvaluator`: runs an `eval_script` through `ExecTool` and passes when the output contains `PASS`.

`VLMEvaluator` currently always fails with a not-implemented message. It must not be described as a complete visual evaluator.

## 3. Harness Lifecycle

The implemented flow is:

1. `run_eval` reads `config.json` and creates either `GeminiClient` or `OllamaClient` through `LLMClient`.
2. Tools are registered and Markdown skills are loaded.
3. `HarnessRunner` loads `benchmark/tasks.json`.
4. A `StepHook` is injected into `AgentLoop`, allowing the harness to record tool steps without creating a reverse dependency from the agent to the harness.
5. `runAll()` asks the injected `Environment` to remove known benchmark artifacts once before the batch. Normal runs use `NativeEnvironment`; focused tests may inject `SandboxEnvironment`.
6. Tasks run sequentially through `AgentLoop::run()` with a three-second delay between tasks.
7. The harness selects an evaluator using `eval_type`, then calculates evaluator, action-level, and final results.
8. `exportResults()` creates a timestamped run directory containing summary JSON, summary text, and one trajectory per task.

See [`sequence_harness.md`](sequence_harness.md) for the detailed sequence.

### 3.1 Artifact Cleanup and Isolation

The harness currently asks its `Environment` to remove `notes.txt`, `result.txt`, `capital.txt`, `output.txt`, `calc.txt`, and `data.txt`, together with artifacts declared by tasks. Absolute paths and paths containing `..` are rejected before the environment is called. If cleanup fails, the batch stops with `ARTIFACT_CLEANUP_FAILED` so an old file cannot create a false positive.

`HarnessRunner` creates a `NativeEnvironment` by default, so production behavior still uses the real filesystem. Tests can inject `SandboxEnvironment`, which keeps virtual files in memory, or a deliberately failing implementation that returns a specific `EnvError`. This dependency injection proves the harness depends on the `Environment` interface instead of a concrete cleanup implementation.

An important limitation is that cleanup runs **once before the entire batch**. The implementation does not create an independent workspace for each task. Tasks share a working directory; `task_003` and `task_007` also use the `notes.txt` produced by `task_002` in the same batch. This is batch-level cleanup, not complete per-task isolation.

## 4. Scoring

The harness stores three result layers for every task:

| Metric | Current condition |
|---|---|
| Evaluator score | `KeywordEvaluator` or `FunctionalEvaluator` returns pass |
| Action-level score | The task does not require a tool, or a recorded step uses one of `required_tools` and `TrajectoryStep::success` is true |
| Final success | Evaluator passes **and** action-level evaluation passes |

Each batch score is the number of tasks that satisfy the corresponding condition divided by the total number of tasks.

`TrajectoryStep::success` is the authoritative tool-execution signal. The harness does not infer failure from words inside a valid tool result, because a successful file listing may legitimately contain names such as `error_recovery.md`.

Action-level evaluation is still a heuristic: it verifies that a relevant tool succeeded, but it does not independently verify every artifact post-condition. File post-conditions are protected by the evaluator and affect `final_success`. Therefore, action-level score alone must not be used to claim that a task completed correctly.

## 5. Trajectory and Export

Each recorded tool step currently contains:

- `thought`: raw reasoning text when an LLM response produced the tool call; it may be empty for a deterministic fallback action;
- `action`: an object containing `type`, `tool`, and `args`;
- `tool_result`: the result or `ToolError`;
- `latency_ms`: elapsed time since the previous hook event;
- `tokens_used`: a placeholder field that is not currently measured.

Exported files:

| File | Contents |
|---|---|
| `eval_results.json` | Aggregate scores, category results, and per-task results |
| `trajectory_task_XXX.json` | Steps, action arguments, tool results, latency, and token fields |
| `benchmark_summary.txt` | A human-readable pass/fail summary |

### 5.1 Token Limitation

`HarnessRunner::createStepHook()` currently assigns `tokens_used = 0`. `LLMClient::generate_chat()` returns only response text, so Gemini `usageMetadata` and Ollama token-count fields do not reach the harness.

Therefore:

> `tokens_used = 0` means **not measured**; it does not mean that the model used zero tokens.

Real token collection is deferred to the Week 10 backlog: add response metadata to `LLMClient`, parse provider usage, pass it through `AgentLoop` and its hook, and include the final-answer LLM call in the total. Character-count estimates must not be reported as official token counts.

## 6. Failure Taxonomy

`HarnessRunner::classifyFailure()` contains branches for the following codes. “Focused test” below means the code has a direct offline fixture in `benchmark/test_harness.cpp`; the other branches are implemented but are not yet closed by a focused test.

| Code | Meaning | Current evidence |
|---|---|---|
| `NONE` | The task reached final success | Covered by successful Strategy fixture |
| `RATE_LIMIT` | Text evidence contains rate-limit, HTTP 429, or resource-exhausted markers | Implemented; no focused test |
| `TIMEOUT` | Text evidence contains timeout markers | Implemented; no focused test |
| `TOOL_NOT_FOUND` | Text evidence says the requested tool does not exist | Implemented; no focused test for the exact AgentLoop signal |
| `INVALID_ARGS` | A tool reports invalid arguments | Focused test |
| `TOOL_EXECUTION_FAILED` | A tool reports ExecutionFailed, AccessDenied, or UnknownError | Focused test for ExecutionFailed |
| `LOOP_DETECTED` | Text evidence says loop detection stopped the agent | Implemented; no focused test |
| `EVALUATOR_ERROR` | The evaluator cannot produce a valid result | Focused test |
| `NO_TOOL_EXECUTION` | A tool-required task has no relevant successful tool step | Focused test |
| `ARTIFACT_MISSING` | A required artifact does not exist | Focused test |
| `ARTIFACT_CONTENT_MISMATCH` | The artifact exists but evaluation fails | Focused test |
| `INCOMPLETE_TASK` | The agent stops without completing the request | Implemented; no focused test |
| `POST_CONDITION_FAIL` | Evaluation fails without a more specific classification | Exercised indirectly |
| `PARSER_FAIL` | Final fallback branch for an inconsistent result state | No explicit parser signal or focused test |

The classifier normalizes evidence and recognizes compact enum forms such as `InvalidArgument` and `ExecutionFailed`. Classification still relies on text evidence. `PARSER_FAIL` must not be presented as a proven end-to-end parser classification until Role A exposes a clear parser-error signal and a focused test reaches that branch. Expanding this taxonomy belongs to the Week 10 bug-fix backlog unless it becomes a submission blocker.

## 7. Historical Run Comparison

| Metric | `run_20260801_212302_253` | `run_20260801_220549_361` |
|---|---:|---:|
| Tasks passed | 2/10 | 10/10 |
| Evaluator score | 0.2 | 1.0 |
| Action-level score | 1.0 | 1.0 |
| Final success rate | 0.2 | 1.0 |
| Simple | 1/4 | 4/4 |
| Medium | 1/4 | 4/4 |
| Hard | 0/2 | 2/2 |

### 7.1 Failed Run `212302_253`

The old run had an action-level score of 1.0 but a final success rate of only 0.2. Historical analysis traced the main cause to incorrect `FileWriteTool` argument parsing: the tool reported `OK` while creating the wrong filename or content. This demonstrates that the old action-level score was too optimistic when considered by itself.

Observed symptoms:

- `task_002`, `005`, `006`, and `009`: incorrect artifact name or content;
- `task_003` and `007`: cascading failures from `notes.txt`;
- `task_009`: stopped by loop detection;
- `task_010`: append operation was incomplete;
- `task_001`: the old instruction and evaluator did not match the files at the repository root.

### 7.2 Passing Run `220549_361`

The historical artifact records 10/10. Two important trajectories are:

- `task_005`: preserves the arguments `47 * 23`, `result.txt,1081`, and `result.txt`; calculator, write, and read steps succeed.
- `task_010`: preserves `ToolError: NotFound`, then writes, appends, and reads back `initial data\nappended`.

This run provides evidence that real arguments were preserved in trajectories and that task 010 completed its recovery path. However, it remains historical evidence and must not be used to claim that the current revision passes 10/10 without a new clean run.

### 7.3 Latest Stored Runs on 2026-08-05

Both `run_20260805_032212_365` and `run_20260805_034207_664` record:

- evaluator score: `1.0`;
- action-level score: `1.0`;
- final success rate: `1.0`;
- all 10 tasks marked `PASS`.

These runs are historical evidence that the benchmark pipeline at those recorded revisions could execute, record, evaluate, and export all ten tasks. They do **not** prove that the current post-integration worktree passes: after the Role B pull, the current offline gate builds but `test_harness` crashes during tool registration. They are also **not sufficient evidence that the configured model independently planned every task**. The current `AgentLoop` checks a deterministic fallback plan before calling the LLM for known benchmark instructions, and the stored trajectories contain fallback-specific values such as `1081`, `Tokyo`, and `56088`. Token fields are also zero because usage is not measured.

The final report must therefore use the wording “pipeline run 10/10” unless a future run records whether each action came from the LLM or fallback. It must not use these artifacts alone to advertise model reasoning quality.

## 8. Multi-Agent Support

`MultiAgentRunner` provides worker registration, dedicated threads, a dispatcher, message queues, receive timeouts, and `stopAndJoinAll()`. `test_multi_agent` verifies that `ping` becomes `RESULT:ping` and that the runner stops completely. `demo_multi_agent` runs calculator and search workers, then combines their results into `report.txt`.

This is an independent extension. `HarnessRunner` does not currently call `MultiAgentRunner`, so the demo is not sufficient evidence for the sub-agent integration bonus.

## 9. Final Verification Procedure

Do not run a real-provider benchmark merely to update documentation. After Roles A and B freeze their code and the team approves quota and cost:

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_harness
./build/test_multi_agent
./build/run_eval
```

Post-run checklist:

1. The new run contains exactly 10 tasks with the 4/4/2 distribution.
2. The log confirms successful cleanup before the batch.
3. Every tool-required task has a relevant successful tool step; the report also records whether the action came from the LLM or deterministic fallback when that metadata becomes available.
4. File tasks create the exact required filename and content during the current run.
5. Task 005 and 010 trajectories preserve real arguments.
6. The report identifies the correct provider and model without exposing an API key.
7. Token value `0` is never described as actual usage.
8. A 10/10 fallback-assisted run is reported as pipeline evidence, not as proof of model planning quality.

## 10. Current Focused Tests

`benchmark/test_harness.cpp` is an offline executable that uses a fake LLM, fake evaluators, and temporary directories. Its source contains focused fixtures for:

- valid task loading;
- rejection of artifact paths containing `..` and duplicate task IDs;
- evaluator Strategy selection with preserved scores and feedback;
- StepHook preservation of action type, tool name, arguments, result, and latency;
- removal of stale batch artifacts in an isolated test directory;
- cleanup through an injected in-memory `SandboxEnvironment`;
- explicit `ARTIFACT_CLEANUP_FAILED` classification when an injected environment rejects cleanup;
- distinction between `ARTIFACT_MISSING` and `ARTIFACT_CONTENT_MISMATCH`;
- distinction between `INVALID_ARGS`, `TOOL_EXECUTION_FAILED`, and `EVALUATOR_ERROR`;
- rejection of a final answer that skips a required tool;
- evaluator, action-level, and final score aggregation.

Run it with:

```bash
./build/test_harness
```

A passing run ends with `ALL HARNESS TESTS PASSED`. Static inspection confirms that sandbox cleanup and intentional cleanup-failure fixtures exercise the `Environment` abstraction for artifact cleanup and artifact-existence checks. However, on the current post-Role-B worktree the executable crashes earlier during tool registration, so the complete fixture set is not currently proven green. Re-run it after Role B closes the Registry regression.

CMake also registers the `harness` and `multi_agent` tests with CTest:

```bash
ctest --test-dir build --output-on-failure
```

## 11. Backlog After Week 9

- Collect real token metadata from Gemini and Ollama.
- Extend failure-taxonomy tests for rate limits, timeouts, and loop detection.
- Separate `tool_steps_count` from total LLM steps when trajectories begin recording final answers.
- Consider isolated workspaces or explicit fixtures to reduce task-order dependencies.
- Record the source of each action (`llm` or `fallback`) in trajectories before comparing model planning quality.
- Implement `HarnessRunner → MultiAgentRunner → MessageQueue` integration only if the team commits to the bonus objective.
