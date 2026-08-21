# AI Agent Framework — Outline đã duyệt

Ngôn ngữ slide và speaker notes: tiếng Việt. Tên class, API và thuật ngữ kỹ thuật giữ nguyên tiếng Anh. Q&A chỉ nằm trong script, không nằm trong deck.

## Role A — Systems / Core

### Slide 1: AI Agent Framework với Ollama và Gemini
- Logo trường/khoa, giảng viên, ba thành viên và MSSV
- Modern C++17/20/23/26

### Slide 2: Bài trình bày đi từ kiến trúc đến bằng chứng
- Agent Core → OOP & C++ → Tools & Vector → Evaluation → Evidence
- Roadmap chung cho toàn bộ video

### Slide 3: Sáu khái niệm giúp đọc toàn bộ hệ thống
- LLM, Tool, AI Agent, ReAct, Harness, Trajectory
- Định nghĩa ngắn trước khi đi vào code

### Slide 4: Từ chatbot sang Agent có khả năng hành động
- ReAct: Think → Act → Observe → Continue/Finish
- Mục tiêu: mở rộng được, lỗi rõ ràng, đánh giá tái lập

### Slide 5: Các tầng được tách bằng contract rõ ràng
- LLMClient → AgentLoop → ToolRegistry → tools
- SkillLoader và HarnessRunner/StepHook

### Slide 6: AgentLoop điều phối LLM, tool và history
- Config provider; parser nhiều format; typed error
- Assistant response và tool observation được giữ đúng role

### Slide 7: Loop Detector và Skill System giữ Agent đúng hướng
- Generic repeat, ping-pong, warning/critical
- Keyword metadata và task-specific skill injection
- Sau slide: clip demo Role A bằng `./build/test_role_a`

## Role B — Tools / Data

### Slide 8: Bốn design pattern nằm trên production path
- Strategy, Template Method, Registry/Factory, Observer/Hook

### Slide 9: Modern C++ làm rõ ownership và contract
- C++17 smart pointers/filesystem/variant/optional
- C++20 ranges; C++23 expected; C++26 deleted function with reason

### Slide 10: ToolRegistry mở rộng tool mà không sửa Agent Core
- Runtime registration/factory, alias, allow/deny, dynamic catalog

### Slide 11: Bộ tool đáp ứng requirement bắt buộc
- Shell, file, web, memory, calculator; Time, JSON, Git
- `memory_save` và `memory_search`; ToolError cho failure path

### Slide 12: Vector Search thay thế tìm kiếm từ khóa (+4)
- SQLite lưu text và embedding
- Ollama `nomic-embed-text` → cosine similarity C++
- `HashEmbedder` chỉ dùng offline test
- Sau slide: clip demo Role B bằng live Vector acceptance

## Role C — Evaluation / Infrastructure

### Slide 13: Harness biến một lần chạy thành evidence
- setup → run → evaluate → record → cleanup
- 10 task: 4 simple / 4 medium / 2 hard

### Slide 14: Trajectory giải thích vì sao task PASS hoặc FAIL
- KeywordEvaluator và FunctionalEvaluator
- Action, args, result, latency, token và final answer

### Slide 15: Hai worker phối hợp qua message queue (+3)
- HarnessRunner → MultiAgentRunner → hai thread → queue → report
- Stop/join bằng RAII; Calculator và Researcher

### Slide 16: Benchmark thật đạt 7/10
- Run `run_20260820_002933_100`, Gemini `gemma-4-31b-it`
- Final 70%; evaluator 70%; action-level 90%
- Action source là `llm`, không dùng fixture fallback
- Sau slide: clip demo Role C bằng Multi-agent + benchmark evidence

### Slide 17: Requirement kỹ thuật đã có bằng chứng kiểm thử
- Clean build và CTest 5/5 PASS
- Vector live acceptance và Multi-agent focused test PASS
- Không claim GUI/VLM hoặc benchmark 10/10

### Slide 18: Thank You
- Cảm ơn thầy/cô và các bạn đã lắng nghe
- Kết thúc tối giản, không đưa câu hỏi Q&A lên slide

## Transition contract

- Fade giữa các slide trong cùng phần.
- Push tại Slide 4, 8 và 13 để báo hiệu chuyển sang Core, Role B và Role C.
- Cuối Slide 7, 12 và 16 có handoff nói ngắn sang clip demo tương ứng.
- Video dùng một timeline hybrid 8–10 phút; không tách thêm video demo riêng.
