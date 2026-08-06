# Sequence Diagram — Agent Run

> Phản ánh luồng `AgentLoop::run()` theo ReAct pattern.  
> Bao gồm các nhánh: tool success, tool error, LLM error, parser fail, policy deny, max_steps, loop detected, final answer.

```mermaid
sequenceDiagram
    autonumber
    participant Caller
    participant AgentLoop
    participant SkillLoader
    participant LLMClient
    participant ToolRegistry
    participant Tool
    participant LoopDetector
    participant StepHook

    Caller->>AgentLoop: run(instruction, max_steps)

    %% ── OBSERVE: build system prompt ──────────────────
    AgentLoop->>SkillLoader: load_skills_for(instruction)
    SkillLoader-->>AgentLoop: system_prompt (injected skill content)
    AgentLoop->>AgentLoop: init history [system, user(instruction)]

    loop Each step 1..max_steps

        %% ── THINK: call LLM ───────────────────────────
        AgentLoop->>LLMClient: generate_chat(history, config)

        alt LLM network / timeout error
            LLMClient-->>AgentLoop: throws / error string
            AgentLoop->>StepHook: notify(step, thought="", tool="", error="LLM_ERROR", latency)
            AgentLoop-->>Caller: return "LLM error — aborting"
        else LLM returns text
            LLMClient-->>AgentLoop: raw_text
        end

        %% ── ACT: parse response ───────────────────────
        AgentLoop->>AgentLoop: parse_llm_response(raw_text)

        alt Parse fail (no valid action found)
            AgentLoop->>StepHook: notify(step, thought=raw_text, tool="", error="PARSER_FAIL", latency)
            AgentLoop->>AgentLoop: append observation("PARSER_FAIL: …") to history
            Note over AgentLoop: model gets chance to retry next step
        else FinalAnswerAction
            AgentLoop->>StepHook: notify(step, thought, tool="final_answer", result=answer, latency)
            AgentLoop-->>Caller: return final_answer
        else ToolCallAction
            AgentLoop->>ToolRegistry: normalize(tool_name)  [alias resolution]
            ToolRegistry-->>AgentLoop: canonical_name

            AgentLoop->>ToolRegistry: is_allowed(canonical_name)

            alt Tool in deny-list or not in allow-list
                ToolRegistry-->>AgentLoop: false
                AgentLoop->>StepHook: notify(step, thought, tool=canonical_name, error="POLICY_DENIED", latency)
                AgentLoop->>AgentLoop: append observation("POLICY_DENIED: …") to history
            else Tool not found in registry
                ToolRegistry-->>AgentLoop: nullptr
                AgentLoop->>StepHook: notify(step, thought, tool=canonical_name, error="TOOL_NOT_FOUND", latency)
                AgentLoop->>AgentLoop: append observation("TOOL_NOT_FOUND: …") to history
            else Tool found
                ToolRegistry-->>AgentLoop: Tool*
                AgentLoop->>Tool: execute(args)

                alt Tool returns ToolError
                    Tool-->>AgentLoop: unexpected(ToolError)
                    AgentLoop->>StepHook: notify(step, thought, tool, error=ToolError, latency)
                    AgentLoop->>AgentLoop: append observation("TOOL_ERROR: …") to history
                    Note over AgentLoop: error fed back so model can recover
                else Tool succeeds
                    Tool-->>AgentLoop: expected(result_string)
                    AgentLoop->>StepHook: notify(step, thought, tool, result, latency, tokens)
                    AgentLoop->>AgentLoop: append observation(result) to history
                end

                %% ── Loop detection ────────────────────
                AgentLoop->>LoopDetector: add_action(canonical_name, normalize(args))
                AgentLoop->>LoopDetector: is_loop_detected()

                alt Loop detected (repeat or ping-pong)
                    LoopDetector-->>AgentLoop: true
                    AgentLoop->>AgentLoop: on_loop_detected()
                    AgentLoop->>StepHook: notify(step, thought, tool, error="LOOP_DETECTED", latency)
                    AgentLoop-->>Caller: return "Loop detected — aborting"
                else No loop
                    LoopDetector-->>AgentLoop: false
                end
            end
        end

        %% ── max_steps guard ───────────────────────────
        alt step == max_steps AND no final answer yet
            AgentLoop->>AgentLoop: on_max_steps_reached()
            AgentLoop->>StepHook: notify(step, thought="", tool="", error="MAX_STEPS_REACHED", latency=0)
            AgentLoop-->>Caller: return "Max steps reached"
        end

    end
```

---

## Bảng nhánh và kết quả

| Nhánh | Điều kiện | Hành động | Kết quả trả về |
|---|---|---|---|
| LLM error | Network timeout / HTTP error | Notify hook, abort | `"LLM error — aborting"` |
| Parser fail | Không parse được action | Notify hook, append observation, retry | Tiếp tục step tiếp |
| Final answer | `FinalAnswerAction` | Notify hook | Answer string |
| Policy denied | Tool trong deny-list | Notify hook, append observation | Tiếp tục step tiếp |
| Tool not found | Tên không có trong registry | Notify hook, append observation | Tiếp tục step tiếp |
| Tool error | `execute()` trả `unexpected` | Notify hook, append error observation | Tiếp tục, model có thể phục hồi |
| Tool success | `execute()` trả `expected` | Notify hook, append result | Tiếp tục step tiếp |
| Loop detected | `LoopDetector::is_loop_detected()` | `on_loop_detected()`, abort | `"Loop detected — aborting"` |
| Max steps | `step == max_steps` | `on_max_steps_reached()`, abort | `"Max steps reached"` |

## Template Method — primitive operations

`AgentLoop::run()` là skeleton cố định. Các bước override được (protected virtual):

| Primitive operation | Hành vi mặc định | Subclass có thể override |
|---|---|---|
| `observe(result)` | Append observation vào history | Có thể ghi log hoặc filter |
| `think_and_act(step)` | Gọi LLM và parse | Có thể mock LLM trong test |
| `execute_tool(action)` | Lookup registry và gọi `Tool::execute` | Có thể inject test double |
| `on_loop_detected()` | Log và set abort flag | Có thể raise exception |
| `on_max_steps_reached()` | Log và set abort flag | Có thể ghi metrics |

Observer/Hook: `StepHook` là `std::function<void(TrajectoryStep)>` được truyền từ `HarnessRunner`. `AgentLoop` **không** include bất kỳ header nào của Harness.