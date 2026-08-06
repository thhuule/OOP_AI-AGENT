# Class Diagram — AI-Agent OOP 2026

> Source of truth: Mermaid diagram below. Tên class, hàm và quan hệ phản ánh đúng source code tại thời điểm Tuần 9.
> Render bằng: GitHub Markdown, VS Code Mermaid Preview, hoặc mermaid.live.

```mermaid
classDiagram
    %% ─────────────────────────────────────────
    %% PACKAGE 1 — Client / Core
    %% ─────────────────────────────────────────
    namespace ClientCore {
        class LLMConfig {
            +string provider
            +string model
            +string base_url
            +string api_key
            +bool   use_mock
            +float  temperature
            +int    max_tokens
            +int    timeout_ms
        }

        class Message {
            +string role
            +string content
            +vector~string~ images
        }

        class LLMClient {
            <<abstract>>
            +generate_chat(messages: vector~Message~, config: LLMConfig) string*
            +supports_images() bool*
        }

        class OllamaClient {
            -LLMConfig config_
            +generate_chat(messages, config) string
            +supports_images() bool
            -build_payload(messages) json
            -post_request(payload) string
        }

        class GeminiClient {
            -LLMConfig config_
            +generate_chat(messages, config) string
            +supports_images() bool
            -build_payload(messages) json
            -post_request(payload) string
        }

        class ToolCallAction {
            +string tool_name
            +string args
        }

        class FinalAnswerAction {
            +string answer
        }

        class SkillLoader {
            -string skills_dir_
            -vector~string~ loaded_skills_
            +load_skills_for(instruction: string) string
            -read_markdown(path: string) string
            -select_by_keyword(instruction, skills) vector~string~
        }

        class LoopDetector {
            -int threshold_
            -vector~pair~string,string~~ history_
            +add_action(tool_name: string, normalized_args: string) void
            +is_loop_detected() bool
            +reset() void
            -is_generic_repeat() bool
            -is_ping_pong() bool
        }

        class AgentLoop {
            -shared_ptr~LLMClient~   llm_
            -shared_ptr~SkillLoader~ skills_
            -ToolRegistry            registry_
            -LoopDetector            detector_
            -function~StepHook~      step_hook_
            -vector~Message~         history_
            +run(instruction: string, max_steps: int) string
            +set_step_hook(hook) void
            #observe(result: string) void
            #think_and_act(step: int) variant~ToolCallAction,FinalAnswerAction~
            #execute_tool(action: ToolCallAction) expected~string,ToolError~
            #on_loop_detected() void
            #on_max_steps_reached() void
        }
    }

    LLMClient <|-- OllamaClient
    LLMClient <|-- GeminiClient
    AgentLoop o-- LLMClient : shared_ptr
    AgentLoop o-- SkillLoader : shared_ptr
    AgentLoop *-- LoopDetector
    AgentLoop ..> ToolCallAction
    AgentLoop ..> FinalAnswerAction
    AgentLoop ..> LLMConfig : uses

    %% ─────────────────────────────────────────
    %% PACKAGE 2 — Tools
    %% ─────────────────────────────────────────
    namespace Tools {
        class Tool {
            <<abstract>>
            +name() string*
            +description() string*
            +execute(args: string) expected~string,ToolError~*
        }

        class ToolError {
            <<enumeration>>
            InvalidArgument
            ExecutionFailed
            AccessDenied
            NotFound
            UnknownError
        }

        class RegistryT {
            <<template>>
            -map~string, function~Creator~~ creators_
            +register_creator(name: string, fn: Creator) void
            +create(name: string) unique_ptr~T~
            +has(name: string) bool
        }

        class ToolRegistry {
            -map~string,unique_ptr~Tool~~ tools_
            -map~string,string~          aliases_
            -set~string~                 allow_list_
            -set~string~                 deny_list_
            +register_tool(tool: unique_ptr~Tool~) void
            +register_alias(alias, canonical: string) void
            +lookup(name: string) Tool*
            +is_allowed(name: string) bool
            +normalize(name: string) string
        }

        class CalculatorTool {
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
        }

        class FileTool {
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
            -parse_args(args) pair~string,string~
        }

        class ExecTool {
            -set~string~ blocked_commands_
            -int timeout_ms_
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
        }

        class WebSearchTool {
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
        }

        class MemoryTool {
            -sqlite3* db_
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
            -memory_save(key, value) string
            -memory_search(query) string
        }

        class TimeTool {
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
        }

        class JsonTool {
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
        }

        class GitTool {
            +name() string
            +description() string
            +execute(args) expected~string,ToolError~
        }
    }

    Tool <|-- CalculatorTool
    Tool <|-- FileTool
    Tool <|-- ExecTool
    Tool <|-- WebSearchTool
    Tool <|-- MemoryTool
    Tool <|-- TimeTool
    Tool <|-- JsonTool
    Tool <|-- GitTool
    ToolRegistry *-- Tool : unique_ptr
    ToolRegistry ..> RegistryT : uses pattern
    Tool ..> ToolError
    AgentLoop *-- ToolRegistry

    %% ─────────────────────────────────────────
    %% PACKAGE 3 — Harness / Evaluator
    %% ─────────────────────────────────────────
    namespace Harness {
        class TrajectoryStep {
            +int    step
            +string thought
            +string tool_name
            +string args
            +string result
            +bool   success
            +double latency_ms
            +int    tokens
        }

        class Task {
            +string id
            +string instruction
            +string eval_type
            +string expected_output
            +string eval_script
            +int    max_steps
            +string difficulty
        }

        class TaskRunResult {
            +Task                    task
            +string                  final_answer
            +bool                    passed
            +double                  evaluator_score
            +double                  action_score
            +string                  failure_reason
            +vector~TrajectoryStep~  trajectory
        }

        class Evaluator {
            <<abstract>>
            +evaluate(output: string, expected: string, task: Task) double*
        }

        class KeywordEvaluator {
            +evaluate(output, expected, task) double
        }

        class FunctionalEvaluator {
            +evaluate(output, expected, task) double
            -run_eval_script(script, output) bool
        }

        class VLMEvaluator {
            <<skeleton>>
            +evaluate(output, expected, task) double
        }

        class HarnessRunner {
            -AgentLoop*              agent_
            -vector~unique_ptr~Evaluator~~ evaluators_
            -vector~Task~            tasks_
            +load_tasks(path: string) void
            +set_agent(agent: AgentLoop*) void
            +run_all() vector~TaskRunResult~
            +export_results(results, dir: string) void
            -find_evaluator(eval_type: string) Evaluator*
            -clean_artifacts(task: Task) void
            -compute_action_score(trajectory) double
        }
    }

    Evaluator <|-- KeywordEvaluator
    Evaluator <|-- FunctionalEvaluator
    Evaluator <|-- VLMEvaluator
    HarnessRunner *-- Evaluator : unique_ptr
    HarnessRunner --> AgentLoop : non-owning ptr
    HarnessRunner ..> Task
    HarnessRunner ..> TaskRunResult
    TaskRunResult *-- TrajectoryStep
    TaskRunResult *-- Task

    %% ─────────────────────────────────────────
    %% PACKAGE 4 — Multi-agent
    %% ─────────────────────────────────────────
    namespace MultiAgent {
        class AgentMessage {
            +string from_agent
            +string to_agent
            +string content
            +string msg_type
        }

        class MessageQueue {
            -queue~AgentMessage~ queue_
            -mutex               mtx_
            -condition_variable  cv_
            +push(msg: AgentMessage) void
            +pop() AgentMessage
            +try_pop(msg) bool
            +shutdown() void
        }

        class SubAgentConfig {
            +string name
            +string role
            +string instruction
            +int    max_steps
        }

        class MultiAgentRunner {
            -MessageQueue                  queue_
            -vector~SubAgentConfig~        configs_
            -vector~thread~                workers_
            -thread                        dispatcher_
            +add_agent(config: SubAgentConfig) void
            +run(instruction: string) void
            +stop() void
            -dispatch_loop() void
            -worker_loop(config, agent) void
        }
    }

    MultiAgentRunner *-- MessageQueue
    MultiAgentRunner ..> SubAgentConfig
    MultiAgentRunner ..> AgentMessage
    MessageQueue *-- AgentMessage

    %% ─────────────────────────────────────────
    %% PACKAGE 5 — Environment
    %% ─────────────────────────────────────────
    namespace Environment {
        class Environment {
            <<abstract>>
            +read_file(path: string) expected~string,string~*
            +write_file(path: string, content: string) expected~void,string~*
            +exec_command(cmd: string, timeout_ms: int) expected~string,string~*
            +list_dir(path: string) expected~vector~string~,string~*
            +sandbox_root() string*
        }

        class NativeEnvironment {
            +read_file(path) expected~string,string~
            +write_file(path, content) expected~void,string~
            +exec_command(cmd, timeout_ms) expected~string,string~
            +list_dir(path) expected~vector~string~,string~
            +sandbox_root() string
        }

        class SandboxEnvironment {
            -string root_
            -set~string~ allowed_cmds_
            +SandboxEnvironment(root: string)
            +read_file(path) expected~string,string~
            +write_file(path, content) expected~void,string~
            +exec_command(cmd, timeout_ms) expected~string,string~
            +list_dir(path) expected~vector~string~,string~
            +sandbox_root() string
            -validate_path(path) bool
            -is_allowed_cmd(cmd) bool
        }
    }

    Environment <|-- NativeEnvironment
    Environment <|-- SandboxEnvironment
    HarnessRunner o-- Environment : uses via interface
```

---

## Ghi chú quan hệ

| Quan hệ | Ký hiệu | Ý nghĩa |
|---|---|---|
| Inheritance | `<\|--` | Concrete class kế thừa abstract |
| Composition | `*--` | Owner sở hữu, object bị hủy cùng owner |
| Aggregation | `o--` | Owner giữ tham chiếu, object tồn tại độc lập |
| Dependency | `..>` | Dùng tạm, không sở hữu |

