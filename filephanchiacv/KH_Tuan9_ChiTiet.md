# Kế hoạch chi tiết Tuần 9 — AI-AGENT OOP 2026

> **Mục tiêu tuần:** hoàn tất bộ UML, viết bản nháp báo cáo kỹ thuật và thay README sơ sài bằng hướng dẫn có thể tái lập.
> **Phân công:** A — Systems/Core; B — Tools/Data; C — Evaluation/Infra.
> **Mốc dữ liệu:** run thật `benchmark/results/run_20260801_220549_361/`, provider Gemini, model `gemma-4-31b-it`, `use_mock=false`.
> **Nguyên tắc:** tài liệu phải mô tả đúng code hiện tại; không sửa evaluator hoặc hậu điều kiện chỉ để tăng điểm.

---

## 1. Đầu ra chung và Definition of Done

| Đầu ra | Người chính | File đề xuất | Điều kiện hoàn thành |
|---|---|---|---|
| Class Diagram | A | `docs/class_diagram.md` | Mermaid render được; đúng ownership, inheritance và dependency |
| Sequence — agent run | A | `docs/sequence_agent_run.md` | Có nhánh tool success/error, final answer và loop detection |
| Sequence — batch eval | A | `docs/sequence_harness.md` | Có load, cleanup, run, evaluate, export theo từng task |
| Component Diagram | A | `docs/component_diagram.md` | Không tạo dependency ngược giữa Agent, Tool và Harness |
| Báo cáo OOP | A | `docs/report_oop_design.md` | Mỗi nhận định có class/file minh chứng |
| Báo cáo Tools | B | `docs/report_tools.md` | Đủ tool, alias, parse args, policy và error handling |
| Báo cáo Eval | C | `docs/report_evaluation.md` | Có số liệu thật, phân tích run hỏng và run đạt |
| README | C | `README.md` | Người mới build và chạy đúng executable chỉ từ README |

Quy ước chung:

- Mỗi người có ít nhất một commit riêng, tiêu đề theo mẫu `[Week9-A] ...`, `[Week9-B] ...`, `[Week9-C] ...`.
- Trước commit: build toàn bộ target bằng WSL, kiểm tra link Markdown và render Mermaid.
- Không commit `config.json`, API key, `build/`, `memory.db`, file task (`notes.txt`, `result.txt`, ...) hoặc run benchmark phát sinh nếu nhóm chưa thống nhất.
- Mỗi tài liệu phải có mục “Giới hạn/việc còn lại”; không mô tả skeleton như tính năng đã hoàn chỉnh.

---

## 2. A — Systems / Core (chủ trì UML)

### 2.1 Class Diagram toàn hệ thống

Chia diagram thành bốn package để dễ đọc:

1. **Client/Core:** `LLMClient`, `GeminiClient`, `OllamaClient`, `LLMConfig`, `Message`, `AgentLoop`, `SkillLoader`, `LoopDetector`, `ToolCallAction`, `FinalAnswerAction`.
2. **Tools:** `Tool`, `ToolRegistry`, `Registry<T>`, `CalculatorTool`, `ExecTool`, `FileTool`, `FileReadTool`, `FileWriteTool`, `FileAppendTool`, `WebSearchTool`, `MemoryTool`, `TimeTool`, `JsonTool`, `GitTool`.
3. **Harness:** `Evaluator`, `KeywordEvaluator`, `FunctionalEvaluator`, `VLMEvaluator`, `HarnessRunner`, `Task`, `TaskRunResult`, `TrajectoryStep`.
4. **Multi-agent:** `MultiAgentRunner`, `SubAgentConfig`, `MessageQueue`, `AgentMessage`.

Các quan hệ bắt buộc phải đúng với code:

- `LLMClient <|-- GeminiClient`, `LLMClient <|-- OllamaClient`.
- `Tool <|--` từng concrete tool; `Evaluator <|--` ba evaluator.
- `AgentLoop` **sở hữu** `ToolRegistry` và `LoopDetector`; giữ `shared_ptr` tới `LLMClient`/`SkillLoader`; giữ callback `StepHook`.
- `ToolRegistry` sở hữu tool bằng `unique_ptr<Tool>` và quản lý alias/allow-list/deny-list.
- `HarnessRunner` giữ con trỏ không sở hữu tới `AgentLoop`, sở hữu evaluator bằng `unique_ptr`, và tạo `StepHook`.
- `MultiAgentRunner` sở hữu các `MessageQueue`, worker thread và dispatcher thread.

Không ghi `AgentLoop::run()` là **Template Method**: hiện hàm này không có các bước `virtual` cho subclass override. Nếu báo cáo bắt buộc pattern này, A phải đề xuất/refactor riêng và có test; không gắn nhãn sai cho code hiện tại.

### 2.2 Sequence Diagram — một agent run

Luồng chính cần thể hiện:

```text
Caller -> AgentLoop::run(instruction, max_steps)
AgentLoop -> SkillLoader: lấy skill và dựng system prompt
loop mỗi step
  AgentLoop -> LLMClient::generate_chat(history)
  AgentLoop -> parse_llm_response(text)
  alt ToolCallAction
    AgentLoop -> ToolRegistry: resolve alias + kiểm tra policy + lookup
    AgentLoop -> Tool::execute(args)
    Tool --> AgentLoop: expected<string, ToolError>
    AgentLoop -> StepHook: thought, tool + args, result/error
    AgentLoop -> LoopDetector: kiểm tra tool_name + normalized args
  else FinalAnswerAction
    AgentLoop --> Caller: final answer
  end
end
```

Bổ sung các nhánh lỗi: LLM error, parser fail, tool không tồn tại, policy từ chối, `ToolError`, quá `max_steps`, và loop detected. Sơ đồ phải cho thấy lỗi tool được đưa lại vào history để model có thể phục hồi.

### 2.3 Sequence Diagram — HarnessRunner batch evaluation

Thể hiện đúng thứ tự:

```text
run_eval -> HarnessRunner::loadTasks()
run_eval -> AgentLoop: set_step_hook(...)
run_eval -> HarnessRunner: set_agent(...), runAll()
HarnessRunner -> HarnessRunner: cleanBenchmarkArtifacts()
loop 10 task
  HarnessRunner -> AgentLoop::run(task.instruction, task.max_steps)
  AgentLoop -> StepHook: trajectory từng bước
  HarnessRunner -> findEvaluator(task.eval_type)
  HarnessRunner -> Evaluator::evaluate(output, expected)
  HarnessRunner -> HarnessRunner: kiểm tra tool step + phân loại failure
end
run_eval -> HarnessRunner::exportResults(results)
```

Ghi rõ `FunctionalEvaluator` chạy `eval_script`; `KeywordEvaluator` đối chiếu output; success cuối cùng kết hợp evaluator-level và action-level đối với task bắt buộc dùng tool.

### 2.4 Component Diagram

Các component: CLI/benchmark entry points, Client, Agent, Skills, Tool + SQLite/CURL/filesystem/shell, Harness + task spec, Multi-agent, và result artifacts. Dependency hợp lệ:

```text
Entry points -> Agent, Harness, Multi-agent
Agent -> LLMClient, Skills, ToolRegistry, LoopDetector
ToolRegistry -> Tool implementations
Harness -> Agent public API, Evaluator, benchmark/tasks.json
Multi-agent -> MessageQueue, Agent public API
```

Không vẽ `AgentLoop -> HarnessRunner`, `Tool -> AgentLoop` hoặc `Evaluator -> AgentLoop internals`.

### 2.5 Báo cáo thiết kế OOP

Giải thích bằng code hiện có:

| Chủ đề/pattern | Vị trí | Ý chính cần chứng minh |
|---|---|---|
| Strategy | `Evaluator` hierarchy | Harness chọn evaluator theo `eval_type` mà không đổi flow chạy |
| Registry | `ToolRegistry`, `Registry<T>` | Lookup runtime, ownership bằng smart pointer, alias và policy tập trung |
| Observer/Callback | `StepHook` | Harness thu trajectory mà Agent không phụ thuộc Harness |
| Adapter | `SharedToolWrapper` trong `AgentLoop` | Chuyển `shared_ptr<T>` sang interface Tool do registry sở hữu |
| RAII | smart pointer, SQLite destructor, thread join | Tài nguyên được giải phóng theo lifetime đối tượng |

Phân tích thêm SOLID: DIP qua `LLMClient`/`Tool`/`Evaluator`, OCP khi thêm client/tool/evaluator, SRP giữa Agent–Tool–Harness. Nêu trung thực rằng `ToolRegistry` hiện chưa dùng `Registry<T>` nội bộ và Registry không phải Factory nếu nó không tạo object.

**Commit:** `[Week9-A] UML diagrams and OOP design report`

---

## 3. B — Tools / Data

### 3.1 Nội dung báo cáo Tools

Mô tả contract chung trong `src/tools/Tool.h`:

- `get_name()` và `get_description()` trả `string_view`, `noexcept`, tránh copy chuỗi tĩnh.
- `execute(args)` trả `expected<string, ToolError>`; phân biệt `InvalidArgument`, `ExecutionFailed`, `AccessDenied`, `NotFound`, `UnknownError`.
- Tool được đăng ký qua `ToolRegistry`, không hardcode logic thực thi vào `AgentLoop`.

Lập bảng cho từng tool với các cột: class/file, canonical name, format args, output, dependency, rủi ro và test minh chứng. Ít nhất bao phủ:

| Nhóm | Canonical name |
|---|---|
| Tính toán/thực thi | `calculator`, `execute_shell` |
| File | `file`, `read_file`, `write_file`, `append_file` |
| Web/memory | `web_search`, `memory` |
| Bổ sung | `time`, `json`, `git` |

Không ghi `memory_save` và `memory_search` là hai tool riêng nếu code vẫn chỉ đăng ký một `MemoryTool`; giải thích chúng là hai operation được chọn qua args.

### 3.2 Alias, policy và parse args

Ghi rõ alias hiện có trong `ToolRegistry.cpp`:

```text
calculate -> calculator
exec -> execute_shell
google_search -> web_search
create_file -> write_file
```

Giải thích alias được normalize trước lookup và trước allow/deny check. Với file tool, trình bày hai input hợp lệ:

```json
{"filename":"notes.txt","content":"Agent test run"}
```

```text
notes.txt,Agent test run
```

Split chuỗi tại dấu phẩy đầu tiên, validate filename/content trước khi ghi, trả lỗi cụ thể nếu sai; `append_file` phải giữ nội dung cũ. Nêu chính sách hạn chế shell, timeout/capture output, xử lý file không tồn tại, CURL error và SQLite lifecycle.

### 3.3 Bằng chứng và kiểm thử cần bổ sung

Thư mục `src/tests` hiện chưa có unit test thực thi được. B phải ghi đây là khoảng trống và đề xuất test tối thiểu:

- File write: JSON args, comma args, content chứa dấu phẩy, thiếu field, filename rỗng.
- Append: file có/không tồn tại, newline, đọc lại đúng nội dung.
- Registry: canonical name, alias, allow-list, deny-list, unknown tool.
- Calculator/JSON: input hợp lệ và malformed.
- Exec: command thành công, non-zero exit, timeout, command bị policy từ chối.

Đính kèm case study run `212302_253` (2/10) → run `220549_361` (10/10): lỗi gốc là `FileWriteTool` parse sai; bản sửa hỗ trợ args, append, bảo toàn `ToolError` và trajectory args.

**Commit:** `[Week9-B] Document tools architecture and validation`

---

## 4. C — Evaluation / Infra

### 4.1 Evaluator và HarnessRunner

Mô tả đúng trách nhiệm:

- `KeywordEvaluator`: tách danh sách keyword từ task spec và kiểm tra trong final answer.
- `FunctionalEvaluator`: chạy `eval_script`, chỉ pass khi kết quả xác nhận `PASS`.
- `VLMEvaluator`: skeleton/hướng phát triển; không tuyên bố đã chấm ảnh hoàn chỉnh.
- `HarnessRunner`: load task, xóa artifact đã khai báo, chạy agent, ghi trajectory, chọn evaluator, tính evaluator/action/final score, phân loại failure và export.
- `EvalResult` chứa `is_passed`, `score`, `feedback`; `expected` tách lỗi kỹ thuật của evaluator khỏi kết quả FAIL hợp lệ.

Giải thích trajectory phải giữ **args thật**, tool result/error, latency và step id. Token hiện đang bằng `0`, nên báo cáo là giới hạn đo lường, không suy diễn thành model không dùng token.

### 4.2 Kết quả benchmark thật

Nguồn: `benchmark/results/run_20260801_220549_361/eval_results.json` và `benchmark_summary.txt`.

| Category | Tasks | Passed | Failed | Success Rate |
|---|---:|---:|---:|---:|
| Simple | 4 | 4 | 0 | 100% |
| Medium | 4 | 4 | 0 | 100% |
| Hard | 2 | 2 | 0 | 100% |
| **Total** | **10** | **10** | **0** | **100%** |

| Chỉ số | Giá trị |
|---|---:|
| Evaluator score | 1.0 |
| Action-level score | 1.0 |
| Final success rate | 1.0 |

So sánh với `run_20260801_212302_253`: 2/10, evaluator score 0.2 nhưng action-level score 1.0. Phân tích vì sao action-level cũ lạc quan giả: tool trả `OK` dù artifact sai. Nêu các sửa chữa đã quan sát trong run mới: đúng filename/content, có `append_file`, args xuất hiện trong trajectory, lỗi `NotFound` được giữ và task 010 phục hồi thành công.

Trước khi chốt báo cáo, C phải chạy lại từ trạng thái sạch và ghi run id mới; 10/10 hiện tại là bằng chứng lịch sử, không thay cho lần xác nhận cuối. Kiểm tra không còn artifact tên lỗi như `{"filename":...` hoặc `{"content":...` làm nhiễu task 001.

### 4.3 Failure taxonomy

Dù run mới không fail, báo cáo vẫn định nghĩa các nhóm để debug:

```text
PARSER_FAIL, TOOL_NOT_FOUND, INVALID_ARGS, TOOL_EXECUTION_FAILED,
ARTIFACT_MISSING, ARTIFACT_CONTENT_MISMATCH, LOOP_DETECTED,
INCOMPLETE_TASK, EVALUATOR_ERROR, RATE_LIMIT
```

Mỗi failure cần: task id, step gây lỗi, args thực tế, hậu điều kiện mong đợi và role chịu trách nhiệm. Không gom mọi lỗi thành `POST_CONDITION_FAIL`.

### 4.4 Multi-agent

Trình bày `MultiAgentRunner`, `SubAgentConfig`, `MessageQueue`, `AgentMessage`, worker threads, dispatcher, mutex/condition variable và shutdown/join. Bằng chứng nghiệm thu:

```bash
./build/test_multi_agent
./build/demo_multi_agent
```

Ghi rõ scenario, message flow và nội dung `report.txt`; không đồng nhất demo multi-agent với 10 task benchmark đơn-agent.

### 4.5 README phải cập nhật

README cần có cấu trúc: Overview, Architecture, Prerequisites, Build, Configuration, Executables, Benchmark Output, Security và Troubleshooting.

Lệnh chuẩn hỗ trợ trong WSL/Linux:

```bash
sudo apt install cmake g++ libcurl4-openssl-dev nlohmann-json3-dev libsqlite3-dev
cmake -S . -B build
cmake --build build -j2
./build/test_multi_agent
./build/demo_multi_agent
./build/run_eval
```

Giải thích đúng executable:

- `OopAgent`: smoke test `GeminiClient` hiện tại, chưa phải `--chat` mode.
- `run_eval`: chạy 10 task và ghi vào thư mục run có timestamp.
- `test_multi_agent`: test local, chạy trước benchmark có mạng.
- `demo_multi_agent`: demo phối hợp nhiều agent.

Config phải hướng dẫn copy `config.json.example` thành `config.json`, giải thích `provider`, `model`, `api_url`, `api_key`, `use_mock`; tuyệt đối không đưa key thật vào tài liệu. Phải cảnh báo `run_eval` có thể gọi provider thật, tốn quota và sinh artifact.

**Commit:** `[Week9-C] Evaluation report and reproducible README`

---

## 5. Thứ tự thực hiện và checkpoint

### Đầu tuần

- C chốt run id/bảng số liệu; B chốt danh sách tool/alias; A chốt danh sách class và quan hệ.
- Cả nhóm thống nhất dùng `docs/` ở repository root, không trộn với ảnh cũ trong `src/docs/`.

### Giữa tuần

- A mở PR UML để B/C kiểm tra quan hệ thuộc layer của mình.
- B/C hoàn thành báo cáo phần mình và gắn link source/result tương đối.
- C cập nhật README, thử lại từ một môi trường/build directory mới.

### Cuối tuần

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_multi_agent
```

Chỉ sau khi xác nhận config không lộ key và cả nhóm đồng ý chi phí/quota mới chạy:

```bash
./build/run_eval
```

Checklist nghiệm thu:

- [ ] Mermaid render không lỗi và khớp tên class/hàm trong source.
- [ ] Báo cáo không gọi sai pattern hoặc mô tả skeleton như tính năng hoàn chỉnh.
- [ ] Bảng benchmark trỏ đến đúng run id, đủ evaluator/action/final score.
- [ ] Trajectory của task 005/010 có args thật và post-condition đúng.
- [ ] README không còn lệnh `./OopAgent --chat` hoặc đường dẫn output sai.
- [ ] Build toàn bộ target thành công; `test_multi_agent` pass.
- [ ] Không có secret, artifact benchmark hoặc thay đổi ngoài phạm vi trong commit.
- [ ] Mỗi thành viên có ít nhất một commit và một lượt review chéo.

> **Mốc kế tiếp:** Tuần 10 dành cho bug fix, sanitizer/Valgrind, slide và rehearsal; Tuần 11 nộp UML. Vì vậy UML phải được review xong trong Tuần 9, không chỉ tạo file nháp.
