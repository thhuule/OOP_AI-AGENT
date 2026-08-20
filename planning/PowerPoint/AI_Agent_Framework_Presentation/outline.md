# AI Agent Framework — Outline đã duyệt

Ngôn ngữ slide và speaker notes: tiếng Việt. Tên class, API và thuật ngữ kỹ thuật giữ nguyên tiếng Anh. Q&A không nằm trong deck.

## Slide 1: AI Agent Framework với Ollama và Gemini
- Modern C++17/20/23/26
- Role A: Systems/Core; Role B: Tools/Data; Role C: Eval/Infra
- Placeholder tên và MSSV cho ba thành viên
- Vai trò: cover tối giản

## Slide 2: Từ chatbot sang Agent có khả năng hành động
- LLM đơn thuần chỉ sinh văn bản
- ReAct: Think → Act → Observe → Continue/Finish
- Mục tiêu: mở rộng được, lỗi rõ ràng, đánh giá tái lập
- Vai trò: problem → solution

## Slide 3: Các tầng được tách bằng contract rõ ràng
- LLMClient → AgentLoop → ToolRegistry → tools
- SkillLoader chọn instruction theo task
- HarnessRunner quan sát qua StepHook
- Vai trò: architecture

## Slide 4: AgentLoop điều phối LLM, tool và history
- Interface chung cho text và ảnh base64
- Config provider thay vì hard-code
- Parser JSON/fenced JSON/escaped arguments
- Lỗi parse được phân loại
- Vai trò: runtime flow

## Slide 5: Loop Detector và Skill System giữ Agent đúng hướng
- Generic repeat và ping-pong
- Warning/critical thresholds
- Ba Markdown skill có keyword metadata
- Skill phù hợp được inject trước mỗi run
- Vai trò: control and reliability

## Slide 6: Bốn design pattern nằm trên production path
- Strategy
- Template Method
- Registry/Factory
- Observer/Hook
- Vai trò: OOP evidence

## Slide 7: Modern C++ được dùng để làm rõ ownership và contract
- C++17: smart pointers, filesystem, variant, optional
- C++20: ranges và views
- C++23: expected và println
- C++26: deleted function with reason
- Vai trò: language-feature timeline

## Slide 8: ToolRegistry mở rộng tool mà không sửa Agent Core
- Runtime registration và factory
- Alias normalization
- Allow/deny policy
- Catalog động đi vào system prompt
- Vai trò: registry flow

## Slide 9: Bộ tool đáp ứng requirement bắt buộc
- Shell, file, web search, memory, calculator
- memory_save và memory_search
- Time, JSON và Git
- ToolError cho failure path
- Vai trò: tool inventory

## Slide 10: Vector Search thay thế tìm kiếm từ khóa (+4)
- SQLite lưu text và embedding
- Production dùng Ollama nomic-embed-text
- Cosine similarity viết bằng C++
- HashEmbedder chỉ dành cho offline test
- Vai trò: representative bonus flow; sample slide

## Slide 11: Harness biến một lần chạy thành evidence
- setup → run → evaluate → record → cleanup
- 10 task: 4 simple / 4 medium / 2 hard
- Dọn artifact cũ trước mỗi batch
- Vai trò: evaluation pipeline

## Slide 12: Trajectory giải thích vì sao task PASS hoặc FAIL
- KeywordEvaluator và FunctionalEvaluator
- Action, arguments, result, latency và token
- Final answer không tính là tool step
- Vai trò: evaluator and traceability

## Slide 13: Hai worker phối hợp qua message queue (+3)
- HarnessRunner → MultiAgentRunner → hai thread
- Queue, mutex và condition_variable
- Stop/join bằng RAII
- Demo Calculator và Researcher
- Vai trò: multi-agent flow

## Slide 14: Benchmark thật đạt 7/10
- Run run_20260820_002933_100
- Gemini gemma-4-31b-it
- Final success 70%; evaluator 70%; action-level 90%
- Action source là llm, không dùng fixture fallback
- Vai trò: evidence dashboard

## Slide 15: Requirement kỹ thuật đã có bằng chứng kiểm thử
- Clean build và CTest 5/5 PASS
- Vector live acceptance PASS
- Multi-agent focused test PASS
- Không claim GUI/VLM hoặc benchmark 10/10
- Chuyển sang video demo
- Vai trò: conclusion
