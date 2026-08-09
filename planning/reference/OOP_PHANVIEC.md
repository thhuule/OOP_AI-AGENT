# Phân việc nhóm — Đồ án OOP 2026: AI Agent với Ollama API
 
> **Nhóm:** 3 người, năng lực tương đương  
> **Bắt đầu:** Tuần 3 | **Code freeze nội bộ:** cuối Tuần 10 | **Nộp đầy đủ:** trước 21:00 Chủ nhật Tuần 12
> **Điểm thưởng đã chốt Tuần 10:** Vector Search (+4đ), Multi-agent (+3đ) và VLM/GUI Agent (+8đ); chỉ bắt đầu/merge sau mọi gate bắt buộc.
>
> **Nguồn ràng buộc hiện hành:** [`OOP Project 2026 AI Agent.docx (1).md`](OOP%20Project%202026%20AI%20Agent.docx%20%281%29.md), [tổng quan Tuần 10](../weekly-plans/KH_Tuan10_TongQuan_1Trang.md) và [kế hoạch chi tiết Tuần 10](../weekly-plans/KH_Tuan10_ChiTiet.md). Mốc demo live Tuần 13 đã bị hủy; Tuần 12 phải nộp thiết kế, source, báo cáo hoàn chỉnh và link YouTube video demo ở chế độ Unlisted.
 
---
 
## Phân vai 3 người
 
| Người | Role | Layer phụ trách |
|---|---|---|
| **A** | Systems / Core | LLMClient + AgentLoop + parser + SkillLoader + LoopDetector + Environment + UML |
| **B** | Tools / Data | Tool/ToolRegistry/Factory + 5 tool bắt buộc + ít nhất 3 tool bổ sung + MemoryTool (SQLite) |
| **C** | Eval / Infra | Harness + Evaluator + Benchmark + trajectory + Multi-agent + build/docs/video |
 
---
 
## Timeline chi tiết (Tuần 3 → 12)
 
### Tuần 3 — Foundation & Setup
 
**A:**
- Setup repo GitHub, `CMakeLists.txt`, cấu trúc thư mục theo đề
- Viết abstract interface `LLMClient.h`; phối hợp B/C khóa `Tool.h`, `Evaluator.h` (pure virtual)
- `SkillLoader` skeleton: scan thư mục `skills/`, load `.md`; viết 3 skill có nội dung thực
- Test kết nối Ollama từ Google Colab → gọi được `/api/chat`
**B:**
- `Tool.h` abstract class + `ToolRegistry` skeleton (đăng ký/lookup theo tên)
- Implement `CalculatorTool` + `FileTool` (read/write) — 2 tool đơn giản nhất
**C:**
- Setup `Trajectory` + `Step` data class
- Chốt format `benchmark/tasks.json` và khung test/harness không phụ thuộc Agent internals
> ⚠️ **Action quan trọng tuần 3:** Cả nhóm review và **lock interface** `Tool.h`, `LLMClient.h`, `Evaluator.h` trước khi làm tiếp. Thay đổi interface sau tuần 4 sẽ break nhiều thứ.
 
**Commit target:** repo có structure, build được, Ollama connect được.
 
---
 
### Tuần 4 — Core loop + Tools batch 1
 
**A:**
- `OllamaClient` hoàn chỉnh: POST `/api/chat`, handle text + multimodal (base64 ảnh) qua cùng `LLMClient` interface
- Cấu hình đủ base URL, model, temperature, `max_tokens`, timeout; test request serialization
- Error handling: timeout, connection refused, malformed JSON
- `AgentLoop` skeleton: conversation history, max\_steps
**B:**
- `ExecTool` (chạy shell command, capture stdout/stderr)
- `WebSearchTool` (DuckDuckGo hoặc SearXNG)
- Tool policy: allow/deny list
**C:**
- `HarnessRunner` skeleton: setup → run → evaluate → record
- `KeywordEvaluator` implementation
- Viết 4 task đơn giản cho `tasks.json`
---
 
### Tuần 5 — ReAct loop hoàn chỉnh + Tools batch 2
 
**A:**
- ReAct loop đầy đủ: Observe → Think → Act → Observe
- Parse tool call từ LLM response (regex + JSON fallback)
- Inject skill vào system prompt
**B:**
- `MemoryTool` với SQLite: `memory_save` + `memory_search`
- 3 tool bổ sung tham khảo OpenClaw hoặc Hermes, thuộc 3 loại khác nhau; lưu bảng nguồn tham khảo và test — chọn trước tuần 5 bắt đầu
- Test end-to-end: Agent gọi được tool thật
**C:**
- `FunctionalEvaluator`: chạy eval\_script, parse PASS/FAIL
- Trajectory recording: lưu thought/action/result/latency/tokens từng step
- 4 task trung bình cho benchmark
> ⚠️ **Điểm nguy hiểm nhất:** Tuần 5 là lần đầu 3 tầng chạy cùng nhau. Nếu interface chưa thống nhất từ tuần 3 thì sẽ bị tắc ở đây.
 
---
 
### Tuần 6 — Loop detection + Harness đầy đủ
 
**A:**
- `LoopDetector`: generic repeat + ping-pong, configurable threshold
- Warning vs critical log, graceful stop
- Integration test: agent chạy có loop → detect đúng
- Tạo `Environment` abstract cùng `NativeEnvironment`/`SandboxEnvironment`; giữ Harness phụ thuộc interface
**B:**
- Polish tất cả tools: trả `std::expected<T, ToolError>` với lỗi cụ thể; dùng `std::optional<T>` ở nơi giá trị có thể vắng
- C++17 features pass: `std::variant` cho Action type, `std::filesystem` cho SkillLoader
- Unit test từng tool
**C:**
- `HarnessRunner` hoàn chỉnh: inject `step_hook` vào AgentLoop (Observer pattern)
- Batch evaluation: chạy tập task, tính success rate
- Export JSON trajectory đúng format spec
---
 
### Tuần 7 — C++ modern features + Multi-agent foundation
 
**A:**
- C++20: chứng minh ít nhất 2 feature độc lập (ví dụ `std::ranges` và một feature phù hợp khác)
- C++23: chứng minh ít nhất 2 feature (`std::expected<T,E>` và `std::print`/`std::println`) trên đường code được test
- C++26: guarded `std::inplace_vector` hoặc 1 feature phù hợp, có portability fallback
**B:**
- Ensure đạt tối thiểu 4 feature C++17 theo đề; có thể giữ target nội bộ cao hơn nếu đều có source/test thật
- Smart pointer audit: không leak, `unique_ptr`/`shared_ptr` đúng chỗ
- Template `Registry<T>` và Factory creator theo tên; không chỉ lookup object có sẵn
**C:**
- **Multi-agent foundation:** `HarnessRunner` spawn sub-agent trên thread mới
- `std::queue` + `mutex` cho message queue giữa agents
- Design test case cho 2-agent scenario
---
 
### Tuần 8 — Multi-agent hoàn chỉnh + 2 task khó + integration
 
**A:**
- Integration full pipeline: LLMClient → AgentLoop → Tool → Harness
- Refactor đảm bảo abstraction: AgentLoop không biết Harness tồn tại
- Refactor `AgentLoop::run()` thành Template Method đúng nghĩa: skeleton cố định + primitive operations/hook override được; có subclass test
**B:**
- Tích hợp Registry/Factory để tạo concrete tool theo tên; test alias/policy/create/unknown/duplicate
- Fix bugs từ integration test tuần 7
**C:**
- `VLMEvaluator` (Strategy pattern; nếu còn skeleton thì ghi đúng trạng thái, không nhận là evaluator ảnh hoàn chỉnh)
- Multi-agent demo: task phức tạp → chia 2 sub-agent chạy song song
- Muốn nhận +3đ, nối khả năng spawn sub-agent vào `HarnessRunner` qua `MultiAgentRunner`/Agent public API và test message queue; demo riêng hiện tại chưa tự động chứng minh đúng tiêu chí bonus
- 2 task khó cho benchmark (multi-step, agent tự quyết thứ tự tool call)
- Benchmark chạy được, success rate có số
---
 
### Tuần 9 — UML + Báo cáo draft

**A (chủ trì UML):**
- Vẽ Class Diagram toàn hệ thống (Mermaid)
- Vẽ Sequence Diagram — 1 agent run hoàn chỉnh
- Vẽ Sequence Diagram — HarnessRunner batch evaluation
- Vẽ Component Diagram
- Audit class hierarchy tối thiểu, đặc biệt `Environment` → `NativeEnvironment`/`SandboxEnvironment`
- Khóa bằng chứng 4 pattern bắt buộc: Strategy, Template Method, Registry/Factory, Observer/Hook; blocker nào chưa có source + test phải sửa trước khi chốt báo cáo
**B:**
- Viết phần báo cáo: Tools — thiết kế, implementation, thách thức
- Lập bảng tool → loại → nguồn OpenClaw/Hermes → args/dependency → test để chứng minh 3 tool bổ sung thuộc 3 loại
**C:**
- Viết phần báo cáo: Benchmark/Eval — success rate, phân tích kết quả
- README: build instructions, cấu hình Ollama, run example
- Lập checklist gói nộp Tuần 12 và storyboard video YouTube Unlisted
**A:**
- Viết phần báo cáo: Thiết kế OOP — class hierarchy, design patterns, abstraction layers
- Lập ma trận C++: ≥4 C++17, ≥2 C++20, ≥2 C++23, ≥1 C++26; mỗi feature có file, mục đích, test và fallback nếu cần
---
 
### Tuần 10 — Hoàn tất implementation + Code Freeze

- Đóng toàn bộ requirement bắt buộc, bug, focused test và evidence/source path cho docs theo `KH_Tuan10_ChiTiet.md`; render 4 UML.
- Clean build, 4 test executable, CTest, clean-state/benchmark evidence, sanitizer (hoặc limitation), package dry-run và clean extraction.
- Sau mọi gate bắt buộc PASS, triển khai/merge lần lượt Vector Search (+4), Multi-agent Coordination (+3) và VLM/GUI Agent (+8); mỗi bonus merge xong phải full regression lại.
- Chốt final revision/freeze note cuối tuần. Sau freeze: không thêm feature mới, trừ Critical Fix → targeted test → regression → re-freeze.
---
 
### Tuần 11 — Presentation Phase (sau Code Freeze)

- Merge, format và review cuối reports/README/submission checklist theo evidence của revision đã freeze; không thêm claim hoặc feature mới.
- Làm slide theo final revision; cả nhóm review tính chính xác kỹ thuật.
- Chốt demo flow, quay/chỉnh video, đặt YouTube Unlisted và kiểm tra link bằng cửa sổ chưa đăng nhập.
- Chuẩn bị oral: mỗi người giải thích phần code/test/limitation mình phụ trách.
- Không thêm implementation feature mới; chỉ Critical Fix theo quy trình re-freeze.
 
---
 
### Tuần 12 — Nộp đầy đủ
 
> **Deadline: trước 21:00 Chủ nhật**
 
- Class diagram + sequence diagram theo yêu cầu thiết kế; giữ đủ bốn UML trong tài liệu dự án
- Source code + README hướng dẫn biên dịch/chạy
- Báo cáo hoàn chỉnh + slide
- Link YouTube video demo ở chế độ Unlisted
- ZIP đúng tên: `MSSV1_MSSV2_MSSV3_OopAgent.zip`
- Theo đề, tên ZIP chỉ cần có đủ ba MSSV; hậu tố `OopAgent` có thể bỏ
- Đủ cấu trúc thư mục theo mục VI đề bài
- Personal Access Token quyền read-only để giảng viên truy cập repository private
- Nộp qua Moodle
---
 
### Tuần 13 — Không còn mốc demo live
 
Đề cập nhật đã bỏ demo trực tiếp. Nhóm chỉ lưu bản nộp, kiểm tra link video còn truy cập được và sẵn sàng giải thích code khi giảng viên cần làm rõ; không lập thêm deliverable Tuần 13.
---
 
## Commit tracking
 
**Yêu cầu đề:** ≥18 commit cho nhóm 3, khoảng cách không quá 7 ngày.
 
| Tuần | A | B | C |
|------|---|---|---|
| 3 | repo + interfaces + SkillLoader | ToolRegistry + 2 tools | Trajectory + task schema |
| 4 | OllamaClient | ExecTool + WebTool | HarnessRunner skeleton |
| 5 | ReAct loop | MemoryTool + 3 tools | FunctionalEvaluator |
| 6 | LoopDetector | Tool polish + tests | Harness + JSON export |
| 7 | C++20/23/26 | Smart ptr audit | Multi-agent foundation |
| 8 | Integration refactor | VLMEvaluator | Multi-agent demo |
| 9 | UML + báo cáo OOP | Báo cáo Tools | README + Báo cáo Eval |
| 10 | Core/parser/client bug fix + docs/UML | Tool error/package fix + docs | Full regression/benchmark/freeze |
| 11 | Review frozen code + oral | Review frozen code + oral | Slide/video/demo flow |
 
**8 tuần × 3 người = 24 commit** → đủ an toàn, mỗi người ≥8 commit riêng.
 
---
 
## Checklist điểm tối đa (100đ)
 
### Thiết kế OOP (25đ)
- [ ] Class diagram đầy đủ, đúng UML notation
- [ ] Inheritance hierarchy hợp lý, không vi phạm LSP
- [ ] 4 design patterns đúng context: Strategy, Template Method, Registry/Factory, Observer
- [ ] Template Method và Registry/Factory có source + focused test, không chỉ xuất hiện trong báo cáo
- [ ] Separation of concerns: AgentLoop không biết Harness
- [ ] Có `Environment` abstract → `NativeEnvironment`, `SandboxEnvironment` theo class hierarchy tối thiểu
### Kỹ thuật C++ (20đ)
- [ ] ≥4 tính năng C++17 trong bảng yêu cầu, mỗi feature có source/test cụ thể
- [ ] Thêm ≥2 C++20, ≥2 C++23, ≥1 C++26
- [ ] Không tính `std::string_view` là C++20; C++26 có guarded portability fallback
- [ ] Không memory leak, smart pointer đúng chỗ
- [ ] Exception handling có ý nghĩa
### Chức năng (25đ)
- [ ] 5 tool bắt buộc hoạt động đúng (exec, read/write, web\_search, memory, calculator)
- [ ] 3 tool bổ sung thuộc 3 loại khác nhau, có nguồn tham khảo OpenClaw/Hermes
- [ ] LLM config đủ base URL, model, temperature, `max_tokens`, timeout; text-only và multimodal đi qua cùng interface
- [ ] Agent loop + loop detection (2 loại)
- [ ] Skill system load và inject đúng
- [ ] Harness runner + trajectory JSON output
### Benchmark (15đ)
- [ ] 10 task hợp lệ (4 đơn giản, 4 trung bình, 2 khó)
- [ ] Evaluator chạy đúng, JSON hợp lệ
- [ ] Báo cáo success rate có phân tích
### Tài liệu (15đ)
- [ ] README: build, run, cấu hình Ollama
- [ ] Báo cáo: thiết kế, khó khăn, kết quả
- [ ] Slide thuyết trình mạch lạc
- [ ] Trước 21:00 Chủ nhật Tuần 12: thiết kế + source + báo cáo hoàn chỉnh + link YouTube Unlisted đã kiểm tra
### Điểm thưởng đã chốt (tối đa +15đ; chỉ sau mandatory gates)
- [ ] Vector Search (+4đ): embedding-based memory, cosine similarity, focused ranking test và regression pass.
- [ ] Multi-agent (+3đ): Harness spawn sub-agent trên thread mới; message queue với `std::queue` + `mutex`; demo task phức tạp chia 2 agent song song.
- [ ] GUI Agent (+8đ): screenshot → VLM → bounded action executor → controlled browser-search/copy demo; focused test + regression pass.
