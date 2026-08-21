# KỊCH BẢN THUYẾT TRÌNH + DEMO FLOW — AI AGENT FRAMEWORK

> **Deck chuẩn:** `AI_Agent_Framework_Presentation_Formatted.pptx` — 18 slide.
>
> **Thời lượng mục tiêu:** 15–18 phút, demo đan xen đúng phần đang trình bày.
>
> **Nguyên tắc:** không đọc nguyên văn slide; mỗi slide nói một ý chính rồi chuyển sang bằng chứng gần nhất.

## Quy tắc an toàn khi trình bày

- Không mở `config.json`, API key, token hoặc secret.
- Không chạy `run_eval` live nếu mạng/quota không ổn định; dùng evidence của run `run_20260820_002933_100`.
- Không nói “zero warning”, “benchmark 10/10”, “OopAgent chat”, hoặc claim `std::inplace_vector` đang hoạt động.
- Phân biệt rõ: CTest kiểm tra code; benchmark 7/10 đo hành vi model thật.

## Phân vai và thời gian

| Phần | Người nói | Slide | Demo | Thời gian |
|---|---|---:|---|---:|
| Mở đầu và khái niệm | Role A | 1–3 | — | 2 phút |
| Agent Core | Role A | 4–7 | Build + AgentLoop/LoopDetector | 4 phút |
| OOP, C++ và Tools | Role B | 8–12 | Registry + Vector test | 4 phút |
| Harness và Multi-agent | Role C | 13–15 | CTest + Multi-agent | 3.5 phút |
| Kết quả và kết luận | Role C | 16–18 | Benchmark evidence | 3 phút |

---

# ROLE A — MỞ ĐẦU VÀ AGENT CORE

## Slide 1 — AI Agent Framework

### Lời nói

> Kính chào thầy và các bạn. Nhóm chúng em trình bày AI Agent Framework viết bằng Modern C++, kết nối Ollama và Gemini. Hệ thống cho phép mô hình chọn và gọi công cụ, lưu memory, chạy benchmark và xuất evidence. Nhóm chia ba vai trò: Systems/Core, Tools/Data và Eval/Infra.

### Chuyển slide

> Trước tiên, em xin đi qua lộ trình của toàn bộ bài trình bày.

## Slide 2 — Roadmap

### Lời nói

> Bài trình bày đi từ Agent Core, các nguyên lý OOP và Modern C++, sang hệ thống tool và Vector Search, rồi kết thúc bằng Harness, Multi-agent và benchmark thật. Mỗi phần đều gắn với code hoặc evidence có thể kiểm tra, nên nhóm không chỉ mô tả kiến trúc trên slide.

### Chuyển slide

> Để theo dõi các phần sau, chúng ta chỉ cần nắm sáu khái niệm nền tảng.

## Slide 3 — Sáu khái niệm giúp đọc toàn bộ hệ thống

### Lời nói

> LLM là mô hình sinh quyết định hoặc câu trả lời. Tool là chức năng có input, output và lỗi rõ ràng. Khi ghép LLM với vòng điều khiển, trạng thái và tool, ta có AI Agent. Agent chạy theo ReAct: Think, Act, Observe rồi lặp hoặc kết thúc. Harness là bộ chạy và chấm task; Trajectory là nhật ký từng bước để giải thích kết quả.

### Chuyển slide

> Từ các khái niệm đó, điểm khác biệt đầu tiên là Agent có thể biến quyết định thành hành động.

## Slide 4 — Từ chatbot sang Agent có khả năng hành động

### Lời nói

> Chatbot chỉ trả văn bản. Agent thêm vòng lặp Think, Act, Observe và Continue hoặc Finish. Mô hình chọn bước; framework chạy tool; kết quả được đưa lại vào history. Vì vậy hệ thống vừa mở rộng được, vừa ghi nhận lỗi và có thể đánh giá tái lập.

## Slide 5 — Các tầng được tách bằng contract rõ ràng

### Lời nói

> `LLMClient` che giấu khác biệt giữa Gemini và Ollama. `Agent Core` điều phối ReAct và history. Tầng Tools cung cấp Registry, Factory, policy và `ToolError`. Tầng Evaluation chứa Harness, Evaluator Strategy, Trajectory và StepHook. Agent chỉ phát sự kiện; Harness quan sát mà không tạo dependency ngược vào Core.

### Demo 1 — Configure và build

```bash
cmake -S . -B build
cmake --build build -j2
```

### Lời dẫn demo

> Hai lệnh này cấu hình và build toàn bộ project. Kết luận cần nói là build hoàn tất, không có compile error. Không kết luận “zero warning” vì dependency có thể phát warning tùy compiler.

## Slide 6 — AgentLoop điều phối LLM, tool và history

### Lời nói

> `LLMClient` dùng chung một contract cho text và ảnh base64, với endpoint, model, temperature, max token và timeout từ cấu hình. `AgentLoop` giữ đúng thứ tự history: system, user, assistant và tool. Parser hỗ trợ JSON thường, fenced JSON và escaped arguments. Nếu tool call bị hỏng, hệ thống trả `PARSE_ERROR` và không chạy một lệnh bị cắt dở.

### Mở code

1. [`agent_loop.h`](../../src/agent/agent_loop.h)
2. [`agent_loop.cpp`](../../src/agent/agent_loop.cpp)

### Điểm cần chỉ

- `AgentLoop::run()` giữ skeleton ReAct.
- System prompt lấy catalog động từ Registry.
- Assistant response và tool observation được thêm vào history.
- StepHook ghi từng bước mà không buộc Agent biết Harness.

## Slide 7 — Loop Detector và Skill System giữ Agent đúng hướng

### Lời nói

> `LoopDetector` phát hiện việc lặp cùng action hoặc ping-pong giữa hai action. Mức warning cho Agent cơ hội sửa; mức critical dừng an toàn. `SkillLoader` chọn skill theo keyword của task. Nếu không có skill khớp, `task_planner` là mặc định, nên hệ thống không bị rơi vào trạng thái thiếu hướng dẫn.

### Mở code

1. [`LoopDetector.cpp`](../../src/agent/LoopDetector.cpp)
2. [`SkillLoader.cpp`](../../src/agent/SkillLoader.cpp)
3. [`task_planner.md`](../../skills/task_planner.md)

### Chuyển người nói

> Phần Core đã giữ vòng chạy an toàn. Tiếp theo, Role B trình bày các pattern, tính năng Modern C++ và cách mở rộng tool.

---

# ROLE B — OOP, MODERN C++ VÀ TOOLS

## Slide 8 — Bốn design pattern nằm trên production path

### Lời nói

> Bốn pattern đều nằm trên luồng chạy thật. Strategy xuất hiện ở Tool và Evaluator. Template Method nằm trong `AgentLoop::run()`. Registry/Factory nằm ở `ToolRegistry`. Observer được hiện thực bằng StepHook để Harness ghi trajectory. Các pattern này được kiểm tra bằng focused test, không chỉ xuất hiện trong sơ đồ.

### Code mapping

- Strategy/Template Method: [`agent_loop.h`](../../src/agent/agent_loop.h)
- Registry/Factory: [`ToolRegistry.cpp`](../../src/tools/ToolRegistry.cpp)
- Observer/Hook: [`HarnessRunner.cpp`](../../src/harness/HarnessRunner.cpp)

## Slide 9 — Modern C++ làm rõ ownership và contract

### Lời nói

> C++17 cung cấp smart pointer, filesystem, variant và optional. C++20 dùng ranges và views. C++23 dùng `std::expected` và `std::println`. Với C++26, `MultiAgentRunner` xóa copy constructor và copy assignment kèm lý do vì object sở hữu worker threads. Nhóm không claim `std::inplace_vector`, vì standard library hiện tại chưa hỗ trợ đầy đủ.

### Mở code

- [`MultiAgentRunner.h`](../../src/multiagent/MultiAgentRunner.h), chỉ hai dòng `= delete("...")`.

## Slide 10 — ToolRegistry mở rộng tool mà không sửa Agent Core

### Lời nói

> Concrete tool chỉ cần cung cấp tên, mô tả và `execute(arguments)`. Tool đăng ký vào Registry; Agent tra cứu hoặc tạo instance mới mà không phụ thuộc concrete class. Alias chuẩn hóa tên tương thích, allow/deny list áp policy, còn catalog động được đưa thẳng vào system prompt để LLM chỉ nhìn thấy tool có thật ở runtime.

### Mở code

- [`ToolRegistry.cpp`](../../src/tools/ToolRegistry.cpp)

### Điểm cần chỉ

- `register_all_tools()`
- `register_creator()` và fresh-instance factory
- `register_alias()`
- `catalog()`
- `memory_save` và `memory_search`

## Slide 11 — Bộ tool đáp ứng requirement bắt buộc

### Lời nói

> Năm nhóm bắt buộc gồm Shell, File, Web Search, persistent Memory và Calculator. File hỗ trợ read, write và append. Memory có hai entry rõ ràng là `memory_save` và `memory_search`. Ba tool bổ sung là Time, JSON và Git. Mọi failure được trả bằng `ToolError`; framework không biến lỗi thành output thành công giả.

### Demo ngắn

```bash
./build/test_tools
```

### Lời dẫn demo

> Target này kiểm tra Registry/Factory, alias, file artifact, error path, Web Search fixture, Memory lifecycle và Vector ranking offline.

## Slide 12 — Vector Search thay thế tìm kiếm từ khóa

### Lời nói

> Production dùng Ollama model `nomic-embed-text` để chuyển văn bản thành embedding. Text và vector được lưu trong SQLite; query cũng được embedding rồi xếp hạng bằng cosine similarity viết trong C++. Offline test chủ động inject `HashEmbedder`. Production không tự fallback, vì fallback âm thầm sẽ làm kết quả trông hợp lệ nhưng không còn là semantic search thật.

### Mở code

1. [`MemoryTool.cpp`](../../src/tools/MemoryTool.cpp)
2. [`Embedding.cpp`](../../src/tools/Embedding.cpp)

### Live acceptance — chỉ chạy khi Ollama sẵn sàng

```bash
curl http://localhost:11434/api/tags
RUN_LIVE_OLLAMA=1 ./build/test_tools
```

> Nếu Ollama không sẵn sàng, mở [`week10_75_verification_2026-08-20.md`](../../docs/evidence/week10_75_verification_2026-08-20.md) và nói rõ đây là evidence đã lưu, không giả vờ là live run.

### Chuyển người nói

> Tools đã có contract và đường Vector production. Role C sẽ trình bày cách Harness kiểm tra toàn bộ luồng và cách hai worker phối hợp.

---

# ROLE C — HARNESS, MULTI-AGENT VÀ EVIDENCE

## Slide 13 — Harness biến một lần chạy thành evidence

### Lời nói

> Harness chạy theo năm bước: setup, run, evaluate, record và cleanup. Trước batch, nó dọn artifact cũ để task không dùng kết quả còn sót. Bộ benchmark có 10 task gồm 4 simple, 4 medium và 2 hard. Success rate ở đây đo model thật, không phải điểm unit test.

### Mở code

- [`HarnessRunner.cpp`](../../src/harness/HarnessRunner.cpp)
- [`tasks.json`](../../benchmark/tasks.json)

## Slide 14 — Trajectory giải thích vì sao task PASS hoặc FAIL

### Lời nói

> `KeywordEvaluator` kiểm tra nội dung; `FunctionalEvaluator` kiểm tra hậu điều kiện thật như file có tồn tại và đúng nội dung. Trajectory lưu action, tool name, arguments, result, success, latency và token metadata khi provider cung cấp. Final answer được ghi để hoàn chỉnh luồng nhưng không tính nhầm thành tool step.

### Demo 2 — Focused tests và CTest

```bash
./build/test_role_a
./build/test_tools
./build/test_multi_agent
ctest --test-dir build --output-on-failure
```

### Kết quả cần đọc đúng

- `test_role_a`: parser, history, config/error contract, LoopDetector và skill selection PASS.
- `test_tools`: Registry, error paths, Memory và offline Vector ranking PASS.
- `test_multi_agent`: queue, worker lifecycle, cleanup và failure propagation PASS.
- CTest: **5/5 PASS**.

## Slide 15 — Hai worker phối hợp qua message queue

### Lời nói

> `HarnessRunner` gọi `MultiAgentRunner`, rồi runner tạo hai worker thread. Message queue được bảo vệ bằng mutex và condition variable. Calculator tính 47 nhân 23; Researcher tìm thủ đô Nhật Bản. Harness chỉ ghi PASS khi nhận đủ hai kết quả; worker error hoặc timeout vẫn là FAIL. Khi kết thúc, runner gửi stop và join các thread.

### Demo 3 — Multi-agent thật

```bash
./build/demo_multi_agent '47 * 23' 'Japan capital'
cat artifacts/demo/report.txt
```

### Mở code nếu được hỏi

- [`HarnessRunner.cpp`](../../src/harness/HarnessRunner.cpp): `runMultiAgentDemo()`
- [`MultiAgentRunner.cpp`](../../src/multiagent/MultiAgentRunner.cpp)
- [`MessageQueue.h`](../../src/multiagent/MessageQueue.h)

## Slide 16 — Benchmark 7/10

### Lời nói

> Run evidence là `run_20260820_002933_100`, dùng Gemini `gemma-4-31b-it`. Kết quả là final success 7/10, evaluator score 70% và action-level score 90%. Tất cả action có source là LLM; không dùng fixture fallback để ép điểm. Ba task fail phản ánh quyết định của model: task 004 chọn sai tool; task 005 và 009 lặp calculator đến khi LoopDetector dừng.

### Demo 4 — Mở benchmark evidence, không cần chạy live

```bash
cat benchmark/results/run_20260820_002933_100/benchmark_summary.txt
```

Sau đó mở:

1. `benchmark/results/run_20260820_002933_100/trajectory_task_010.json`
2. `benchmark/results/run_20260820_002933_100/trajectory_task_005.json`
3. `benchmark/results/run_20260820_002933_100/eval_results.json`

### Task 010 — PASS

> Agent đọc `data.txt` và nhận NotFound, sau đó tạo file với `initial data`, append `appended` rồi đọc lại. Evaluator và action-level đều PASS.

### Task 005 — FAIL trung thực

> Calculator trả đúng 1081 nên action-level PASS, nhưng model lặp lại calculator thay vì ghi file. LoopDetector dừng ở bước 5, vì vậy evaluator và final result FAIL.

## Slide 17 — Requirement kỹ thuật đã có bằng chứng kiểm thử

### Lời nói

> Nhóm có ba cổng bằng chứng chính: CTest 5/5 PASS, Vector live acceptance PASS và Multi-agent focused test PASS. Bốn design pattern có code và test; bốn Mermaid diagram mô tả component, class và sequence; benchmark thật được giữ nguyên 7/10. Điều này tách rõ phần code đã kiểm chứng với phần hành vi model còn biến động.

### Evidence mapping

- [`requirement_traceability_final_2026-08-20.md`](../../docs/evidence/requirement_traceability_final_2026-08-20.md)
- [`week10_75_verification_2026-08-20.md`](../../docs/evidence/week10_75_verification_2026-08-20.md)
- [`component_diagram.md`](../../docs/diagrams/component_diagram.md)
- [`sequence_harness.md`](../../docs/diagrams/sequence_harness.md)

## Slide 18 — Thank you for listening

### Lời nói

> Nhóm chúng em xin cảm ơn thầy và các bạn đã lắng nghe. Nhóm xin nhận câu hỏi về Agent Core, hệ thống Tool, Vector Search, Harness, Multi-agent hoặc benchmark evidence.

---

# DEMO FLOW RÚT GỌN

## Chuẩn bị trước khi trình bày

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Mở sẵn các tab:

1. `src/agent/agent_loop.cpp`
2. `src/tools/ToolRegistry.cpp`
3. `src/tools/Embedding.cpp`
4. `src/harness/HarnessRunner.cpp`
5. `benchmark/results/run_20260820_002933_100/benchmark_summary.txt`
6. `benchmark/results/run_20260820_002933_100/trajectory_task_010.json`
7. `benchmark/results/run_20260820_002933_100/trajectory_task_005.json`

## Thứ tự demo trong bài

| Sau slide | Demo | Lệnh/chứng cứ | Mục tiêu |
|---:|---|---|---|
| 5 | Build | `cmake --build build -j2` | Chứng minh project build được |
| 11–12 | Tools/Vector | `./build/test_tools` hoặc live Ollama | Chứng minh Registry và semantic path |
| 14 | Regression | `ctest --test-dir build --output-on-failure` | Chứng minh 5/5 code tests PASS |
| 15 | Multi-agent | `./build/demo_multi_agent ...` | Chứng minh hai worker và report |
| 16 | Benchmark | Mở run `run_20260820_002933_100` | Giải thích 7/10 bằng trajectory |

## Phương án dự phòng

- Build đã chạy: chỉ chạy `cmake --build build -j2`, không xóa `build/`.
- Ollama lỗi: mở evidence live acceptance đã lưu.
- Provider thật chậm: không chạy `run_eval`; mở benchmark summary và trajectory.
- Multi-agent live lỗi ngoài dự kiến: mở `artifacts/demo/report.txt` của lần chạy gần nhất và nói rõ đó là artifact đã lưu.

---

# CODE MAPPING KHI Q&A

- ReAct, Template Method, StepHook: [`agent_loop.h`](../../src/agent/agent_loop.h)
- Parser và history: [`agent_loop.cpp`](../../src/agent/agent_loop.cpp)
- Loop detection: [`LoopDetector.cpp`](../../src/agent/LoopDetector.cpp)
- Skill selection: [`SkillLoader.cpp`](../../src/agent/SkillLoader.cpp)
- Registry/Factory/policy: [`ToolRegistry.cpp`](../../src/tools/ToolRegistry.cpp)
- Vector memory: [`MemoryTool.cpp`](../../src/tools/MemoryTool.cpp), [`Embedding.cpp`](../../src/tools/Embedding.cpp)
- Harness/Evaluator/Trajectory: [`HarnessRunner.cpp`](../../src/harness/HarnessRunner.cpp)
- Message queue: [`MessageQueue.h`](../../src/multiagent/MessageQueue.h)
- Multi-agent runner: [`MultiAgentRunner.cpp`](../../src/multiagent/MultiAgentRunner.cpp)

# Q&A NGẮN

## Vì sao dùng `std::expected` thay vì exception cho lỗi Tool?

> Input sai, file không tồn tại và timeout là kết quả có thể dự đoán. `std::expected<T, ToolError>` buộc caller xử lý cả success và failure ngay trong contract. Nhóm không claim exception luôn chậm hoặc luôn không phù hợp.

## Vì sao production không fallback sang `HashEmbedder`?

> Fallback âm thầm có thể trả kết quả trông hợp lệ nhưng không còn là embedding thật. Production trả lỗi rõ ràng; `HashEmbedder` chỉ được inject trong offline test.

## Vì sao benchmark 7/10 vẫn hợp lệ?

> CTest chứng minh code và contract hoạt động. Benchmark đo quyết định của model thật nên có biến động. Đề yêu cầu 10 task và success rate, không yêu cầu model đạt 10/10.

## C++26 nằm ở đâu?

> `MultiAgentRunner` sở hữu worker threads nên không được copy. Deleted function with reason giúp compiler hiển thị nguyên nhân ngay khi có code cố copy object này.

---

# CHECKLIST TRƯỚC KHI TRÌNH BÀY

- [ ] Thay tên/MSSV placeholder trên slide 1 nếu cần.
- [ ] Tập đúng deck 18 slide; không dùng số slide từ bản 15 slide cũ.
- [ ] Build và CTest 5/5 PASS.
- [ ] `./build/test_tools` và `./build/test_multi_agent` PASS.
- [ ] `artifacts/demo/report.txt` tồn tại nếu demo Multi-agent bằng artifact.
- [ ] Mở đúng run `run_20260820_002933_100`.
- [ ] Nói đúng: task 010 PASS; task 005 FAIL do LoopDetector.
- [ ] Không mở `config.json`, API key hoặc token.
- [ ] Không nói “zero warning”, “benchmark 10/10” hoặc claim `std::inplace_vector`.
- [ ] Terminal và code đủ lớn để đọc ở video 1080p.

# NẾU CHẠY BENCHMARK MỚI

```bash
./build/run_eval
```

Chỉ chạy khi provider, quota và mạng sẵn sàng. Nếu có run mới, phải báo đúng run ID và score mới; không thay số bằng kết quả nhớ từ terminal.
