# Kế hoạch chi tiết Tuần 5 — AI-AGENT OOP 2026
 
> **Mục tiêu tuần 5:** ReAct loop hoàn chỉnh + Tools batch 2 + FunctionalEvaluator  
> **Deadline commit:** Cuối tuần 5, mỗi người ≥1 commit  
> **⚠️ Checkpoint quan trọng nhất:** Cuối tuần 5, chạy thử pipeline đầu-cuối lần đầu tiên
 
---
 
## Review code hiện tại
 
### ✅ Tốt
- `LLMClient.h` — interface clean, dùng `std::expected`, `std::optional` đúng chỗ
- `Tool.h` — interface rõ ràng, `get_name()` + `get_description()` + `execute()` đủ cho LLM đọc
- `ToolRegistry.h` — có allow/deny list, dùng `unordered_map` + `unordered_set` hợp lý
- `HarnessRunner.h/.cpp` — Strategy Pattern + Observer Pattern đúng chuẩn
- `KeywordEvaluator` — logic split + match keyword ổn
- `tasks.json` — đúng format đề
### ⚠️ Cần fix trước tuần 5
 
**1. `SkillLoader.h` không có namespace `oop_agent`**
- Tất cả file khác đều dùng `namespace oop_agent`, riêng `SkillLoader` thì không
- Dễ gây conflict khi include chung
- Fix: thêm `namespace oop_agent { ... }` bao quanh class
**2. `AgentLoop` chưa có `StepHook`**
- `HarnessRunner::createStepHook()` tạo ra hook nhưng `AgentLoop` chưa có chỗ nhận hook
- Tuần 5 khi kết nối 2 tầng sẽ bị block nếu không fix
- Fix: A cần thêm method `set_step_hook(StepHook hook)` vào `AgentLoop`
**3. `AgentLoop` chưa inject `SkillLoader`**
- `run()` chưa có system prompt từ skill
- Fix: A cần inject `SkillLoader::getSystemPrompt()` vào `memory_` trước khi gọi LLM
**4. `tasks.json` thiếu dấu `]` đóng**
- File JSON hiện tại bị thiếu `]` ở cuối → parse lỗi
- Fix: thêm `]` vào cuối file
---
 
## A — Systems / Core
 
### 1. ReAct loop đầy đủ
**File:** `src/agent/agent_loop.cpp`
 
Implement `run()` theo vòng lặp ReAct:
```
Observe → Think → Act → Observe → ...
```
 
Cụ thể:
```cpp
std::string AgentLoop::run(const std::string& instruction, int max_steps) {
    // 1. Inject skill vào system prompt
    // 2. Add instruction vào memory_
    // 3. Loop max_steps lần:
    //    a. Gọi LLM → nhận response
    //    b. Parse tool call từ response
    //    c. Nếu có tool call → execute tool → add result vào memory_
    //    d. Nếu không có tool call → đây là final answer, return
    //    e. Gọi step_hook nếu có
    // 4. Nếu hết max_steps → return "Max steps reached"
}
```
 
### 2. Parse tool call từ LLM response
**File:** `src/agent/agent_loop.cpp`
 
Gemma4 trả về tool call theo format:
```json
{"tool": "calculator", "args": "15*17"}
```
 
Cần parse bằng `nlohmann_json`, fallback sang regex nếu JSON lỗi:
```cpp
// Thử parse JSON trước
// Nếu fail → dùng regex tìm pattern {"tool": "...", "args": "..."}
```
 
### 3. Thêm `set_step_hook()` vào `AgentLoop`
**File:** `src/agent/agent_loop.h`
 
```cpp
using StepHook = std::function<void(const std::string&, 
                                     const std::string&, 
                                     const std::string&)>;
void set_step_hook(StepHook hook);
```
 
### 4. Inject SkillLoader vào AgentLoop
**File:** `src/agent/agent_loop.h/.cpp`
 
```cpp
void set_skill_loader(std::shared_ptr<SkillLoader> loader);
```
 
### Rủi ro A tuần 5:
- Gemma4 không follow ReAct format → cần thêm instruction rõ trong system prompt
- Parse tool call phức tạp → implement JSON parse trước, regex là fallback
**Commit:** `[Week5-A] ReAct loop + tool call parser + step_hook`
 
---
 
## B — Tools / Data
 
### 1. `MemoryTool` với SQLite
**File:** `src/tools/MemoryTool.h/.cpp`
 
```cpp
// memory_save: lưu text vào SQLite
// memory_search: tìm kiếm theo keyword
```
 
Cần cài SQLite:
```bash
sudo apt install libsqlite3-dev
```
 
Thêm vào `CMakeLists.txt`:
```cmake
find_package(SQLite3 REQUIRED)
target_link_libraries(OopAgent PRIVATE SQLite::SQLite3)
```
 
### 2. 3 tool bổ sung từ OpenClaw
Chọn 3 tool thuộc 3 loại khác nhau trước tuần 5 bắt đầu. Gợi ý:
- `GitTool` — chạy git command
- `JsonTool` — parse/format JSON
- `TimeTool` — lấy thời gian hiện tại
### 3. Test end-to-end
- Agent gọi được `CalculatorTool`, `FileTool`, `ExecTool`, `WebSearchTool` thật sự
- Không chỉ unit test từng tool riêng
### Rủi ro B tuần 5:
- SQLite setup phức tạp → cài và test riêng trước khi integrate
- 3 tool bổ sung chưa chọn → quyết định ngay đầu tuần
**Commit:** `[Week5-B] MemoryTool + 3 OpenClaw tools`
 
---
 
## C — Eval / Infra (bạn)
 
### 1. `FunctionalEvaluator`
**File:** `src/harness/FunctionalEvaluator.h/.cpp`
 
Chạy `eval_script` bằng `ExecTool`, parse output có chứa "PASS" không:
 
```cpp
class FunctionalEvaluator : public Evaluator {
public:
    std::string_view get_name() const noexcept override {
        return "functional_eval";
    }
 
    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& eval_script
    ) override;
};
```
 
Implementation:
```cpp
// 1. Chạy eval_script bằng popen() hoặc ExecTool
// 2. Nếu output chứa "PASS" → is_passed = true, score = 1.0
// 3. Nếu timeout (>10s) → return EvalError::ExecutionTimeout
// 4. Nếu output không có "PASS" → is_passed = false, score = 0.0
```
 
### 2. Trajectory recording
**File:** `src/agent/Trajectory.h` + `src/harness/HarnessRunner.cpp`
 
Hiện tại `HarnessRunner` có `current_trajectory_` nhưng chưa export ra file JSON per task.
 
Cần implement export trajectory theo format đề:
```json
{
  "task_id": "task_001",
  "model": "gemma4",
  "success": true,
  "total_tokens": 0,
  "total_time_ms": 0,
  "steps": [
    {
      "step_id": 0,
      "thought": "...",
      "action": {...},
      "tool_result": "...",
      "tokens_used": 0,
      "latency_ms": 0
    }
  ]
}
```
 
### 3. Đăng ký `FunctionalEvaluator` vào `HarnessRunner`
**File:** `src/harness/HarnessRunner.cpp`
 
```cpp
// Trong constructor, thêm:
registerEvaluator("functional", std::make_unique<FunctionalEvaluator>());
```
 
### 4. Fix `tasks.json`
Thêm `]` đóng vào cuối file — hiện tại thiếu, sẽ crash khi parse.
 
### 5. 4 task trung bình cho benchmark
Thêm vào `tasks.json` 4 task mức medium — kết hợp 2-3 tool liên tiếp:
 
```json
{
  "id": "task_005",
  "description": "Tính toán và lưu kết quả",
  "instruction": "Tính 47 * 23 rồi lưu kết quả vào file result.txt",
  "eval_type": "functional",
  "eval_script": "test -f result.txt && grep 1081 result.txt && echo PASS",
  "max_steps": 10
},
{
  "id": "task_006",
  "description": "Tìm kiếm và ghi kết quả",
  "instruction": "Tìm thủ đô của Nhật Bản rồi ghi vào file capital.txt",
  "eval_type": "functional",
  "eval_script": "test -f capital.txt && grep -i tokyo capital.txt && echo PASS",
  "max_steps": 10
},
{
  "id": "task_007",
  "description": "Đọc file và tính tổng",
  "instruction": "Đọc file notes.txt, đếm số từ trong đó rồi in ra kết quả",
  "eval_type": "keyword",
  "expected_keywords": "words, count",
  "max_steps": 10
},
{
  "id": "task_008",
  "description": "Chạy script và lưu output",
  "instruction": "Chạy hello.sh rồi lưu output vào file output.txt",
  "eval_type": "functional",
  "eval_script": "test -f output.txt && grep Hello output.txt && echo PASS",
  "max_steps": 10
}
```
 
### Rủi ro C tuần 5:
- `FunctionalEvaluator` phụ thuộc `ExecTool` của B → cần B xong trước hoặc mock
- Trajectory format phức tạp → làm skeleton trước, điền data sau
**Commit:** `[Week5-C] FunctionalEvaluator + trajectory recording + 4 task medium`
 
---
 
## ⚠️ Checkpoint cuối tuần 5 — QUAN TRỌNG NHẤT
 
Đây là lần đầu tiên 3 tầng chạy cùng nhau. Cả nhóm verify:
 
| | Verify | Người chịu trách nhiệm |
|---|---|---|
| Pipeline | `AgentLoop::run()` gọi được LLM thật từ Colab | A |
| Tool call | Agent parse được tool call, gọi đúng tool | A + B |
| Skill inject | System prompt có nội dung từ skill file | A + C |
| Harness | `HarnessRunner::runAll()` chạy 4 task, ra PASS/FAIL | C |
| StepHook | Trajectory được ghi lại đúng format | A + C |
| Build | `cmake .. && make` không lỗi | Cả nhóm |
 
> ⚠️ Nếu pipeline không chạy được cuối tuần 5 → họp khẩn cả nhóm, xác định chỗ bị block và fix ngay.  
> Để qua tuần 6 mà pipeline chưa chạy → rủi ro cao không kịp deadline.