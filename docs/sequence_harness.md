# Sequence Diagram — Batch Evaluation

Diagram dưới đây mô tả code hiện tại của `benchmark/run_eval.cpp` và `HarnessRunner`. Cleanup diễn ra một lần trước batch; các task sau đó chạy tuần tự trong cùng working directory.

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

## Bản render đã kiểm chứng

![Sequence diagram batch evaluation](sequence_harness.png)

Nguồn Mermaid độc lập để render lại: [`sequence_harness.mmd`](sequence_harness.mmd).

## Ghi chú đối chiếu

- Strategy: `HarnessRunner` chọn `KeywordEvaluator` hoặc `FunctionalEvaluator` bằng `eval_type`.
- Observer/Hook: `AgentLoop` chỉ gọi callback; nó không include hay phụ thuộc `HarnessRunner`.
- Tool args được đóng gói trong JSON action trước khi gửi qua hook.
- Hook hiện chỉ ghi tool step. Lần LLM tạo final answer chưa có trajectory step riêng.
- `tokens_used` đang được gán `0`; đây là trạng thái chưa đo.
- Nếu một tool không tồn tại, AgentLoop đưa lỗi vào history để model thử lại; code hiện không gọi hook ở nhánh tool-not-found.
- `runAll()` chờ ba giây giữa hai task để giảm áp lực rate limit.

## Điều cần kiểm thử

`benchmark/test_harness.cpp` hiện đã kiểm tra task spec/ID/path, Strategy selection, cleanup artifact cũ, bảo toàn tool args, artifact missing/content mismatch, invalid/tool/evaluator error, bắt buộc tool step và tổng hợp điểm.

Các kiểm tra còn lại sau khi interface liên quan ổn định:

1. Harness dùng `Environment` abstraction thay vì filesystem trực tiếp.
2. Cleanup failure được tạo bằng fake environment và phải dừng batch.
3. Rate limit, timeout và loop detection được phân loại bằng fixture độc lập.
