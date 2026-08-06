# Sequence Diagram — Batch Evaluation

The diagram below represents the current implementation of `benchmark/run_eval.cpp` and `HarnessRunner`. Cleanup runs once before the batch; the tasks then run sequentially in the same working directory.

```mermaid
sequenceDiagram
    autonumber
    actor User
    participant Eval as run_eval
    participant Config as config.json
    participant Harness as HarnessRunner
    participant Tasks as benchmark/tasks.json
    participant Agent as AgentLoop
    participant LLM as LLMClient
    participant Tool as ToolRegistry/Tool
    participant Hook as StepHook
    participant Evaluator as Evaluator Strategy
    participant FS as Filesystem

    User->>Eval: ./build/run_eval
    Eval->>Config: Read provider/model/credentials
    Config-->>Eval: Configuration
    Eval->>Eval: Create GeminiClient or OllamaClient
    Eval->>Agent: Register tools and skills
    Eval->>Harness: Construct(tasks path, output dir)
    Harness->>Tasks: loadTasks()
    Tasks-->>Harness: 10 task specifications
    Harness->>Harness: Validate fields, evaluator, tools, paths, IDs
    Eval->>Agent: set_step_hook(Harness.createStepHook())
    Eval->>Harness: set_agent(AgentLoop)
    Eval->>Harness: runAll()

    Harness->>FS: Remove known benchmark artifacts
    alt Cleanup failed
        FS-->>Harness: Filesystem error
        Harness-->>Eval: Failed results, batch does not run
    else Cleanup succeeded
        loop Each task, sequentially
            Harness->>Agent: run(instruction, max_steps)
            loop Each AgentLoop step
                Agent->>LLM: generate_chat(history)
                alt LLM error or malformed response
                    LLM-->>Agent: LLMError
                    Agent->>Agent: Add retry instruction or stop at limit
                else Tool call
                    LLM-->>Agent: tool name + args
                    Agent->>Tool: find and execute(args)
                    alt Tool success
                        Tool-->>Agent: Result
                    else Tool rejected/failed
                        Tool-->>Agent: ToolError
                    end
                    Agent->>Hook: thought, action with args, result
                    Hook->>Harness: Append trajectory step + latency
                else Final answer
                    LLM-->>Agent: Final text
                    Agent->>Agent: Check completion guards
                end
            end
            Agent-->>Harness: Final output
            Harness->>Harness: findEvaluator(eval_type)
            Harness->>Evaluator: evaluate(output, expected)
            alt Keyword evaluator
                Evaluator-->>Harness: Keyword match score/feedback
            else Functional evaluator
                Evaluator->>Tool: ExecTool(eval_script)
                Tool->>FS: Check current artifact/post-condition
                FS-->>Tool: Script output
                Tool-->>Evaluator: PASS or failure
                Evaluator-->>Harness: Functional score/feedback
            end
            Harness->>Harness: Check relevant successful tool step
            Harness->>Harness: final = evaluator AND action-level
            Harness->>Harness: classifyFailure()
        end
        Harness-->>Eval: TaskRunResult list
        Eval->>Harness: exportResults(results)
        Harness->>FS: Create timestamped run directory
        Harness->>FS: Write eval_results.json
        Harness->>FS: Write trajectory_task_XXX.json
        Harness->>FS: Write benchmark_summary.txt
        Harness-->>Eval: Export status
    end
```

## Verified Render

![Sequence diagram batch evaluation](sequence_harness.png)

Standalone Mermaid source for reproducible rendering: [`sequence_harness.mmd`](sequence_harness.mmd).

## Verification Notes

- Strategy: `HarnessRunner` selects `KeywordEvaluator` or `FunctionalEvaluator` according to `eval_type`.
- Observer/Hook: `AgentLoop` only invokes a callback; it neither includes nor depends on `HarnessRunner`.
- Tool arguments are packaged in the JSON action before being sent through the hook.
- The hook currently records tool steps only. The LLM's final answer does not have a separate trajectory step.
- `tokens_used` is currently set to `0`, which means it has not been measured.
- If a tool does not exist, `AgentLoop` adds the error to the history so the model can retry; the current code does not invoke the hook on the tool-not-found path.
- `runAll()` waits three seconds between tasks to reduce rate-limit pressure.

## Remaining Tests

`benchmark/test_harness.cpp` currently covers task specifications, IDs and paths, Strategy selection, stale-artifact cleanup, tool-argument preservation, missing artifacts, content mismatches, invalid/tool/evaluator errors, required tool steps, and score aggregation.

Add the following checks after the related interfaces stabilize:

1. The harness uses an `Environment` abstraction instead of accessing the filesystem directly.
2. A cleanup failure produced by a fake environment stops the batch.
3. Rate limits, timeouts, and loop detection are classified with independent fixtures.
