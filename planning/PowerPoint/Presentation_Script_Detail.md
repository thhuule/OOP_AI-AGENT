# Kịch bản thuyết trình chi tiết — AI Agent Framework bằng Modern C++

> **Thời lượng video mục tiêu:** 8–10 phút<br>
> **Hình thức:** slide thuyết trình xen ba đoạn demo đã quay và cắt gọn<br>
> **Demo:** đặt ngay sau phần của Role A, Role B và Role C; không dồn thành video thứ hai<br>
> **Evidence dùng trong bài:** `CTest 5/5 PASS`, benchmark Gemini `7/10`, Vector và Multi-agent focused tests PASS.

## Phân vai và thời gian

| Phần | Người nói | Slide | Thời gian |
|---|---|---:|---:|
| Mở đầu, khái niệm và Agent Core | Role A | 1–7 | 3 phút |
| OOP, Modern C++ và Tools | Role B | 8–12 | 2.5 phút |
| Harness, bonus và kết quả | Role C | 13–17 | 2.5 phút |
| Ba đoạn demo đã cắt gọn | A / B / C | Xen giữa các phần | 1.5–2 phút |

Không đọc toàn bộ chữ trên slide. Slide chỉ hiển thị từ khóa, sơ đồ và evidence; phần bên dưới là lời nói.

---

# Role A — Kiến trúc và Agent Core

## Slide 1 — Đề tài và thành viên

### Nội dung trên slide

- **AI Agent Framework with Ollama & Gemini APIs**
- Modern C++17/20/23/26
- Logo trường/khoa và giảng viên
- A: Systems/Core · B: Tools/Data · C: Eval/Infra

### Lời nói

> Kính chào thầy cô và các bạn. Nhóm chúng em xin trình bày đồ án AI Agent Framework được xây dựng bằng Modern C++. Hệ thống có thể kết nối mô hình cục bộ qua Ollama hoặc mô hình đám mây qua Gemini, cho phép LLM gọi công cụ, lưu memory và tự đánh giá bằng benchmark. Nhóm chia công việc thành ba phần: Role A phụ trách Agent Core và LLM Client; Role B phụ trách Tools, SQLite và Vector Search; Role C phụ trách Harness, Multi-agent và evidence kiểm thử.

## Slide 2 — Bản đồ bài trình bày

### Nội dung trên slide

- Agent Core
- OOP và Modern C++
- Tools và Vector
- Harness và Multi-agent
- Test và Benchmark evidence

### Lời nói

> Bài trình bày đi theo một đường duy nhất từ kiến trúc đến bằng chứng. Đầu tiên là cách Agent suy luận và kiểm soát lỗi. Tiếp theo là thiết kế OOP, Modern C++ và hệ thống Tools. Sau đó nhóm trình bày Vector Memory, Harness và Multi-agent. Cuối cùng, mọi claim được đối chiếu bằng focused tests, CTest và benchmark thật.

### Transition

> Trước khi đi vào code, chúng ta cần thống nhất sáu khái niệm sẽ xuất hiện xuyên suốt bài.

## Slide 3 — Sáu khái niệm nền

### Nội dung trên slide

- LLM, Tool, AI Agent và ReAct
- Harness và Trajectory

### Lời nói

> LLM là mô hình sinh quyết định hoặc câu trả lời. Tool là một chức năng có input, output và lỗi rõ ràng. Khi kết hợp LLM với vòng điều khiển, trạng thái và các tool, ta có AI Agent. ReAct là chu trình Think, Act và Observe giúp Agent quyết định bước tiếp theo. Harness là bộ chạy task và chấm kết quả; Trajectory là nhật ký từng bước được dùng làm evidence.

### Transition

> Sau khi đã có các khái niệm nền, trước hết hãy xem Agent khác chatbot thông thường ở điểm nào.

## Slide 4 — Bài toán và mục tiêu

### Nội dung trên slide

- LLM chỉ sinh văn bản, không tự thao tác môi trường
- ReAct: **Think → Act → Observe → Continue/Finish**
- Mục tiêu: mở rộng được, lỗi rõ ràng, đánh giá tái lập được

### Lời nói

> Một LLM thông thường có thể trả lời bằng văn bản nhưng không tự đọc file, tìm kiếm web hay tính toán bằng chương trình. Agent giải quyết giới hạn này bằng vòng lặp ReAct: mô hình chọn hành động, framework chạy tool, trả observation về cho mô hình, rồi tiếp tục cho đến khi có final answer. Mục tiêu của nhóm không chỉ là gọi API mà còn phải thiết kế đúng OOP: mỗi tầng có trách nhiệm riêng, công cụ có thể mở rộng và mọi lần chạy benchmark đều có trajectory để kiểm tra lại.

## Slide 5 — Kiến trúc tổng quan

### Nội dung trên slide

- `LLMClient` → `AgentLoop` → `ToolRegistry` → concrete tools
- `SkillLoader` cung cấp instruction theo task
- `HarnessRunner` quan sát Agent qua `StepHook`
- Agent không phụ thuộc ngược vào Harness

### Lời nói

> Kiến trúc được chia thành các thành phần rõ ràng. `LLMClient` che giấu khác biệt giữa Ollama và Gemini. `AgentLoop` điều khiển ReAct nhưng chỉ biết interface của tool. `ToolRegistry` quản lý tên, mô tả, factory và policy. `SkillLoader` chọn hướng dẫn phù hợp dựa trên từ khóa của task. Ở tầng ngoài, `HarnessRunner` nhận từng `TrajectoryStep` qua callback `StepHook`. Vì Agent chỉ phát sự kiện mà không biết ai đang nghe, phần Core không phụ thuộc vào Harness và có thể kiểm thử độc lập.

## Slide 6 — LLM Client và Agent Loop

### Nội dung trên slide

- Một interface cho text và ảnh base64
- Config: endpoint, model, temperature, max tokens, timeout
- Parser hỗ trợ JSON/fenced JSON và escaped arguments
- Lỗi được phân loại, không xem JSON hỏng là final answer

### Lời nói

> `LLMClient` cung cấp một interface chung cho cả text và multimodal message. Cấu hình provider được đọc từ file thay vì gắn cứng trong business logic. Trong mỗi bước, `AgentLoop` gửi toàn bộ conversation history, nhận phản hồi rồi parse tool call. Parser dùng quét object cân bằng kết hợp `nlohmann::json`, nên giữ được các chuỗi argument có escape. Nếu phản hồi có ý định gọi tool nhưng JSON bị hỏng, hệ thống trả lỗi parse rõ ràng và không thực thi một tool call bị cắt dở.

## Slide 7 — Loop Detector và Skill System

### Nội dung trên slide

- Generic repeat và ping-pong
- Warning/critical thresholds, critical thì dừng
- Ba skill Markdown có keyword metadata
- Skill phù hợp được inject trước mỗi run

### Lời nói

> Agent có thể lặp lại cùng một action hoặc đổi qua lại giữa hai action. `LoopDetector` lưu chuỗi action gần đây để nhận biết generic repeat và ping-pong. Khi vượt ngưỡng critical, Agent dừng an toàn thay vì tiếp tục tốn API. Song song đó, `SkillLoader` quét thư mục `skills`, đọc keyword metadata và chỉ inject skill phù hợp với task; nếu không có keyword khớp thì dùng `task_planner` mặc định.

### Demo Role A — 20–30 giây

```bash
./build/test_role_a
```

> Đây là focused test cho parser, history role và error contract của Agent Core. Video chỉ giữ phần tên test liên quan và dòng PASS cuối.

### Transition Role A → Role B

> Agent Core đã có khả năng suy luận và kiểm soát luồng. Tiếp theo, Role B sẽ trình bày cách Agent mở rộng khả năng thông qua OOP, Tools và Vector Memory.

---

# Role B — OOP, Modern C++ và Tools

## Slide 8 — Bốn Design Patterns bắt buộc

### Nội dung trên slide

| Pattern | Vị trí áp dụng |
|---|---|
| Strategy | `Evaluator` và `Tool` interfaces |
| Template Method | `AgentLoop::run()` |
| Registry/Factory | `ToolRegistry` |
| Observer/Hook | `StepHook` |

### Lời nói

> Hệ thống áp dụng đủ bốn pattern bắt buộc. Strategy cho phép nhiều Evaluator và Tool dùng chung interface. Template Method nằm tại `AgentLoop::run`: skeleton của vòng lặp được giữ cố định, còn các primitive có thể override trong test. Registry/Factory tạo hoặc tra cứu tool theo tên mà Core không cần biết concrete class. Cuối cùng, Observer được hiện thực bằng `StepHook`, giúp Harness ghi trajectory mà không tạo dependency ngược vào Agent.

## Slide 9 — Modern C++17 đến C++26

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

## Slide 10 — Tool Registry và Factory

### Nội dung trên slide

- Tool contract: name, description, `execute(arguments)`
- Runtime registration và fresh-instance factory
- Alias normalization
- Allow/deny policy
- Catalog động được đưa vào system prompt

### Lời nói

> Mỗi tool triển khai cùng một contract gồm tên, mô tả và hàm execute. `ToolRegistry` vừa giữ shared instance để lookup, vừa giữ creator lambda để tạo instance mới. Alias giúp chuyển tên tương thích về canonical name; allow/deny list chặn tool theo policy. Quan trọng hơn, Agent tạo phần Available Tools trực tiếp từ catalog của Registry. Vì vậy tên và mô tả mà LLM nhìn thấy luôn khớp với các tool thực sự đang đăng ký.

## Slide 11 — Bộ công cụ

### Nội dung trên slide

- Bắt buộc: shell, file, web search, memory, calculator
- Memory có entry rõ ràng: `memory_save`, `memory_search`
- Bổ sung: time, JSON, Git
- Tool failure trả `ToolError`, không dựng kết quả thành công giả

### Lời nói

> Project đáp ứng năm nhóm tool bắt buộc: chạy shell; đọc, ghi và append file; web search; persistent memory; và calculator. Hai tên production `memory_save` và `memory_search` giúp LLM gọi memory trực tiếp. Ba tool bổ sung là time, JSON và Git. Các tool kiểm tra input và trả `ToolError` cho trường hợp không hợp lệ, timeout hoặc dịch vụ ngoài không sẵn sàng. Cách này cho phép Agent và Harness phân biệt lỗi thật với output hợp lệ.

## Slide 12 — Persistent Memory và Vector Search (+4)

### Nội dung trên slide

- SQLite lưu text và embedding
- Production: Ollama `nomic-embed-text`
- Cosine similarity viết bằng C++
- `save/search` là vector path chính
- Offline test inject `HashEmbedder`; production không tự fallback

### Lời nói

> Bonus Vector Search thay keyword search chính bằng embedding similarity. Embedding là vector số biểu diễn nội dung văn bản; cosine similarity đo độ gần về hướng để xếp hạng semantic similarity. Khi lưu memory, production gửi text tới `nomic-embed-text` qua Ollama rồi lưu vector cùng dữ liệu trong SQLite. Offline test inject `HashEmbedder` để có kết quả xác định; production không tự fallback.

### Demo Role B — 40–50 giây

```bash
curl http://localhost:11434/api/tags
RUN_LIVE_OLLAMA=1 ./build/test_tools
```

> Output xác nhận Ollama có `nomic-embed-text` và live Vector acceptance PASS.

### Transition Role B → Role C

> Sau khi Agent có công cụ và bộ nhớ, câu hỏi tiếp theo là làm sao chứng minh toàn bộ hệ thống hoạt động đúng. Role C sẽ trả lời bằng Harness, Multi-agent và benchmark.

---

# Role C — Harness, Multi-agent và kết quả

## Slide 13 — Benchmark Harness

### Nội dung trên slide

- `setup → run → evaluate → record → cleanup`
- 10 task: **4 simple / 4 medium / 2 hard**
- Dọn artifact cũ trước mỗi batch
- Success rate là phép đo model, không phải unit-test score

### Lời nói

> `HarnessRunner` tự động hóa toàn bộ quá trình đánh giá. Trước mỗi batch, Harness dọn các artifact được khai báo để task không tận dụng kết quả cũ. Sau đó Agent chạy task, Evaluator kiểm tra output và Harness xuất evidence. Bộ benchmark có đúng 10 task theo yêu cầu: 4 đơn giản, 4 trung bình và 2 khó. Khác với CTest dùng để kiểm tra code, success rate của benchmark đo hành vi của model thật nên có thể thay đổi giữa các lần chạy.

## Slide 14 — Evaluator và Trajectory

### Nội dung trên slide

- `KeywordEvaluator` và `FunctionalEvaluator`
- Evaluator Strategy được chọn theo task
- Trajectory: action, args, result, success, latency, token usage
- Final answer được ghi nhưng không tính là tool step

### Lời nói

> `KeywordEvaluator` kiểm tra nội dung đầu ra, còn `FunctionalEvaluator` kiểm tra hậu điều kiện thật như file có tồn tại và đúng nội dung hay không. Mỗi bước chạy được lưu trong trajectory JSON, gồm tool, arguments, kết quả, trạng thái, latency và token metadata khi provider cung cấp. Final answer cũng được ghi để có luồng hoàn chỉnh, nhưng không bị tính nhầm là một lần gọi tool. Nhờ vậy, nhóm có thể xem chính xác task thất bại ở quyết định của model, tool hay evaluator.

## Slide 15 — Multi-agent Coordination (+3)

### Nội dung trên slide

- `HarnessRunner → MultiAgentRunner → 2 worker threads`
- `std::queue + mutex + condition_variable`
- Stop/join bằng RAII
- Demo thật: Calculator `47 × 23` và Researcher tìm thủ đô Nhật Bản

### Lời nói

> Bonus Multi-agent có đường tích hợp thật từ Harness sang `MultiAgentRunner`. Runner tạo hai worker thread và giao tiếp qua message queue được bảo vệ bởi mutex và condition variable. Khi kết thúc, runner gửi tín hiệu dừng và join toàn bộ thread để không để lại worker chạy nền. Demo hiện tại chia hai subtask độc lập: Calculator tính 47 nhân 23, còn Researcher tìm thủ đô Nhật Bản. Harness nhận hai kết quả và tạo report; test cũng kiểm tra rằng lỗi của worker không bị biến thành PASS.

## Slide 16 — Kết quả thực nghiệm

### Nội dung trên slide

- Run: `run_20260820_002933_100`
- Provider/model: Gemini `gemma-4-31b-it`
- Final success: **7/10 = 70%**
- Evaluator: **70%** · Action-level: **90%**
- Toàn bộ recorded action có source `llm`, không có fixture fallback

### Lời nói

> Run thật được chọn làm evidence là `run_20260820_002933_100`, sử dụng Gemini với model `gemma-4-31b-it`. Kết quả cuối là 7 trên 10 task, evaluator score 70% và action-level score 90%. Tất cả action được ghi với source là LLM, không dùng deterministic fixture để ép điểm. Ba task không pass cho thấy giới hạn của model: task 4 chọn sai tool; task 5 và 9 lặp lại calculator cho đến khi LoopDetector dừng. Đề yêu cầu báo cáo success rate, không yêu cầu model phải đạt 10/10.

## Slide 17 — Tổng kết và giới hạn

### Nội dung trên slide

- Technical gates: clean build, CTest **5/5 PASS**
- Vector live acceptance PASS
- Multi-agent focused test PASS
- 4 design patterns và 4 Mermaid diagrams
- Giới hạn: model không deterministic; GUI/VLM không được claim

### Lời nói

> Tóm lại, candidate kỹ thuật đã clean build và vượt qua 5 trên 5 CTest. Hai bonus nhóm chọn là Vector Search và Multi-agent đều có production path, focused test và tài liệu tương ứng. Hệ thống đáp ứng bốn design pattern và bốn Mermaid diagram bắt buộc. Nhóm không claim GUI/VLM là feature hoàn chỉnh và giữ nguyên kết quả benchmark 7 trên 10 để phản ánh model thật. Ba đoạn demo vừa xem nối trực tiếp requirement với code, test và evidence; đó cũng là tiêu chí nhóm dùng để chốt project.

## Slide 18 — Thank You

### Nội dung trên slide

- THANK YOU
- Cảm ơn thầy/cô và các bạn đã lắng nghe

### Lời nói

> Phần trình bày của nhóm em xin kết thúc tại đây. Nhóm em cảm ơn thầy/cô và các bạn đã lắng nghe.

---

# Demo Role C — 60–80 giây, đặt sau Slide 16 và trước Slide 17

Chuẩn bị sẵn terminal ở đúng repository. Không hiển thị `config.json`, API key hoặc thông tin cá nhân.

```bash
./build/demo_multi_agent '47 * 23' 'Japan capital'
cat artifacts/demo/report.txt
cat benchmark/results/run_20260820_002933_100/benchmark_summary.txt
```

Chỉ nhanh hai worker chạy qua queue, report tổng hợp và benchmark **7/10**. Mở trajectory của `task_010` để chỉ một task PASS có `source: llm`; nếu cần giải thích failure gate, mở `task_005` và chỉ `LOOP_DETECTED` thay vì che giấu lỗi.

> Chuyển cuối: “Multi-agent cho thấy hệ thống phối hợp được nhiều worker; Harness và trajectory cho thấy kết quả đó được kiểm chứng thế nào. Em xin kết thúc bằng các gate kỹ thuật và giới hạn đã xác nhận.”

Nếu dùng output đã chuẩn bị trước, đọc đúng run ID và nói rõ đây là run ngày 2026-08-20. Không chạy `run_eval` trực tiếp khi quay nếu provider không ổn định vì một run có thể mất nhiều phút.

---

# Code mapping khi Q&A

- Template Method và StepHook: [`agent_loop.h`](../../src/agent/agent_loop.h)
- Parser và ReAct implementation: [`agent_loop.cpp`](../../src/agent/agent_loop.cpp)
- Registry/Factory: [`ToolRegistry.cpp`](../../src/tools/ToolRegistry.cpp)
- Vector memory: [`MemoryTool.cpp`](../../src/tools/MemoryTool.cpp)
- Message queue: [`MessageQueue.h`](../../src/multiagent/MessageQueue.h)
- Harness/Multi-agent integration: [`HarnessRunner.cpp`](../../src/harness/HarnessRunner.cpp)
- Final evidence: [`requirement_traceability_final_2026-08-20.md`](../../docs/evidence/requirement_traceability_final_2026-08-20.md)

# Q&A thường gặp và cách trả lời ghi điểm

## Câu hỏi 1: Tại sao không dùng exceptions để xử lý lỗi của Tool mà lại dùng `std::expected`?

> Các lỗi như input không hợp lệ, file không tồn tại hoặc timeout là kết quả có thể dự đoán của một lần gọi tool. `std::expected<T, ToolError>` thể hiện ngay trong kiểu trả về rằng caller phải xử lý cả success và failure, nên contract rõ ràng và dễ kiểm thử. Exception vẫn phù hợp cho lỗi bất thường; nhóm không claim exceptions luôn chậm hoặc luôn gây crash.

## Câu hỏi 2: Thuật toán Cosine Similarity trong Vector Search hoạt động như thế nào?

> Cosine Similarity tính cosin của góc giữa hai vector bằng tích vô hướng chia cho tích độ dài của chúng. Kết quả nằm trong khoảng từ -1 đến 1; giá trị càng gần 1 thì hai vector càng cùng hướng và nội dung thường càng tương đồng về ngữ nghĩa. Project dùng phép tính này để xếp hạng các memory đã lưu theo độ gần với query.

## Câu hỏi 3: Bộ nhớ SQLite được quản lý vòng đời như thế nào để tránh leak file handle?

> `MemoryTool` áp dụng RAII. Constructor khởi tạo kết nối SQLite và object giữ quyền sở hữu handle. Khi object ra khỏi scope hoặc `unique_ptr` bị giải phóng, destructor gọi `sqlite3_close`, nên handle được đóng kể cả khi luồng chạy kết thúc sớm. `test_memory_lifecycle` kiểm tra cả database hợp lệ và đường dẫn database lỗi để bảo đảm không crash và cleanup an toàn.

# Checklist trước khi quay

- [ ] Thay “Role A/B/C” bằng tên và MSSV thật trên slide đầu.
- [ ] Thay placeholder logo, giảng viên, tên và MSSV trên slide đầu.
- [ ] Tập thử với đồng hồ; video hybrid hoàn chỉnh không vượt 10 phút.
- [ ] Slide 16 dùng đúng model, run ID và ba metric đã ghi.
- [ ] Ba clip demo được đặt ngay sau phần Role A, Role B và Role C; không tạo video demo thứ hai.
- [ ] Không claim `std::inplace_vector`, auto HashEmbedder fallback, zero warnings hoặc zero memory leaks.
- [ ] Không hiển thị secret, đường dẫn cá nhân hoặc `config.json`.
- [ ] Demo chỉ mô tả action thật sự có trong output/trajectory.
- [ ] Video được upload YouTube ở chế độ **Unlisted** và mở được khi đăng xuất.
