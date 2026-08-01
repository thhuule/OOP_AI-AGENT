# Kế hoạch chi tiết Tuần 9 — AI-AGENT OOP 2026

> **Mục tiêu tuần:** hoàn tất UML, xây dựng bản nháp báo cáo kỹ thuật và thay README sơ sài bằng hướng dẫn có thể tái lập.
>
> **Phân công:** A — Systems/Core; B — Tools/Data; C — Evaluation/Infra.
>
> **Mốc dữ liệu lịch sử:** `benchmark/results/run_20260801_220549_361/`, provider Gemini, model `gemma-4-31b-it`, `use_mock=false`.
>
> **Nguồn ràng buộc:** `filephanchiacv/_MConverter.eu_OOP Project 2026 AI Agent.md`.
>
> **Nguyên tắc:** tài liệu phải mô tả đúng code hiện tại; không sửa task, hậu điều kiện hoặc evaluator chỉ để tăng điểm; không mô tả skeleton như tính năng đã hoàn chỉnh.

## Luồng tài liệu

```text
1. User's Requirement
        ↓
2. System Features
        ↓
3. Tech Solutions
        ↓
4. Architecture & OOP Design
        ↓
5. Detailed Logic & AI Integration
        ↓
6. Implementation & Code Structure
        ↓
7. Verification & Testing
        ↓
8. Evaluation & Benchmark
        ↓
9. Limitations & Future Work
```

---

## 1. User's Requirement — Yêu cầu người dùng và yêu cầu đồ án

### 1.1 Bài toán cần giải quyết

Xây dựng một AI Agent framework bằng C++ có khả năng kết nối LLM, lựa chọn skill, gọi tool, duy trì vòng lặp ReAct, phát hiện lặp vô hạn và được đánh giá bằng một harness có thể tái lập. Hệ thống phải được phân lớp rõ ràng, áp dụng OOP thực chất và cho phép thay thế LLM client mà không làm thay đổi agent loop.

### 1.2 Ràng buộc bắt buộc từ đề bài

| Nhóm yêu cầu | Ràng buộc cần chứng minh | Bằng chứng Tuần 9 cần chuẩn bị |
|---|---|---|
| LLM Client | Có abstract interface; hỗ trợ endpoint Ollama/OpenAI-compatible, text-only và multimodal qua cùng interface; cấu hình model, URL, temperature, max tokens; xử lý timeout/network/JSON lỗi | Class diagram, sequence agent run, báo cáo client và README configuration; ghi rõ phần multimodal đã hoàn chỉnh hay còn thiếu |
| Tool Registry | Đăng ký runtime, không hardcode trong `AgentLoop`; tool có name/description/execute; allow-list/deny-list | Class/component diagram, bảng tool, alias và policy |
| Tool tối thiểu | Có exec, read/write file, web search, memory, calculator; tham khảo để bổ sung ít nhất 3 tool thuộc 3 loại | Bảng inventory tool, source path, args, dependency, test |
| Skill System | Load Markdown, keyword selection, tối thiểu 3 skill, inject trước mỗi run | Sequence agent run, mô tả `SkillLoader`, liệt kê skill hiện có |
| Agent Loop | ReAct, parse tool call, lưu history, giới hạn `max_steps`, kết thúc graceful | Sequence diagram và phần logic chi tiết |
| Loop Detection | Phát hiện generic repeat và ping-pong; threshold cấu hình được; cảnh báo/dừng | Báo cáo thuật toán và test tập trung |
| Harness/Evaluator | Setup → run → evaluate → record; trajectory có thought/action/result/latency/tokens; ít nhất 2 evaluator; batch và JSON export | Sequence harness, report evaluation, benchmark artifacts |
| OOP | Tối thiểu 4 pattern; đúng inheritance/composition/dependency; không vi phạm abstraction layers | Class/component diagram và báo cáo OOP có dẫn chứng code |
| UML | Class diagram, sequence agent run, sequence batch evaluation, component diagram bằng Mermaid | Bốn tài liệu trong `docs/` |
| C++ | Ít nhất 4 kỹ thuật C++17, 2 kỹ thuật C++20, 2 kỹ thuật C++23 và 1 kỹ thuật C++26 | Ma trận feature → file → lý do sử dụng → fallback |
| Benchmark | Ít nhất 10 task: 4 simple, 4 medium, 2 hard; báo success rate | `benchmark/tasks.json`, run sạch và report evaluation |
| Nộp bài | Tuần 11 nộp UML; Tuần 12 source + báo cáo; Tuần 13 demo | Checkpoint, README và checklist nghiệm thu |

### 1.3 Definition of Done Tuần 9

| Đầu ra | Chủ trì | File đề xuất | Điều kiện hoàn thành |
|---|---|---|---|
| Class Diagram | A | `docs/class_diagram.md` | Mermaid render được; đúng inheritance, ownership và dependency |
| Sequence — agent run | A | `docs/sequence_agent_run.md` | Có tool success/error, final answer, `max_steps` và loop detection |
| Sequence — batch eval | A/C | `docs/sequence_harness.md` | Có load, cleanup, run, evaluate và export theo từng task |
| Component Diagram | A | `docs/component_diagram.md` | Không tạo dependency ngược giữa Agent, Tool và Harness |
| Báo cáo OOP | A | `docs/report_oop_design.md` | Mỗi nhận định có class/file minh chứng; không gắn pattern sai |
| Báo cáo Tools | B | `docs/report_tools.md` | Đủ tool, alias, args, policy, dependency và error handling |
| Báo cáo Eval | C | `docs/report_evaluation.md` | Có run thật, so sánh run hỏng/run đạt và failure taxonomy |
| README | C | `README.md` | Người mới có thể build, cấu hình và chạy đúng executable |

---

## 2. System Features — Các chức năng hệ thống

### 2.1 LLM và cấu hình

- `LLMClient` là abstraction chung; `OllamaClient` và `GeminiClient` là implementation cụ thể.
- Client gửi chat history, nhận phản hồi model và chuyển lỗi HTTP/timeout/malformed JSON thành lỗi có thể chẩn đoán.
- Khả năng gửi ảnh base64 qua cùng interface phải được đối chiếu với code; nếu chưa hoàn chỉnh, ghi vào phần giới hạn thay vì tự nhận đã hỗ trợ multimodal.
- Cấu hình phải tách khỏi source: provider, model, API URL, API key, `use_mock` và các tham số sinh.
- Tài liệu phải ưu tiên yêu cầu Ollama của đề, đồng thời giải thích Gemini là backend thay thế qua cùng abstraction.

### 2.2 Tool và dữ liệu

Các tool hiện cần được mô tả trong báo cáo:

| Nhóm | Canonical name | Trách nhiệm chính |
|---|---|---|
| Tính toán/thực thi | `calculator`, `execute_shell` | Tính biểu thức; chạy shell trong policy hạn chế |
| File | `file`, `read_file`, `write_file`, `append_file` | Đọc, ghi và nối nội dung file |
| Web/memory | `web_search`, `memory` | Tìm kiếm web; lưu/tìm memory bằng SQLite |
| Bổ sung | `time`, `json`, `git` | Thời gian, xử lý JSON và thao tác Git giới hạn |

Không mô tả `memory_save` và `memory_search` là hai tool riêng nếu registry chỉ đăng ký một `MemoryTool`; đây là hai operation được chọn qua args.

Alias hiện cần ghi đúng:

```text
calculate     -> calculator
exec          -> execute_shell
google_search -> web_search
create_file   -> write_file
```

Alias phải được normalize trước lookup và trước kiểm tra allow-list/deny-list.

### 2.3 Skill, agent loop và loop detection

- `SkillLoader` đọc các file `.md`, chọn skill theo keyword và inject hướng dẫn vào system prompt.
- Ba skill hiện có: `task_planner.md`, `error_recovery.md`, `step_verifier.md`.
- `AgentLoop` thực hiện Observe → Think → Act → Observe, duy trì history và dừng bằng final answer, `max_steps`, lỗi nghiêm trọng hoặc loop detection.
- `LoopDetector` phải phân biệt hành động theo `tool_name + normalized args`, đồng thời nhận diện generic repeat và ping-pong.

### 2.4 Harness, evaluator và multi-agent

- `HarnessRunner` load task, cô lập artifact, chạy agent, thu trajectory, chọn evaluator và export kết quả.
- `KeywordEvaluator` kiểm tra keyword; `FunctionalEvaluator` chạy `eval_script`; `VLMEvaluator` chỉ được ghi là skeleton/hướng phát triển nếu chưa chấm ảnh hoàn chỉnh.
- `MultiAgentRunner`, `MessageQueue` và `AgentMessage` cung cấp demo phối hợp bằng worker/dispatcher thread; đây là phần mở rộng, không được đồng nhất với benchmark 10 task đơn-agent.

---

## 3. Tech Solutions — Giải pháp kỹ thuật

### 3.1 Công nghệ và dependency

| Nhu cầu | Giải pháp | Lưu ý tài liệu |
|---|---|---|
| HTTP tới LLM/web | CURL | Nêu timeout, status code và lỗi kết nối |
| JSON request/response | nlohmann_json | Không parse JSON bằng regex khi cần bảo toàn escape/type |
| Persistent memory | SQLite3 | Quản lý connection/statement theo RAII |
| Filesystem/skills/artifacts | `std::filesystem` | Path validation và cleanup theo task |
| Đồng thời multi-agent | Threads, mutex, condition variable | Shutdown và join rõ ràng |
| Build | CMake trên WSL/Linux | Build toàn bộ target bằng chuẩn dự án cấu hình |

### 3.2 Kỹ thuật C++ cần truy vết

Báo cáo OOP phải lập ma trận riêng cho các kỹ thuật C++17/20/23/26, gồm: feature, vị trí dùng, mục đích, test và portability fallback. Không chỉ liệt kê tên feature.

Các minh chứng trọng tâm:

- Smart pointer cho ownership của client, tool và evaluator.
- `std::function`/lambda cho `StepHook`.
- Abstract interface và pure virtual cho `LLMClient`, `Tool`, `Evaluator`.
- `std::filesystem` cho skill/artifact.
- `std::expected` và `string_view` trên đường code hiện tại nếu được bật theo chuẩn mới.
- Giữ guarded C++26 feature path và fallback portability; GNU/Clang dùng `-std=c++26`, MSVC dùng `/std:c++latest` nơi đã cấu hình.

### 3.3 Chính sách lỗi và an toàn

- `Tool::execute(args)` trả `expected<string, ToolError>` và giữ các nhóm lỗi cụ thể: `InvalidArgument`, `ExecutionFailed`, `AccessDenied`, `NotFound`, `UnknownError`.
- Shell policy phải hạn chế command nguy hiểm, có timeout và capture stdout/stderr.
- File tool phải validate filename/content trước khi ghi; không tạo artifact nếu parse thất bại.
- Không ghi API key vào source, README, log, trajectory hoặc commit.
- Benchmark thật có thể tốn quota và tạo artifact; chỉ chạy sau khi kiểm tra config và có xác nhận của nhóm/người dùng.

---

## 4. Architecture & OOP Design — Kiến trúc và thiết kế OOP

### 4.1 Phân lớp kiến trúc

```text
Entry points ──> Agent public API ──> LLMClient
      │                 │             SkillLoader
      │                 │             ToolRegistry ──> Tool implementations
      │                 └──────────── LoopDetector
      ├────────> Harness ────────────> Evaluator + benchmark/tasks.json
      └────────> Multi-agent ────────> MessageQueue + Agent public API
```

Các dependency bị cấm:

- `AgentLoop -> HarnessRunner`.
- `Tool implementation -> AgentLoop`.
- `Evaluator -> AgentLoop internals`.

### 4.2 Class diagram bắt buộc

Chia diagram thành bốn package:

1. **Client/Core:** `LLMClient`, `GeminiClient`, `OllamaClient`, `LLMConfig`, `Message`, `AgentLoop`, `SkillLoader`, `LoopDetector`, `ToolCallAction`, `FinalAnswerAction`.
2. **Tools:** `Tool`, `ToolRegistry`, `Registry<T>` và toàn bộ concrete tool.
3. **Harness:** `Evaluator`, ba concrete evaluator, `HarnessRunner`, `Task`, `TaskRunResult`, `TrajectoryStep`.
4. **Multi-agent:** `MultiAgentRunner`, `SubAgentConfig`, `MessageQueue`, `AgentMessage`.

Quan hệ phải khớp code:

- `LLMClient <|-- GeminiClient`, `LLMClient <|-- OllamaClient`.
- `Tool <|--` từng concrete tool; `Evaluator <|--` từng concrete evaluator.
- `AgentLoop` sở hữu `ToolRegistry` và `LoopDetector`, giữ `shared_ptr` tới `LLMClient`/`SkillLoader`, và giữ callback `StepHook`.
- `ToolRegistry` sở hữu tool bằng `unique_ptr<Tool>` và quản lý alias/policy.
- `HarnessRunner` giữ con trỏ không sở hữu tới `AgentLoop`, sở hữu evaluator bằng `unique_ptr` và tạo `StepHook`.
- `MultiAgentRunner` sở hữu queue, worker threads và dispatcher thread.

### 4.3 Design pattern và SOLID

| Pattern/nguyên tắc | Vị trí | Điều cần chứng minh |
|---|---|---|
| Strategy | `Evaluator` hierarchy | Chọn evaluator theo `eval_type` mà không đổi flow chạy |
| Registry | `ToolRegistry`, `Registry<T>` | Lookup runtime, ownership, alias và policy tập trung |
| Observer/Hook | `StepHook` | Harness thu trajectory mà Agent không phụ thuộc Harness |
| Adapter | `SharedToolWrapper` trong `AgentLoop` | Chuyển shared ownership sang interface mà registry quản lý |
| RAII | Smart pointer, SQLite, thread join | Tài nguyên được giải phóng theo lifetime |
| DIP/OCP/SRP | Interface và layer boundaries | Client/tool/evaluator thay thế được, trách nhiệm tách biệt |

Không gọi `AgentLoop::run()` là **Template Method** khi các bước chưa phải hook `virtual` cho subclass override. Do đề yêu cầu pattern này, báo cáo phải ghi đây là khoảng cách tuân thủ và chọn một trong hai hướng: refactor có test, hoặc trình bày trung thực là chưa đạt trọn tiêu chí. Tương tự, không gọi `Registry<T>` là Factory nếu nó không tạo object, và không nói `ToolRegistry` dùng `Registry<T>` nội bộ nếu code chưa làm vậy.

---

## 5. Detailed Logic & AI Integration — Logic chi tiết và tích hợp AI

### 5.1 Luồng một agent run

```text
Caller -> AgentLoop::run(instruction, max_steps)
AgentLoop -> SkillLoader: chọn skill và dựng system prompt
loop mỗi step
  AgentLoop -> LLMClient::generate_chat(history)
  AgentLoop -> parse_llm_response(text)
  alt ToolCallAction
    AgentLoop -> ToolRegistry: normalize alias + policy + lookup
    AgentLoop -> Tool::execute(args)
    Tool --> AgentLoop: expected<string, ToolError>
    AgentLoop -> StepHook: thought, tool, args, result/error, latency
    AgentLoop -> LoopDetector: tool_name + normalized args
    AgentLoop -> history: observation thành công hoặc lỗi cụ thể
  else FinalAnswerAction
    AgentLoop --> Caller: final answer
  end
end
```

Sequence diagram phải có các nhánh: LLM error, parser fail, tool không tồn tại, policy từ chối, `ToolError`, đạt `max_steps`, loop detected và final answer. Lỗi tool phải được đưa lại vào history để model có cơ hội phục hồi.

### 5.2 Parse và thực thi file tool

Hai input hợp lệ của `write_file`/`append_file`:

```json
{"filename":"notes.txt","content":"Agent test run"}
```

```text
notes.txt,Agent test run
```

Với dạng chuỗi, split tại dấu phẩy đầu tiên để content vẫn có thể chứa dấu phẩy. Với JSON, dùng parser thật, kiểm tra type và field. Chỉ ghi sau khi filename/content hợp lệ. `append_file` phải bảo toàn nội dung cũ và trả lỗi rõ ràng khi thao tác thất bại.

### 5.3 Luồng batch evaluation

```text
run_eval -> HarnessRunner::loadTasks()
run_eval -> AgentLoop: set_step_hook(...)
run_eval -> HarnessRunner: set_agent(...), runAll()
HarnessRunner -> HarnessRunner: cleanBenchmarkArtifacts(task)
loop 10 task
  HarnessRunner -> AgentLoop::run(task.instruction, task.max_steps)
  AgentLoop -> StepHook: trajectory từng bước
  HarnessRunner -> findEvaluator(task.eval_type)
  HarnessRunner -> Evaluator::evaluate(output, expected)
  HarnessRunner -> HarnessRunner: kiểm tra tool step + hậu điều kiện + failure reason
end
run_eval -> HarnessRunner::exportResults(results)
```

`FunctionalEvaluator` chạy `eval_script`; `KeywordEvaluator` đối chiếu output. Với task bắt buộc dùng tool, final success phải kết hợp evaluator-level, action-level và hậu điều kiện artifact.

---

## 6. Implementation & Code Structure — Triển khai và cấu trúc mã nguồn

### 6.1 Ánh xạ module

| Module | Source chính | Chủ trì tài liệu |
|---|---|---|
| Client/Core | `src/client/`, `src/agent/` | A |
| Tools/Data | `src/tools/` | B |
| Harness/Evaluator | `src/harness/`, `benchmark/` | C |
| Skills | `src/skills/` | A |
| Multi-agent | `src/multiagent/`, `benchmark/test_multi_agent.cpp`, `benchmark/demo_multi_agent.cpp` | C |
| Build/config | `CMakeLists.txt`, `config.json.example`, `README.md` | C, review chéo A/B |
| Technical docs | `docs/` | A chủ trì, B/C cung cấp nội dung |

### 6.2 Nội dung báo cáo theo role

**A — Systems/Core**

- Hoàn tất bốn UML và `docs/report_oop_design.md`.
- Chứng minh interface, ownership, pattern, SOLID, ReAct, parsing, error propagation và loop detection bằng source hiện có.
- Ghi riêng ma trận C++17/20/23/26 và portability fallback.
- Commit đề xuất: `[Week9-A] UML diagrams and OOP design report`.

**B — Tools/Data**

- Hoàn tất `docs/report_tools.md` với bảng: class/file, canonical name, args, output, dependency, risk và test evidence.
- Giải thích alias, allow/deny policy, file args, append, shell restriction, CURL error và SQLite lifecycle.
- Dùng case study run `212302_253` → `220549_361` để minh họa sửa parsing, bảo toàn `ToolError` và trajectory args.
- Commit đề xuất: `[Week9-B] Document tools architecture and validation`.

**C — Evaluation/Infra**

- Hoàn tất `docs/report_evaluation.md` và cập nhật README.
- Mô tả task loading, artifact isolation, trajectory, evaluator selection, scoring, failure taxonomy và export.
- Ghi rõ token hiện bằng `0` là giới hạn đo lường, không phải model không dùng token.
- Mô tả multi-agent như phần mở rộng có test/demo riêng.
- Commit đề xuất: `[Week9-C] Evaluation report and reproducible README`.

### 6.3 README bắt buộc

README cần có: Overview, Architecture, Prerequisites, Build, Configuration, Executables, Benchmark Output, Security và Troubleshooting.

Lệnh chuẩn trên WSL/Linux:

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

Hướng dẫn copy `config.json.example` thành `config.json`; tuyệt đối không chép API key thật vào tài liệu hoặc commit.

---

## 7. Verification & Testing — Kiểm chứng và kiểm thử

### 7.1 Quy trình xác minh

Với mỗi component thay đổi:

1. Đọc source và cập nhật tài liệu theo code hiện tại.
2. Build toàn bộ target bằng WSL.
3. Chạy executable nhỏ nhất liên quan trước.
4. Render Mermaid và kiểm tra link/source path trong Markdown.
5. Chạy benchmark thật cuối cùng chỉ khi config an toàn và chi phí/quota được xác nhận.

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_multi_agent
```

### 7.2 Test tối thiểu cần bổ sung

Thư mục `src/tests` hiện chưa có bộ unit test thực thi rõ ràng. Khoảng trống này phải được nêu trong báo cáo và xử lý bằng test tập trung:

- File write: JSON args, comma args, content chứa dấu phẩy, thiếu field, filename rỗng.
- Append: file có/không tồn tại, newline và nội dung đọc lại.
- Registry: canonical name, alias, allow-list, deny-list, unknown tool.
- Calculator/JSON: input hợp lệ và malformed.
- Exec: thành công, non-zero exit, timeout và command bị policy từ chối.
- Parser: JSON escape, object/string args, tool call malformed.
- Loop detector: repeat cùng args, cùng tool khác args và ping-pong.
- Evaluator: keyword, functional PASS/FAIL, evaluator error và artifact mismatch.

### 7.3 Checklist nghiệm thu Tuần 9

- [ ] Bốn Mermaid diagram render không lỗi và khớp tên class/hàm trong source.
- [ ] Không gọi sai design pattern hoặc mô tả skeleton như tính năng hoàn chỉnh.
- [ ] Ma trận yêu cầu đề bài → feature → source → test không còn mục bắt buộc bị bỏ quên.
- [ ] Bảng tool ghi đúng canonical name, alias, args, error và dependency.
- [ ] Trajectory task 005/010 giữ args thật và hậu điều kiện chính xác.
- [ ] README không còn lệnh `./OopAgent --chat` hoặc đường dẫn output sai.
- [ ] Build toàn bộ target thành công; `test_multi_agent` pass.
- [ ] Không có API key, config thật, build output hay task artifact trong commit.
- [ ] Mỗi thành viên có commit riêng và một lượt review chéo.

---

## 8. Evaluation & Benchmark — Đánh giá và benchmark

### 8.1 Benchmark bắt buộc

`benchmark/tasks.json` là nguồn sự thật: tối thiểu 10 task, gồm 4 simple, 4 medium và 2 hard. Task yêu cầu tool phải có ít nhất một tool step thực sự thành công. Task tạo file phải kiểm tra đúng filename và content được tạo trong chính run hiện tại.

### 8.2 Kết quả lịch sử gần nhất được dùng trong kế hoạch

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

Đây là bằng chứng lịch sử, không thay cho run xác nhận cuối từ trạng thái sạch.

### 8.3 Phân tích regression

So với `run_20260801_212302_253` (2/10, evaluator score 0.2, action-level score 1.0), lỗi gốc lớn nhất là `FileWriteTool` parse args sai. Action-level cũ lạc quan giả vì tool trả `OK` dù artifact sai. Run mới cho thấy filename/content đúng, có `append_file`, args xuất hiện trong trajectory, lỗi `NotFound` được giữ và task 010 phục hồi.

Trước run xác nhận cuối:

- Xóa/cô lập artifact của từng task, gồm `notes.txt`, `result.txt`, `capital.txt`, `calc.txt`, `data.txt`, `output.txt` và các filename malformed.
- Kiểm tra `config.json` mà không in API key.
- Xác nhận `use_mock=false` nếu báo cáo kết quả provider thật.
- Không báo 10/10 dựa trên file cũ ở repository root.

### 8.4 Failure taxonomy

```text
PARSER_FAIL
TOOL_NOT_FOUND
INVALID_ARGS
TOOL_EXECUTION_FAILED
ARTIFACT_MISSING
ARTIFACT_CONTENT_MISMATCH
LOOP_DETECTED
INCOMPLETE_TASK
EVALUATOR_ERROR
RATE_LIMIT
```

Mỗi failure phải có task id, step gây lỗi, args thực tế, hậu điều kiện mong đợi và role chịu trách nhiệm; không gom mọi lỗi thành `POST_CONDITION_FAIL`.

---

## 9. Limitations & Future Work — Giới hạn và hướng phát triển

### 9.1 Giới hạn hiện tại

- `AgentLoop::run()` chưa chứng minh đầy đủ Template Method theo định nghĩa trong đề.
- `VLMEvaluator` chưa được mô tả như evaluator thị giác hoàn chỉnh nếu vẫn là skeleton.
- Token usage trong trajectory còn bằng `0`, nên chưa đánh giá được chi phí/token.
- `src/tests` chưa có bộ unit test thực thi đầy đủ cho parser, tool, loop detector và evaluator.
- Kết quả 10/10 hiện có là lịch sử; cần run sạch hiện thời trước báo cáo cuối.
- `Environment` abstraction trong đề phải được đối chiếu với code; nếu chưa có `NativeEnvironment`/`SandboxEnvironment`, ghi là khoảng cách tuân thủ thay vì tự nhận đã đạt.
- Multi-agent là điểm mở rộng và demo riêng, chưa phải bằng chứng cho benchmark đơn-agent.

### 9.2 Hướng phát triển

**Tuần 10:** bug fix, sanitizer/Valgrind, hoàn thiện test, slide và rehearsal.

**Tuần 11:** nộp UML đã review; vì vậy bốn diagram phải hoàn tất ở Tuần 9, không chỉ là bản nháp.

**Trước Tuần 12:** chốt source, báo cáo, README, benchmark sạch, kiểm tra secret và khả năng tái lập.

Các hướng điểm thưởng chỉ triển khai khi phần bắt buộc ổn định:

- GUI/VLM agent với screenshot và action executor.
- Persistent memory dùng embedding/vector search.
- Multi-agent coordination với message queue và worker thread.

### 9.3 Quy ước Git và bàn giao

- Mỗi thành viên tối thiểu 6 commit; nhóm 2 người tối thiểu 12, nhóm 3 người tối thiểu 18.
- Khoảng cách giữa hai commit gần nhất không quá 7 ngày.
- Chênh lệch lượng đóng góp source giữa các thành viên không quá 20%; chỉ làm báo cáo/test không được tính là đóng góp mã nguồn đầy đủ theo yêu cầu đề.
- Không commit `config.json`, API key, `build/`, database, task artifact hoặc benchmark run phát sinh nếu nhóm chưa thống nhất.
- Trước nộp, chuẩn bị quyền truy cập repository private theo hướng dẫn của giảng viên và bảo đảm mọi thành viên giải thích được phần code của mình.

---

## Checkpoint thực hiện

### Đầu tuần

- C chốt run id và bảng số liệu; B chốt danh sách tool/alias; A chốt danh sách class và quan hệ.
- Cả nhóm thống nhất dùng `docs/` ở repository root, không trộn tài liệu mới với ảnh cũ trong `src/docs/`.

### Giữa tuần

- A mở review UML để B/C kiểm tra quan hệ thuộc layer của mình.
- B/C hoàn thành báo cáo phần mình và gắn link source/result tương đối.
- C cập nhật README và thử build từ môi trường/build directory sạch.

### Cuối tuần

- Build toàn bộ target và chạy `test_multi_agent`.
- Review chéo tài liệu theo ma trận yêu cầu.
- Chỉ chạy `run_eval` sau khi xác nhận config không lộ key và chi phí/quota đã được đồng ý.
