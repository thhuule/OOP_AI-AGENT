# Kế hoạch chi tiết Nửa Tuần 7 còn lại + Tuần 8 — AI-AGENT OOP 2026

> **Mục tiêu:** C++ modern features + Multi-agent hoàn chỉnh + Full benchmark với Gemini thật  
> **Deadline commit:** Mỗi người ≥2 commit (1 tuần 7, 1 tuần 8)  
> **⚠️ Quan trọng:** Tuần 8 phải có success rate thật từ Gemini — không dùng mock

---

## A — Systems / Core

### Nửa tuần 7

**1. C++23 features**
- `std::expected<T,E>` — đã có rồi ✅
- Thêm: `std::print` (C++23) thay `std::cout` ở một số chỗ log
- Hoặc: `std::ranges::to` để convert container

**2. C++26 feature**
- `std::inplace_vector` hoặc `std::copyable_function` — chọn 1 phù hợp
- Tham khảo: https://en.cppreference.com/w/cpp/26

**3. Refactor `AgentLoop` — tách biệt khỏi Harness**

Đảm bảo `agent_loop.h` không có bất kỳ `#include` nào từ `harness/`:
```cpp
// ❌ Không được có
#include "harness/HarnessRunner.h"

// ✅ Chỉ expose hook interface
using StepHook = std::function<void(...)>;
void set_step_hook(StepHook hook);
```

**Commit:** `[Week7-A] C++23/26 features + AgentLoop refactor`

### Tuần 8

**1. Integration full pipeline test**
- Chạy `run_eval.cpp` với Gemini thật, 8 task liên tiếp
- Không crash, không hang
- Log rõ ràng từng task

**2. Fix parse tool call cho Gemini**
- Gemini có thể trả về format khác mock → test và điều chỉnh regex
- Đảm bảo `parse_tool_call()` handle được cả:
  - JSON thuần: `{"tool": "calculator", "args": "47*23"}`
  - JSON trong markdown: ` ```json {"tool": "..."} ``` `

**3. LoopDetector test với Gemini thật**
- LLM thật dễ loop hơn mock → verify LoopDetector dừng đúng lúc
- Log warning/critical ra console rõ ràng

**Commit:** `[Week8-A] Integration full pipeline + parse fix`

---

## B — Tools / Data

### Nửa tuần 7

**1. `Registry<T>` generic template**
**File:** `src/tools/Registry.h`

```cpp
template<typename T>
class Registry {
public:
    void register_item(const std::string& name, std::unique_ptr<T> item) {
        items_[name] = std::move(item);
    }

    T* get(const std::string& name) const {
        auto it = items_.find(name);
        return it != items_.end() ? it->second.get() : nullptr;
    }

    std::vector<std::string> list() const {
        std::vector<std::string> names;
        for (const auto& [k, v] : items_) names.push_back(k);
        return names;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<T>> items_;
};

// ToolRegistry có thể dùng Registry<Tool>
```

**2. Smart pointer audit lần cuối**
- Scan toàn bộ tools, không còn raw `new`/`delete`
- `unique_ptr` cho single owner, `shared_ptr` cho shared owner
- Dùng `make_unique` / `make_shared` thay vì `new`

**Commit:** `[Week7-B] Registry<T> generic + smart ptr audit`

### Tuần 8

**1. `VLMEvaluator` skeleton**
**File:** `src/harness/VLMEvaluator.h`

```cpp
class VLMEvaluator : public Evaluator {
public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "vlm_eval";
    }

    std::expected<EvalResult, EvalError> evaluate(
        const std::string& agent_output,
        const std::string& expected_output
    ) override;
    // TODO: dùng VLM để evaluate output có ảnh
};
```

Chưa cần implement thật — chỉ cần class đúng interface, build pass.

**2. Fix bugs từ integration test**
- Tool nào gọi sai tên với Gemini → fix `get_name()`
- Parse args lỗi → fix `execute()` input parsing
- Verify đủ 10 tool đăng ký trong `run_eval.cpp`

**Commit:** `[Week8-B] VLMEvaluator skeleton + bug fix`

---

## C — Eval / Infra (bạn)

### Nửa tuần 7

**1. Multi-agent demo hoàn chỉnh**
**File:** `src/harness/MultiAgentRunner.cpp` + test file

Demo scenario:
```
Task: "Tính 47*23 VÀ tìm thủ đô Nhật Bản, lưu cả 2 vào report.txt"

→ Agent 1 (calculator_agent):
   - Nhận task: "tính 47*23"
   - Gọi CalculatorTool → 1081
   - Push kết quả vào global_bus_

→ Agent 2 (search_agent):
   - Nhận task: "tìm thủ đô Nhật Bản"
   - Gọi WebSearchTool → "Tokyo"
   - Push kết quả vào global_bus_

→ Main thread:
   - Đọc 2 kết quả từ global_bus_
   - Ghi vào report.txt
```

Lưu ý rate limit Gemini 30 RPM — thêm sleep giữa 2 agent:
```cpp
std::this_thread::sleep_for(std::chrono::seconds(3));
```

**2. Fix `startAll()` — copy task_func trước khi spawn**
```cpp
for (auto& [id, config] : agents_) {
    auto* in_q = in_queues_[id].get();
    auto func_copy = config.task_func; // copy trước để tránh UB

    worker_threads_.emplace_back([this, id, func_copy, in_q]() {
        func_copy(*in_q, global_bus_);
    });
}
```

**Commit:** `[Week7-C] MultiAgentRunner demo hoàn chỉnh`

### Tuần 8

**1. 2 task khó cho benchmark**

Thêm vào `tasks.json`:

```json
{
  "id": "task_009",
  "description": "Multi-step: tính toán, lưu file, đọc lại verify",
  "instruction": "Tính 123 * 456, lưu kết quả vào file calc.txt, sau đó đọc lại file và xác nhận kết quả đúng",
  "eval_type": "functional",
  "eval_script": "test -f calc.txt && grep 56088 calc.txt && echo PASS",
  "max_steps": 15
},
{
  "id": "task_010",
  "description": "Error recovery: tìm file không tồn tại, tạo mới, ghi dữ liệu",
  "instruction": "Đọc file data.txt (nếu không có thì tạo mới với nội dung 'initial data'), sau đó ghi thêm dòng 'appended' vào cuối file",
  "eval_type": "functional",
  "eval_script": "test -f data.txt && grep appended data.txt && echo PASS",
  "max_steps": 15
}
```

**2. Chạy full benchmark 10 task với Gemini thật**
- Chạy `./OopAgent` với Gemini thật
- Lưu output ra file để có log
- Chụp màn hình success rate để dùng cho báo cáo

**3. Phân tích kết quả**

Ghi chú lại (dùng cho báo cáo mục 8 tuần 9):
- Task nào PASS, task nào FAIL
- FAIL vì lý do gì: LLM không follow format, tool không tìm thấy, timeout?
- Success rate theo category: simple X/4, medium X/4, hard X/2

**Commit:** `[Week8-C] 2 hard tasks + full benchmark results`

---

## Checkpoint cuối tuần 8

| | Verify | Người |
|---|---|---|
| C++23/26 | ≥2 C++23 + ≥1 C++26 feature trong code | A |
| AgentLoop | Không có `#include` từ `harness/` | A |
| Integration | 8 task chạy không crash với Gemini thật | A |
| Registry<T> | Template generic compile được | B |
| VLMEvaluator | Class đúng interface, build pass | B |
| Multi-agent | 2 agent chạy song song, ra `report.txt` | C |
| 10 tasks | task_009 + task_010 có trong tasks.json | C |
| Benchmark | Success rate % thật, có log để báo cáo | C |
| Build | `cmake .. && make` không lỗi | Cả nhóm |
| Commit | Mỗi người ≥2 commit | Cả nhóm |

---

## Lưu ý quan trọng

**Rate limit Gemini free tier:** 30 RPM — khi chạy 10 task × ~8 calls/task = ~80 calls. Nên thêm sleep 3-4 giây giữa mỗi task trong `runAll()`.

**Multi-agent + Gemini:** 2 agent cùng gọi API → dễ hit rate limit. Serialize API calls bằng mutex hoặc chạy tuần tự nếu bị throttle.

**Lưu benchmark output:** Mỗi lần chạy benchmark → copy `eval_results.json` ra nơi khác để có data cho báo cáo tuần 9.
