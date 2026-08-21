# DEMO FLOW — C++ AI AGENT FRAMEWORK

> **Mục tiêu:** quay một video hybrid 8–10 phút, xen kẽ slide và ba clip demo ngắn để chứng minh code build/test được, hai bonus Vector và Multi-agent có production path, và benchmark có evidence thật.
>
> **Nguyên tắc:** không hiển thị `config.json` hoặc API key. Không chạy `run_eval` trực tiếp khi quay nếu mạng/quota không ổn định; dùng run evidence đã lưu và nói rõ run ID.

## Timeline dựng video

1. Slide 1–7 — Role A trình bày Core; chèn clip `./build/test_role_a`.
2. Slide 8–12 — Role B trình bày OOP, Tools và Vector; chèn clip live Vector acceptance.
3. Slide 13–16 — Role C trình bày Harness, Multi-agent và benchmark; chèn clip Multi-agent + evidence.
4. Slide 17 — kết luận bằng chuỗi `Requirement → Code → Test → Evidence`.

Các phần terminal bên dưới là nguồn cho ba clip này, không phải một video demo thứ hai.

## 1. Source of truth dùng trong video

- Requirement và traceability: [requirement_traceability_final_2026-08-20.md](../../docs/evidence/requirement_traceability_final_2026-08-20.md)
- Test evidence: [week10_75_verification_2026-08-20.md](../../docs/evidence/week10_75_verification_2026-08-20.md)
- Benchmark task: [tasks.json](../../benchmark/tasks.json)
- Benchmark được chọn: `benchmark/results/run_20260820_002933_100/`
- Kết quả đã xác nhận: **7/10 final PASS**, **70% evaluator score**, **90% action-level score**
- Model của run: Gemini `gemma-4-31b-it`
- Tất cả action trong run được ghi với `source: "llm"`; không dùng fixture fallback để ép điểm.

## 2. Phân chia và thời lượng hybrid

| Phần | Thời gian | Người nói | Nội dung chính |
|---|---:|---|---|
| Slide 1–7 | 00:00–02:40 | Role A | Tổng quan, khái niệm, Agent Core |
| Demo A | 02:40–03:10 | Role A | Focused test Core |
| Slide 8–12 | 03:10–05:20 | Role B | OOP/C++, Tools và Vector |
| Demo B | 05:20–05:55 | Role B | Ollama + live Vector acceptance |
| Slide 13–16 | 05:55–08:00 | Role C | Harness, trajectory, Multi-agent, benchmark |
| Demo C | 08:00–09:10 | Role C | Hai worker, report và benchmark evidence |
| Slide 17 | 09:10–09:40 | Role C | Kết luận và giới hạn |

---

## PHẦN 1 — KIẾN TRÚC TỔNG THỂ

### Màn hình

1. Mở [README.md](../../README.md).
2. Mở [component_diagram.md](../../docs/diagrams/component_diagram.md).
3. Mở [sequence_agent_run.md](../../docs/diagrams/sequence_agent_run.md).

### Lời nói

> “Đây là C++ AI Agent Framework theo vòng lặp ReAct. Entry point benchmark là `run_eval`. Harness đưa task vào AgentLoop; AgentLoop gọi LLM, phân tích tool call, lấy tool từ ToolRegistry, thực thi rồi đưa observation trở lại history. Harness quan sát các bước qua StepHook và chấm bằng Evaluator Strategy.
>
> Role A phụ trách Agent Core và LLM Client. Role B phụ trách Tools, SQLite và Vector Search. Role C phụ trách Harness, evidence và Multi-agent.”

### Không nói

- Không gọi `OopAgent` là giao diện chat. Binary này chỉ là Gemini smoke test và không hỗ trợ `--chat`.
- Không claim GUI hoặc VLM bonus đã hoàn chỉnh.

---

## PHẦN 2 — CONFIGURE VÀ BUILD

### Lệnh chạy trong WSL

```bash
cmake -S . -B build
cmake --build build -j2
```

### Lời nói

> “Project dùng CMake và build ra chín executable, gồm `OopAgent`, `run_eval`, các focused tests và `demo_multi_agent`. Kết quả cần chứng minh ở đây là configure và build hoàn tất, không có compile error.”

### Lưu ý khi quay

- Không xóa `build/` ngay trước lúc quay nếu không cần; build lại toàn bộ sẽ làm video dài.
- Có thể xuất hiện warning từ dependency `nlohmann/json` với compiler mới. Không nói “zero warning”; chỉ kết luận **không có compile error**.

---

## PHẦN 3 — AGENT CORE, OOP VÀ MODERN C++

### Màn hình

1. Mở [agent_loop.h](../../src/agent/agent_loop.h) và [agent_loop.cpp](../../src/agent/agent_loop.cpp).
2. Chỉ `AgentLoop::run()` và các primitive của Template Method.
3. Mở [MultiAgentRunner.h](../../src/multiagent/MultiAgentRunner.h), chỉ hai dòng copy constructor/copy assignment bị xóa có lý do.

### Lời nói

> “`AgentLoop::run()` giữ skeleton ReAct cố định: tạo system prompt, hỏi LLM, phân tích action, gọi tool, lưu assistant response và tool observation vào history, rồi lặp đến final answer hoặc failure gate.
>
> Project chứng minh bốn pattern trên production path: Strategy cho Tool và Evaluator; Template Method cho AgentLoop; Registry/Factory cho ToolRegistry; Observer/Hook để Harness ghi trajectory mà Agent Core không cần biết Harness.
>
> Với C++26, `MultiAgentRunner` dùng deleted function with reason. Đối tượng này sở hữu worker threads nên không được copy. Project không claim `std::inplace_vector`.”

---

## PHẦN 4 — TOOLREGISTRY VÀ VECTOR SEARCH

### Màn hình

1. Mở [ToolRegistry.cpp](../../src/tools/ToolRegistry.cpp):
   - `register_all_tools()`
   - catalog động từ `get_name()` và `get_description()`
   - alias và allow/deny policy
   - hai production entry `memory_save`, `memory_search`
2. Mở [MemoryTool.cpp](../../src/tools/MemoryTool.cpp):
   - constructor/destructor RAII
   - lưu text và embedding vào SQLite
   - vector search
3. Mở [Embedding.cpp](../../src/tools/Embedding.cpp):
   - `OllamaEmbedder::embed()`
   - `cosine_similarity()`

### Lời nói

> “ToolRegistry cho phép Agent Core tìm hoặc tạo tool theo tên mà không phụ thuộc vào concrete class. System prompt cũng lấy catalog động từ registry nên tên tool mà LLM thấy khớp với runtime.
>
> Hai entry `memory_save` và `memory_search` là production path của persistent Vector Memory. Khi lưu, `OllamaEmbedder` gọi model `nomic-embed-text`, sau đó MemoryTool lưu text và vector vào SQLite. Khi tìm, query được embedding và xếp hạng bằng cosine similarity. Keyword search cũ chỉ còn legacy mode.
>
> SQLite handle được quản lý bằng RAII; destructor của MemoryTool gọi `sqlite3_close`.”

### Live Vector acceptance — chỉ chạy khi Ollama đang hoạt động

```bash
curl http://localhost:11434/api/tags
RUN_LIVE_OLLAMA=1 ./build/test_tools
```

Nếu không có Ollama trong lúc quay, mở evidence đã lưu và không giả vờ đây là live run.

---

## PHẦN 5 — FOCUSED TESTS VÀ CTEST

### Lệnh

```bash
./build/test_role_a
./build/test_tools
./build/test_multi_agent
ctest --test-dir build --output-on-failure
```

### Kết quả cần chỉ trên màn hình

- `test_role_a`: AgentLoop, parser, client config/error contract, usage metadata và OOP checks PASS.
- `test_tools`: Registry/Factory, file tools, error paths, Memory và offline Vector ranking PASS.
- `test_multi_agent`: hai worker, queue, cleanup và failure propagation PASS.
- CTest: **5/5 tests PASS**.

### Lời nói

> “Focused tests kiểm tra contract cụ thể của từng role. CTest gom năm target: harness, multi_agent, tools, template_method và role_a. Đây là test của code và integration cục bộ; nó khác với success rate của model thật trong benchmark.”

Không đọc một con số “bao nhiêu unit test” nếu output hiện tại không in tổng số đó.

---

## PHẦN 6 — MULTI-AGENT BONUS

### Lệnh

```bash
./build/demo_multi_agent '47 * 23' 'Japan capital'
cat artifacts/demo/report.txt
```

### Màn hình code

- [HarnessRunner.cpp](../../src/harness/HarnessRunner.cpp): `runMultiAgentDemo()`
- [MultiAgentRunner.cpp](../../src/multiagent/MultiAgentRunner.cpp)
- [test_multi_agent.cpp](../../benchmark/test_multi_agent.cpp)

### Lời nói

> “Multi-agent không còn là class đứng riêng. Production demo đi từ `HarnessRunner::runMultiAgentDemo()` sang `MultiAgentRunner`. Harness gửi hai subtask vào hai worker thread qua message queue: Calculator tính 47 nhân 23; Researcher tìm thủ đô Nhật Bản.
>
> Runner dùng mutex và condition variable để bảo vệ queue. Khi kết thúc, nó gửi stop và join worker. Report chỉ ghi `STATUS=PASS` khi nhận đủ hai kết quả; worker error hoặc timeout được giữ là FAIL, không bị thay bằng câu trả lời giả.”

---

## PHẦN 7 — BENCHMARK THẬT VÀ TRAJECTORY

### Không nên chạy live khi quay

`./build/run_eval` gọi provider thật, có thể mất nhiều phút, gặp timeout/503 và tiêu thụ quota. Demo ổn định nên mở evidence đã được duyệt:

```bash
cat benchmark/results/run_20260820_002933_100/benchmark_summary.txt
```

Sau đó mở:

- `benchmark/results/run_20260820_002933_100/trajectory_task_010.json`
- `benchmark/results/run_20260820_002933_100/trajectory_task_005.json`
- `benchmark/results/run_20260820_002933_100/eval_results.json`

### Task 010 — ví dụ PASS

> “Task 010 yêu cầu đọc file chưa tồn tại, tạo file với `initial data`, append `appended`, rồi kiểm tra kết quả. Trajectory cho thấy `read_file` trả NotFound, Agent chuyển sang `write_file`, sau đó `append_file` và đọc lại. Evaluator và action-level đều PASS.”

### Task 005 — ví dụ FAIL trung thực

> “Task 005 gọi đúng calculator và nhận 1081, nên action-level PASS. Tuy nhiên model lặp lại cùng calculator call thay vì chuyển sang ghi file. LoopDetector dừng vòng lặp ở bước 5, vì vậy evaluator FAIL và final result FAIL. Đây là giới hạn của model trong run này, không phải test code bị che giấu.”

### Kết quả phải đọc đúng

> “Run `run_20260820_002933_100` dùng Gemini `gemma-4-31b-it`, đạt 7 trên 10 task, evaluator score 70% và action-level score 90%. Đề yêu cầu báo cáo success rate, không yêu cầu cố định 10 trên 10.”

### Token usage

> “Code hiện tại có đọc Gemini `usageMetadata` và Ollama `prompt_eval_count`/`eval_count`, rồi export `tokens_used` và `total_tokens`. Giá trị 0 nghĩa là provider không trả metadata trong response đó; không có nghĩa là request không dùng token.”

---

## PHẦN 8 — KẾT LUẬN

### Lời nói

> “Nhóm đã chứng minh build thành công, CTest 5/5 PASS, Vector live acceptance có evidence, Multi-agent có đường tích hợp từ Harness và benchmark thật đạt 7/10. Kết quả benchmark phụ thuộc hành vi model và dịch vụ mạng, nên nhóm báo cáo đúng run thay vì sửa task hoặc dùng fallback để ép 10/10.
>
> Phạm vi bonus được chốt là Vector và Multi-agent. GUI/VLM không được claim hoàn chỉnh.”

---

## 3. Checklist trước khi quay

- [ ] Quay từ repository root trong WSL.
- [ ] Build hiện tại đã hoàn tất.
- [ ] `test_role_a`, `test_tools`, `test_multi_agent` và CTest 5/5 PASS.
- [ ] Ollama và `nomic-embed-text` hoạt động nếu chọn quay live Vector acceptance.
- [ ] `artifacts/demo/report.txt` được tạo bởi lần chạy Multi-agent vừa quay.
- [ ] Mở đúng run `run_20260820_002933_100`.
- [ ] Nói rõ Task 010 PASS và Task 005 FAIL do LoopDetector; không nói Task 005 PASS.
- [ ] Không hiển thị `config.json`, API key, token hoặc local secret.
- [ ] Không nói “zero warning”, “benchmark 10/10”, “OopAgent chat”, “Multi-agent chưa tích hợp Harness” hoặc “token usage chưa hỗ trợ”.
- [ ] Video tối thiểu 1080p; terminal và code đủ lớn để đọc.

## 4. Nếu muốn chạy benchmark mới

Chỉ chạy khi provider, quota và mạng đã sẵn sàng:

```bash
./build/run_eval
```

Run mới phải được báo cáo bằng chính run ID và score mới của nó. Không thay số của run cũ bằng kết quả nhớ từ terminal.
