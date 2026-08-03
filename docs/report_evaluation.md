# Evaluation and Benchmark Report

## 1. Scope

This document describes the current Evaluation/Infrastructure implementation: task sources, the benchmark lifecycle, evaluators, trajectories, scoring, historical run evidence, multi-agent support, and unresolved limitations.

Sources of truth:

- Tasks: [`../benchmark/tasks.json`](../benchmark/tasks.json)
- Harness: [`../src/harness/HarnessRunner.h`](../src/harness/HarnessRunner.h), [`../src/harness/HarnessRunner.cpp`](../src/harness/HarnessRunner.cpp)
- Evaluator interface: [`../src/harness/evaluator.h`](../src/harness/evaluator.h)
- Entrypoint: [`../benchmark/run_eval.cpp`](../benchmark/run_eval.cpp)
- Historical results: `../benchmark/results/run_20260801_212302_253/` and `../benchmark/results/run_20260801_220549_361/`

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
5. `runAll()` removes known benchmark artifacts once before the batch.
6. Tasks run sequentially through `AgentLoop::run()` with a three-second delay between tasks.
7. The harness selects an evaluator using `eval_type`, then calculates evaluator, action-level, and final results.
8. `exportResults()` creates a timestamped run directory containing summary JSON, summary text, and one trajectory per task.

See [`sequence_harness.md`](sequence_harness.md) for the detailed sequence.

### 3.1 Artifact Cleanup and Isolation

The harness currently removes `notes.txt`, `result.txt`, `capital.txt`, `output.txt`, `calc.txt`, and `data.txt`, together with artifacts declared by tasks. Absolute paths and paths containing `..` are rejected. If cleanup fails, the batch stops so an old file cannot create a false positive.

An important limitation is that cleanup runs **once before the entire batch**. The implementation does not create an independent workspace for each task. Tasks share a working directory; `task_003` and `task_007` also use the `notes.txt` produced by `task_002` in the same batch. This is batch-level cleanup, not complete per-task isolation.

## 4. Scoring

The harness stores three result layers for every task:

| Metric | Current condition |
|---|---|
| Evaluator score | `KeywordEvaluator` or `FunctionalEvaluator` returns pass |
| Action-level score | The task does not require a tool, or a recorded step uses one of `required_tools` and its result does not contain a known error marker |
| Final success | Evaluator passes **and** action-level evaluation passes |

Each batch score is the number of tasks that satisfy the corresponding condition divided by the total number of tasks.

Action-level evaluation is still a heuristic. It checks the tool name and result text but does not independently verify every artifact post-condition. File post-conditions are protected by the evaluator and affect `final_success`. Therefore, action-level score alone must not be used to claim that a task completed correctly.

## 5. Trajectory and Export

Each recorded tool step currently contains:

- `thought`: the LLM response that produced the tool call;
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

`HarnessRunner::classifyFailure()` can currently produce:

| Code | Meaning |
|---|---|
| `NONE` | The task reached final success |
| `RATE_LIMIT` | Evidence contains 429 or resource-exhausted markers |
| `TIMEOUT` | The LLM, tool, or evaluator timed out |
| `TOOL_NOT_FOUND` | The agent requested a tool that does not exist |
| `INVALID_ARGS` | Tool arguments were invalid |
| `TOOL_EXECUTION_FAILED` | A tool returned ExecutionFailed, AccessDenied, or UnknownError |
| `LOOP_DETECTED` | The loop detector stopped the agent |
| `EVALUATOR_ERROR` | The evaluator could not produce a valid evaluation result |
| `NO_TOOL_EXECUTION` | A tool-required task had no relevant successful tool step |
| `ARTIFACT_MISSING` | A required artifact does not exist |
| `ARTIFACT_CONTENT_MISMATCH` | The artifact exists, but its content fails evaluation |
| `INCOMPLETE_TASK` | The agent stopped without completing the request |
| `POST_CONDITION_FAIL` | Evaluation failed without a more specific classification |
| `PARSER_FAIL` | Fallback when evaluator success and final result state conflict |

The classifier normalizes evidence and recognizes compact enum forms such as `InvalidArgument` and `ExecutionFailed`. Classification still relies on text evidence; a future implementation should preserve typed errors across layers instead of relying only on strings.

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
3. Every tool-required task has a real relevant tool step with a non-error result.
4. File tasks create the exact required filename and content during the current run.
5. Task 005 and 010 trajectories preserve real arguments.
6. The report identifies the correct provider and model without exposing an API key.
7. Token value `0` is never described as actual usage.

## 10. Current Focused Tests

`benchmark/test_harness.cpp` is an offline executable that uses a fake LLM, fake evaluators, and temporary directories. It currently verifies:

- valid task loading;
- rejection of artifact paths containing `..` and duplicate task IDs;
- evaluator Strategy selection with preserved scores and feedback;
- StepHook preservation of action type, tool name, arguments, result, and latency;
- removal of stale batch artifacts in an isolated test directory;
- distinction between `ARTIFACT_MISSING` and `ARTIFACT_CONTENT_MISMATCH`;
- distinction between `INVALID_ARGS`, `TOOL_EXECUTION_FAILED`, and `EVALUATOR_ERROR`;
- rejection of a final answer that skips a required tool;
- evaluator, action-level, and final score aggregation.

Run it with:

```bash
./build/test_harness
```

A passing run ends with `ALL HARNESS TESTS PASSED`. The suite does not yet prove that the harness uses an `Environment` abstraction because that hierarchy does not exist. Add those tests after Role A supplies the interface.

CMake also registers the `harness` and `multi_agent` tests with CTest:

```bash
ctest --test-dir build --output-on-failure
```

## 11. Backlog After Week 9

- Collect real token metadata from Gemini and Ollama.
- Test the harness through the `Environment` abstraction and inject intentional cleanup failures.
- Extend failure-taxonomy tests for rate limits, timeouts, and loop detection.
- Separate `tool_steps_count` from total LLM steps when trajectories begin recording final answers.
- Consider isolated workspaces or explicit fixtures to reduce task-order dependencies.
- Implement `HarnessRunner → MultiAgentRunner → MessageQueue` integration only if the team commits to the bonus objective.
