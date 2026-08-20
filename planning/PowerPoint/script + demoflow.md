# KỊCH BẢN THUYẾT TRÌNH + DEMO ĐAN XEN — AI Agent Framework bằng Modern C++

> **Mục tiêu:** một file duy nhất, vừa thuyết trình vừa demo (dẫn chứng) ngay tại chỗ — không trình bày xong hết rồi mới demo.
>
> **Thời lượng mục tiêu:** 15–18 phút (đã gộp phần nói + demo, không tách riêng khối demo 2 phút ở cuối).
>
> **Nguyên tắc chung:**
> - Với mỗi phần: **(1) Slide** → **(2) Lời nói** → **(3) Demo / Dẫn chứng ngay** (mở code, chạy lệnh, mở evidence).
> - Slide chỉ hiện từ khóa, sơ đồ, evidence; không đọc toàn bộ chữ trên slide.
> - Không hiển thị `config.json`, API key, token hoặc local secret.
> - Không chạy `run_eval` trực tiếp khi quay nếu mạng/quota không ổn định; mở run evidence đã lưu và nói rõ run ID.
> - Không nói “zero warning”, “benchmark 10/10”, “OopAgent chat”, “Multi-agent chưa tích hợp Harness”, “token usage chưa hỗ trợ”, hoặc claim `std::inplace_vector`.

## Phân vai và thời gian (tổng thể)

| Phần | Người nói | Slide | Demo đi kèm | Thời gian |
|---|---|---|---|---:|
| 1. Giới thiệu + Kiến trúc | Role A | 1–3 | README, component/sequence diagram | 3 phút |
| 2. Configure & Build | Role B | — | CMake configure + build | 1.5 phút |
| 3. Agent Core, OOP, C++26 | Role A | 4–5, 7 | agent_loop.h/.cpp, MultiAgentRunner.h | 4 phút |
| 4. ToolRegistry & Vector Search | Role B | 6, 8–10 | ToolRegistry.cpp, MemoryTool.cpp, Embedding.cpp (+ live) | 4 phút |
| 5. Tests & CTest | Role C | 11–12 | test_role_a/test_tools/test_multi_agent, CTest | 3 phút |
| 6. Multi-agent | Role C | 13 | demo_multi_agent + report | 2 phút |
| 7. Benchmark & Trajectory | Role C | 14, 12 | benchmark_summary, trajectory 010/005 | 3 phút |
| 8. Tổng kết & giới hạn | Role C | 15 | — | 1.5 phút |

---

# PHẦN 1 — GIỚI THIỆU VÀ KIẾN TRÚC TỔNG QUAN

## Slide 1 — Đề tài và thành viên

### Nội dung trên slide
- **AI Agent Framework with Ollama & Gemini APIs**
- Modern C++17/20/23/26
- A: Systems/Core · B: Tools/Data · C: Eval/Infra

### Lời nói
> Kính chào thầy cô và các bạn. Nhóm chúng em xin trình bày đồ án AI Agent Framework được xây dựng bằng Modern C++. Hệ thống có thể kết nối mô hình cục bộ qua Ollama hoặc mô hình đám mây qua Gemini, cho phép LLM gọi công cụ, lưu memory và tự đánh giá bằng benchmark. Nhóm chia công việc thành ba phần: Role A phụ trách Agent Core và LLM Client; Role B phụ trách Tools, SQLite và Vector Search; Role C phụ trách Harness, Multi-agent và evidence kiểm thử.

## Slide 2 — Bài toán và mục tiêu

### Nội dung trên slide
- LLM chỉ sinh văn bản, không tự thao tác môi trường
- ReAct: **Think → Act → Observe → Continue/Finish**
- Mục tiêu: mở rộng được, lỗi rõ ràng, đánh giá tái lập được

### Lời nói
> Một LLM thông thường có thể trả lời bằng văn bản nhưng không tự đọc file, tìm kiếm web hay tính toán bằng chương trình. Agent giải quyết giới hạn này bằng vòng lặp ReAct: mô hình chọn hành động, framework chạy tool, trả observation về cho mô hình, rồi tiếp tục cho đến khi có final answer. Mục tiêu của nhóm không chỉ là gọi API mà còn phải thiết kế đúng OOP: mỗi tầng có trách nhiệm riêng, công cụ có thể mở rộng và mọi lần chạy benchmark đều có trajectory để kiểm tra lại.

## Slide 3 — Kiến trúc tổng quan

### Nội dung trên slide
- `LLMClient` → `AgentLoop` → `ToolRegistry` → concrete tools
- `SkillLoader` cung cấp instruction theo task
- `HarnessRunner` quan sát Agent qua `StepHook`
- Agent không phụ thuộc ngược vào Harness

### Lời nói
> Kiến trúc được chia thành các thành phần rõ ràng. `LLMClient` che giấu khác biệt giữa Ollama và Gemini. `AgentLoop` điều khiển ReAct nhưng chỉ biết interface của tool. `ToolRegistry` quản lý tên, mô tả, factory và policy. `SkillLoader` chọn hướng dẫn phù hợp dựa trên từ khóa của task. Ở tầng ngoài, `HarnessRunner` nhận từng `TrajectoryStep` qua callback `StepHook`. Vì Agent chỉ phát sự kiện mà không biết ai đang nghe, phần Core không phụ thuộc vào Harness và có thể kiểm thử độc lập.

## Demo ngay — minh họa kiến trúc trên màn hình

### Màn hình
1. Mở [README.md](../../README.md).
2. Mở [component_diagram.md](../../docs/diagrams/component_diagram.md).
3. Mở [sequence_agent_run.md](../../docs/diagrams/sequence_agent_run.md).

### Lời dẫn chứng (nói trong lúc chỉ vào sơ đồ)
> Đây là C++ AI Agent Framework theo vòng lặp ReAct. Entry point benchmark là `run_eval`. Harness đưa task vào AgentLoop; AgentLoop gọi LLM, phân tích tool call, lấy tool từ ToolRegistry, thực thi rồi đưa observation trở lại history. Harness quan sát các bước qua StepHook và chấm bằng Evaluator Strategy. Như thầy cô thấy ở component diagram, sự kết nối là một chiều: Harness → Agent Core → Tools → Environment.

### Không nói
- Không gọi `OopAgent` là giao diện chat. Binary này chỉ là Gemini smoke test và không hỗ trợ `--chat`.
- Không claim GUI hoặc VLM bonus đã hoàn chỉnh.

---

# PHẦN 2 — CONFIGURE VÀ BUILD (DEMO TRỰC TIẾP)

### Lời nói (Role B)
> Sau đây em xin chứng minh project build được thật. Dự án dùng CMake và build ra chín executable, gồm `OopAgent`, `run_eval`, các focused tests và `demo_multi_agent`. Kết quả cần chứng minh ở đây là configure và build hoàn tất, không có compile error.

## Demo ngay — lệnh chạy trong WSL

```bash
cmake -S . -B build
cmake --build build -j2
```

### Lời dẫn chứng
> Như thầy cô thấy, `cmake -S . -B build` cấu hình xong và `cmake --build build -j2` biên dịch toàn bộ source C++ mà không sinh compile error, xuất ra các file thực thi trong `build/`.

### Lưu ý khi quay
- Không xóa `build/` ngay trước lúc quay nếu không cần; build lại toàn bộ sẽ làm video dài.
- Có thể xuất hiện warning từ dependency `nlohmann/json` với compiler mới. Không nói “zero warning”; chỉ kết luận **không có compile error**.

---

# PHẦN 3 — AGENT CORE, OOP, MODERN C++ VÀ C++26

## Slide 4 — LLM Client và Agent Loop

### Nội dung trên slide
- Một interface cho text và ảnh base64
- Config: endpoint, model, temperature, max tokens, timeout
- Parser hỗ trợ JSON/fenced JSON và escaped arguments
- Lỗi được phân loại, không xem JSON hỏng là final answer

### Lời nói
> `LLMClient` cung cấp một interface chung cho cả text và multimodal message. Cấu hình provider được đọc từ file thay vì gắn cứng trong business logic. Trong mỗi bước, `AgentLoop` gửi toàn bộ conversation history, nhận phản hồi rồi parse tool call. Parser dùng quét object cân bằng kết hợp `nlohmann::json`, nên giữ được các chuỗi argument có escape. Nếu phản hồi có ý định gọi tool nhưng JSON bị hỏng, hệ thống trả lỗi parse rõ ràng và không thực thi một tool call bị cắt dở.

## Slide 5 — Loop Detector và Skill System

### Nội dung trên slide
- Generic repeat và ping-pong
- Warning/critical thresholds, critical thì dừng
- Ba skill Markdown có keyword metadata
- Skill phù hợp được inject trước mỗi run

### Lời nói
> Agent có thể lặp lại cùng một action hoặc đổi qua lại giữa hai action. `LoopDetector` lưu chuỗi action gần đây để nhận biết generic repeat và ping-pong. Khi vượt ngưỡng critical, Agent dừng an toàn thay vì tiếp tục tốn API. Song song đó, `SkillLoader` quét thư mục `skills`, đọc keyword metadata và chỉ inject skill phù hợp với task; nếu không có keyword khớp thì dùng `task_planner` mặc định.

## Slide 7 — Modern C++17 đến C++26

### Nội dung trên slide
- C++17: smart pointers, filesystem, variant, optional, function
- C++20: ranges và views
- C++23: `std::expected`, `std::println`
- C++26: deleted function with reason

```cpp
MultiAgentRunner(const MultiAgentRunner&)
  = delete("owns worker threads and is non-copyable");
```

### Lời nói
> Project sử dụng smart pointer và RAII để quản lý lifetime, `filesystem` để thao tác đường dẫn, `variant` cho action và `function` cho callback. C++20 được dùng qua ranges và views. C++23 dùng `std::expected` để trả về hoặc kết quả hoặc lỗi một cách tường minh, cùng `std::println` cho formatted output. Với C++26, nhóm dùng deleted function with reason để giải thích ngay tại compile time vì sao `MultiAgentRunner`, một đối tượng sở hữu worker thread, không được phép copy. `std::inplace_vector` không được claim vì standard library hiện tại chưa hỗ trợ đầy đủ.

## Demo ngay — duyệt code Core

### Màn hình
1. Mở [agent_loop.h](../../src/agent/agent_loop.h) và [agent_loop.cpp](../../src/agent/agent_loop.cpp).
2. Chỉ `AgentLoop::run()` và các primitive của Template Method.
3. Mở [MultiAgentRunner.h](../../src/multiagent/MultiAgentRunner.h), chỉ hai dòng copy constructor/copy assignment bị xóa có lý do.

### Lời dẫn chứng
> `AgentLoop::run()` giữ skeleton ReAct cố định: tạo system prompt, hỏi LLM, phân tích action, gọi tool, lưu assistant response và tool observation vào history, rồi lặp đến final answer hoặc failure gate.
>
> Như thầy cô thấy ở `MultiAgentRunner.h`, hai dòng `= delete(...)` với lý do rõ ràng: đối tượng này sở hữu worker threads nên không được copy. Đây là áp dụng trực tiếp tính năng deleted function with reason của C++26.

---

# PHẦN 4 — TOOLREGISTRY, BỘ CÔNG CỤ VÀ VECTOR SEARCH

## Slide 6 — Bốn Design Patterns bắt buộc

### Nội dung trên slide

| Pattern | Vị trí áp dụng |
|---|---|
| Strategy | `Evaluator` và `Tool` interfaces |
| Template Method | `AgentLoop::run()` |
| Registry/Factory | `ToolRegistry` |
| Observer/Hook | `StepHook` |

### Lời nói
> Hệ thống áp dụng đủ bốn pattern bắt buộc. Strategy cho phép nhiều Evaluator và Tool dùng chung interface. Template Method nằm tại `AgentLoop::run`: skeleton của vòng lặp được giữ cố định, còn các primitive có thể override trong test. Registry/Factory tạo hoặc tra cứu tool theo tên mà Core không cần biết concrete class. Cuối cùng, Observer được hiện thực bằng `StepHook`, giúp Harness ghi trajectory mà không tạo dependency ngược vào Agent.

## Slide 8 — Tool Registry và Factory

### Nội dung trên slide
- Tool contract: name, description, `execute(arguments)`
- Runtime registration và fresh-instance factory
- Alias normalization
- Allow/deny policy
- Catalog động được đưa vào system prompt

### Lời nói
> Mỗi tool triển khai cùng một contract gồm tên, mô tả và hàm execute. `ToolRegistry` vừa giữ shared instance để lookup, vừa giữ creator lambda để tạo instance mới. Alias giúp chuyển tên tương thích về canonical name; allow/deny list chặn tool theo policy. Quan trọng hơn, Agent tạo phần Available Tools trực tiếp từ catalog của Registry. Vì vậy tên và mô tả mà LLM nhìn thấy luôn khớp với các tool thực sự đang đăng ký.

## Slide 9 — Bộ công cụ

### Nội dung trên slide
- Bắt buộc: shell, file, web search, memory, calculator
- Memory có entry rõ ràng: `memory_save`, `memory_search`
- Bổ sung: time, JSON, Git
- Tool failure trả `ToolError`, không dựng kết quả thành công giả

### Lời nói
> Project đáp ứng năm nhóm tool bắt buộc: chạy shell; đọc, ghi và append file; web search; persistent memory; và calculator. Hai tên production `memory_save` và `memory_search` giúp LLM gọi memory trực tiếp. Ba tool bổ sung là time, JSON và Git. Các tool kiểm tra input và trả `ToolError` cho trường hợp không hợp lệ, timeout hoặc dịch vụ ngoài không sẵn sàng. Cách này cho phép Agent và Harness phân biệt lỗi thật với output hợp lệ.

## Slide 10 — Persistent Memory và Vector Search (+4)

### Nội dung trên slide
- SQLite lưu text và embedding
- Production: Ollama `nomic-embed-text`
- Cosine similarity viết bằng C++
- `save/search` là vector path chính
- Offline test inject `HashEmbedder`; production không tự fallback

### Lời nói
> Bonus Vector Search thay keyword search chính bằng embedding similarity. Khi lưu memory, production gửi text tới `nomic-embed-text` qua Ollama rồi lưu vector cùng dữ liệu trong SQLite. Khi tìm kiếm, query cũng được embedding và so sánh với các vector đã lưu bằng cosine similarity viết trong C++. Offline test chủ động inject `HashEmbedder` để có kết quả xác định và không cần mạng; production vẫn dùng Ollama và trả lỗi rõ ràng nếu embedding service không hoạt động, không tự âm thầm đổi thuật toán.

## Demo ngay — duyệt code Tools & Vector

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

### Lời dẫn chứng
> ToolRegistry cho phép Agent Core tìm hoặc tạo tool theo tên mà không phụ thuộc vào concrete class. System prompt cũng lấy catalog động từ registry nên tên tool mà LLM thấy khớp với runtime.
>
> Hai entry `memory_save` và `memory_search` là production path của persistent Vector Memory. Khi lưu, `OllamaEmbedder` gọi model `nomic-embed-text`, sau đó MemoryTool lưu text và vector vào SQLite. Khi tìm, query được embedding và xếp hạng bằng cosine similarity. SQLite handle được quản lý bằng RAII; destructor của MemoryTool gọi `sqlite3_close`.

### Live Vector acceptance — chỉ chạy khi Ollama đang hoạt động

```bash
curl http://localhost:11434/api/tags
RUN_LIVE_OLLAMA=1 ./build/test_tools
```

Nếu không có Ollama trong lúc quay, mở evidence đã lưu và không giả vờ đây là live run.

---

# PHẦN 5 — FOCUSED TESTS, CTest VÀ HARNESS

## Slide 11 — Benchmark Harness

### Nội dung trên slide
- `setup → run → evaluate → record → cleanup`
- 10 task: **4 simple / 4 medium / 2 hard**
- Dọn artifact cũ trước mỗi batch
- Success rate là phép đo model, không phải unit-test score

### Lời nói
> `HarnessRunner` tự động hóa toàn bộ quá trình đánh giá. Trước mỗi batch, Harness dọn các artifact được khai báo để task không tận dụng kết quả cũ. Sau đó Agent chạy task, Evaluator kiểm tra output và Harness xuất evidence. Bộ benchmark có đúng 10 task theo yêu cầu: 4 đơn giản, 4 trung bình và 2 khó. Khác với CTest dùng để kiểm tra code, success rate của benchmark đo hành vi của model thật nên có thể thay đổi giữa các lần chạy.

## Slide 12 — Evaluator và Trajectory

### Nội dung trên slide
- `KeywordEvaluator` và `FunctionalEvaluator`
- Evaluator Strategy được chọn theo task
- Trajectory: action, args, result, success, latency, token usage
- Final answer được ghi nhưng không tính là tool step

### Lời nói
> `KeywordEvaluator` kiểm tra nội dung đầu ra, còn `FunctionalEvaluator` kiểm tra hậu điều kiện thật như file có tồn tại và đúng nội dung hay không. Mỗi bước chạy được lưu trong trajectory JSON, gồm tool, arguments, kết quả, trạng thái, latency và token metadata khi provider cung cấp. Final answer cũng được ghi để có luồng hoàn chỉnh, nhưng không bị tính nhầm là một lần gọi tool. Nhờ vậy, nhóm có thể xem chính xác task thất bại ở quyết định của model, tool hay evaluator.

## Demo ngay — chạy tests và CTest

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

### Lời dẫn chứng
> Focused tests kiểm tra contract cụ thể của từng role. CTest gom năm target: harness, multi_agent, tools, template_method và role_a. Đây là test của code và integration cục bộ; nó khác với success rate của model thật trong benchmark.

Không đọc một con số “bao nhiêu unit test” nếu output hiện tại không in tổng số đó.

---

# PHẦN 6 — MULTI-AGENT BONUS

## Slide 13 — Multi-agent Coordination (+3)

### Nội dung trên slide
- `HarnessRunner → MultiAgentRunner → 2 worker threads`
- `std::queue + mutex + condition_variable`
- Stop/join bằng RAII
- Demo thật: Calculator `47 × 23` và Researcher tìm thủ đô Nhật Bản

### Lời nói
> Bonus Multi-agent có đường tích hợp thật từ Harness sang `MultiAgentRunner`. Runner tạo hai worker thread và giao tiếp qua message queue được bảo vệ bởi mutex và condition variable. Khi kết thúc, runner gửi tín hiệu dừng và join toàn bộ thread để không để lại worker chạy nền. Demo hiện tại chia hai subtask độc lập: Calculator tính 47 nhân 23, còn Researcher tìm thủ đô Nhật Bản. Harness nhận hai kết quả và tạo report; test cũng kiểm tra rằng lỗi của worker không bị biến thành PASS.

## Demo ngay — chạy multi-agent

### Lệnh

```bash
./build/demo_multi_agent '47 * 23' 'Japan capital'
cat artifacts/demo/report.txt
```

### Màn hình code
- [HarnessRunner.cpp](../../src/harness/HarnessRunner.cpp): `runMultiAgentDemo()`
- [MultiAgentRunner.cpp](../../src/multiagent/MultiAgentRunner.cpp)
- [test_multi_agent.cpp](../../benchmark/test_multi_agent.cpp)

### Lời dẫn chứng
> Multi-agent không còn là class đứng riêng. Production demo đi từ `HarnessRunner::runMultiAgentDemo()` sang `MultiAgentRunner`. Harness gửi hai subtask vào hai worker thread qua message queue: Calculator tính 47 nhân 23; Researcher tìm thủ đô Nhật Bản.
>
> Runner dùng mutex và condition variable để bảo vệ queue. Khi kết thúc, nó gửi stop và join worker. Report chỉ ghi `STATUS=PASS` khi nhận đủ hai kết quả; worker error hoặc timeout được giữ là FAIL, không bị thay bằng câu trả lời giả.

---

# PHẦN 7 — BENCHMARK THẬT VÀ TRAJECTORY

## Slide 14 — Kết quả thực nghiệm

### Nội dung trên slide
- Run: `run_20260820_002933_100`
- Provider/model: Gemini `gemma-4-31b-it`
- Final success: **7/10 = 70%**
- Evaluator: **70%** · Action-level: **90%**
- Toàn bộ recorded action có source `llm`, không có fixture fallback

### Lời nói
> Run thật được chọn làm evidence là `run_20260820_002933_100`, sử dụng Gemini với model `gemma-4-31b-it`. Kết quả cuối là 7 trên 10 task, evaluator score 70% và action-level score 90%. Tất cả action được ghi với source là LLM, không dùng deterministic fixture để ép điểm. Ba task không pass cho thấy giới hạn của model: task 4 chọn sai tool; task 5 và 9 lặp lại calculator cho đến khi LoopDetector dừng. Đề yêu cầu báo cáo success rate, không yêu cầu model phải đạt 10/10.

## Demo ngay — mở evidence và trajectory

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
> Task 010 yêu cầu đọc file chưa tồn tại, tạo file với `initial data`, append `appended`, rồi kiểm tra kết quả. Trajectory cho thấy `read_file` trả NotFound, Agent chuyển sang `write_file`, sau đó `append_file` và đọc lại. Evaluator và action-level đều PASS.

### Task 005 — ví dụ FAIL trung thực
> Task 005 gọi đúng calculator và nhận 1081, nên action-level PASS. Tuy nhiên model lặp lại cùng calculator call thay vì chuyển sang ghi file. LoopDetector dừng vòng lặp ở bước 5, vì vậy evaluator FAIL và final result FAIL. Đây là giới hạn của model trong run này, không phải test code bị che giấu.

### Kết quả phải đọc đúng
> Run `run_20260820_002933_100` dùng Gemini `gemma-4-31b-it`, đạt 7 trên 10 task, evaluator score 70% và action-level score 90%. Đề yêu cầu báo cáo success rate, không yêu cầu cố định 10 trên 10.

### Token usage
> Code hiện tại có đọc Gemini `usageMetadata` và Ollama `prompt_eval_count`/`eval_count`, rồi export `tokens_used` và `total_tokens`. Giá trị 0 nghĩa là provider không trả metadata trong response đó; không có nghĩa là request không dùng token.

---

# PHẦN 8 — TỔNG KẾT VÀ GIỚI HẠN

## Slide 15 — Tổng kết, giới hạn

### Nội dung trên slide
- Technical gates: clean build, CTest **5/5 PASS**
- Vector live acceptance PASS
- Multi-agent focused test PASS
- 4 design patterns và 4 Mermaid diagrams
- Giới hạn: model không deterministic; GUI/VLM không được claim

### Lời nói
> Tóm lại, candidate kỹ thuật đã clean build và vượt qua 5 trên 5 CTest. Hai bonus nhóm chọn là Vector Search và Multi-agent đều có production path, focused test và tài liệu tương ứng. Hệ thống đáp ứng bốn design pattern và bốn Mermaid diagram bắt buộc. Nhóm không claim GUI/VLM là feature hoàn chỉnh và giữ nguyên kết quả benchmark 7 trên 10 để phản ánh model thật. Nhóm đã chứng minh build thành công, CTest 5/5 PASS, Vector live acceptance có evidence, Multi-agent có đường tích hợp từ Harness và benchmark thật đạt 7/10. Kết quả benchmark phụ thuộc hành vi model và dịch vụ mạng, nên nhóm báo cáo đúng run thay vì sửa task hoặc dùng fallback để ép 10/10.

---

# CODE MAPPING KHI Q&A

- Template Method và StepHook: [`agent_loop.h`](../../src/agent/agent_loop.h)
- Parser và ReAct implementation: [`agent_loop.cpp`](../../src/agent/agent_loop.cpp)
- Registry/Factory: [`ToolRegistry.cpp`](../../src/tools/ToolRegistry.cpp)
- Vector memory: [`MemoryTool.cpp`](../../src/tools/MemoryTool.cpp)
- Message queue: [`MessageQueue.h`](../../src/multiagent/MessageQueue.h)
- Harness/Multi-agent integration: [`HarnessRunner.cpp`](../../src/harness/HarnessRunner.cpp)
- Final evidence: [`requirement_traceability_final_2026-08-20.md`](../../docs/evidence/requirement_traceability_final_2026-08-20.md)

# Q&A THƯỜNG GẶP VÀ CÁCH TRẢ LỜI GHI ĐIỂM

## Câu 1: Tại sao không dùng exceptions mà lại dùng `std::expected` cho lỗi Tool?

> Các lỗi như input không hợp lệ, file không tồn tại hoặc timeout là kết quả có thể dự đoán của một lần gọi tool. `std::expected<T, ToolError>` thể hiện ngay trong kiểu trả về rằng caller phải xử lý cả success và failure, nên contract rõ ràng và dễ kiểm thử. Exception vẫn phù hợp cho lỗi bất thường; nhóm không claim exceptions luôn chậm hoặc luôn gây crash.

## Câu 2: Thuật toán Cosine Similarity hoạt động như thế nào?

> Cosine Similarity tính cosin của góc giữa hai vector bằng tích vô hướng chia cho tích độ dài của chúng. Kết quả nằm trong khoảng từ -1 đến 1; giá trị càng gần 1 thì hai vector càng cùng hướng và nội dung thường càng tương đồng về ngữ nghĩa. Project dùng phép tính này để xếp hạng các memory đã lưu theo độ gần với query.

## Câu 3: Bộ nhớ SQLite được quản lý vòng đời như thế nào để tránh leak?

> `MemoryTool` áp dụng RAII. Constructor khởi tạo kết nối SQLite và object giữ quyền sở hữu handle. Khi object ra khỏi scope hoặc `unique_ptr` bị giải phóng, destructor gọi `sqlite3_close`, nên handle được đóng kể cả khi luồng chạy kết thúc sớm. `test_memory_lifecycle` kiểm tra cả database hợp lệ và đường dẫn database lỗi để bảo đảm không crash và cleanup an toàn.

## Câu 4: Tại sao production không tự fallback sang `HashEmbedder`?

> Nếu tự fallback, hệ thống có thể trả kết quả tưởng như hợp lệ nhưng không còn là embedding thật từ `nomic-embed-text`. Production trả lỗi rõ ràng; `HashEmbedder` chỉ được inject trong offline test.

## Câu 5: Tại sao benchmark chỉ đạt 7/10 nhưng vẫn hợp lệ?

> CTest chứng minh code và contract hoạt động. Benchmark đo quyết định của model thật nên có tính biến động. Đề yêu cầu tập 10 task và báo cáo success rate, không yêu cầu 10/10.

## Câu 6: C++26 được dùng ở đâu?

> `MultiAgentRunner` sở hữu worker threads nên không được copy. Project dùng deleted function with reason để compiler hiển thị lý do này ngay khi có code cố copy đối tượng.

---

# CHECKLIST TRƯỚC KHI QUAY

- [ ] Thay “Role A/B/C” bằng tên và MSSV thật trên slide đầu.
- [ ] Tập thử với đồng hồ; toàn bộ luồng (nói + demo) không vượt 18 phút.
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
- [ ] Video được upload YouTube ở chế độ **Unlisted** và mở được khi đăng xuất.

---

# NẾU MUỐN CHẠY BENCHMARK MỚI

Chỉ chạy khi provider, quota và mạng đã sẵn sàng:

```bash
./build/run_eval
```

Run mới phải được báo cáo bằng chính run ID và score mới của nó. Không thay số của run cũ bằng kết quả nhớ từ terminal.
