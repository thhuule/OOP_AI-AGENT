# Kế hoạch chi tiết Tuần 6 + Nửa Tuần 7 — AI-AGENT OOP 2026

> **Mục tiêu:** Loop detection + Harness hoàn chỉnh + C++ modern + Multi-agent foundation  
> **Deadline commit:** Cuối tuần, mỗi người ≥2 commit (vì gộp 1.5 tuần)  

## A — Systems / Core

### 1. Đọc URL từ `config.json`
**File:** `config.json` + `src/agent/agent_loop.cpp`

Tạo `config.json` ở root project:
```json
{
  "api_url": "http://url-colab-moi/api/chat",
  "model": "gemma4:e4b",
  "use_mock": false
}
```

Trong `run_eval.cpp` đọc config trước khi khởi tạo client:
```cpp
// Đọc config.json bằng nlohmann_json
// Nếu use_mock = true → dùng MockLLMClient
// Nếu use_mock = false → dùng OllamaClient với api_url
```

Lợi ích: đổi URL Colab chỉ cần sửa `config.json`, không cần recompile.

### 2. `LoopDetector`
**File:** `src/agent/LoopDetector.h` + `LoopDetector.cpp`

Phát hiện 2 loại loop:

**Generic repeat** — agent lặp lại cùng 1 action nhiều lần:
```cpp
// Nếu action[i] == action[i-1] == action[i-2] → loop
// Threshold: 3 lần giống nhau → warning
// Threshold: 5 lần giống nhau → critical, stop
```

**Ping-pong** — agent lặp đi lặp lại 2 action xen kẽ:
```cpp
// Nếu action[i] == action[i-2] và action[i-1] == action[i-3] → ping-pong
// Threshold: 3 chu kỳ → warning
// Threshold: 5 chu kỳ → critical, stop
```

Interface:
```cpp
class LoopDetector {
public:
    enum class Status { Normal, Warning, Critical };

    // Thêm action mới, trả về trạng thái
    Status add_action(const std::string& action);

    // Reset khi bắt đầu task mới
    void reset();

private:
    std::vector<std::string> history_;
    int warning_threshold_ = 3;
    int critical_threshold_ = 5;
};
```

### 3. Tích hợp `LoopDetector` vào `AgentLoop`
**File:** `src/agent/agent_loop.h/.cpp`

```cpp
// Trong AgentLoop::run(), sau mỗi tool call:
auto status = loop_detector_.add_action(tool_name);
if (status == LoopDetector::Status::Warning) {
    std::cerr << "[AgentLoop] Warning: loop detected\n";
}
if (status == LoopDetector::Status::Critical) {
    std::cerr << "[AgentLoop] Critical: stopping agent\n";
    break;
}
```

### 4. C++20 feature
Thêm ít nhất 1 C++20 feature vào codebase:
- **`std::span`** cho buffer trong `OllamaClient`
- **Concepts** để constraint template trong `Registry`
- **Ranges** để duyệt tool list

**Commit:** `[Week6-A] LoopDetector + config.json + C++20 feature`

---

## B — Tools / Data

### 1. Hoàn thiện `MemoryTool` (nếu chưa xong từ tuần 5)
**File:** `src/tools/MemoryTool.h/.cpp`

```cpp
// memory_save: INSERT INTO memory VALUES (text, timestamp)
// memory_search: SELECT * FROM memory WHERE text LIKE '%keyword%'
```

Cài SQLite:
```bash
sudo apt install libsqlite3-dev
```

Thêm vào `CMakeLists.txt`:
```cmake
find_package(SQLite3 REQUIRED)
target_link_libraries(OopAgent PRIVATE SQLite::SQLite3)
```

### 2. Hoàn thiện 3 tool bổ sung (nếu chưa xong từ tuần 5)
Chọn 3 tool thuộc 3 loại khác nhau. Gợi ý:
- `TimeTool` — `get_name() = "get_time"`, trả về thời gian hiện tại
- `JsonTool` — `get_name() = "json_format"`, parse/format JSON string
- `GitTool` — `get_name() = "git"`, chạy git command

### 3. Polish tất cả tools — Exception handling
Mỗi tool cần đảm bảo:
- Return `std::unexpected(ToolError::InvalidArgument)` khi input rỗng hoặc sai format
- Return `std::unexpected(ToolError::ExecutionFailed)` khi chạy lỗi
- Không throw exception ra ngoài — bắt hết trong `execute()`

### 4. `std::variant` cho Action type
**File:** `src/agent/agent_loop.h`

```cpp
// Thay vì dùng string thuần để biểu diễn action
// Dùng variant để type-safe hơn
struct ToolCallAction {
    std::string tool_name;
    std::string args;
};

struct FinalAnswerAction {
    std::string content;
};

using Action = std::variant<ToolCallAction, FinalAnswerAction>;
```

### 5. Smart pointer audit
Review toàn bộ tools:
- `unique_ptr` cho object chỉ có 1 owner
- `shared_ptr` cho object nhiều chỗ dùng
- Không dùng raw `new`/`delete`
- Không có memory leak

**Commit:** `[Week6-B] MemoryTool + 3 tools + polish + std::variant`

---

## C — Eval / Infra

### 1. `HarnessRunner` hoàn chỉnh — Batch evaluation
**File:** `src/harness/HarnessRunner.cpp`

`runAll()` hiện tại đã chạy được — cần thêm:
- In progress từng task: `[1/8] Running task_001...`
- Tính và in success rate sau mỗi task
- Phân loại kết quả theo category (simple/medium/hard)

```cpp
// Sau khi chạy hết:
std::cout << "Simple tasks: X/4 passed\n";
std::cout << "Medium tasks: X/4 passed\n";
std::cout << "Total: X/8 passed (" << rate << "%)\n";
```

### 2. Export JSON trajectory đúng format đề
**File:** `src/harness/HarnessRunner.cpp`

Hiện tại đã export được — verify lại format khớp với mục 7.1 trong đề:
```json
{
  "task_id": "task_001",
  "model": "gemma4:e4b",
  "success": true,
  "total_tokens": 312,
  "total_time_ms": 890,
  "steps": [
    {
      "step_id": 0,
      "thought": "...",
      "action": {"type": "tool_call", "tool": "calculator", "args": "47*23"},
      "tool_result": "1081",
      "tokens_used": 312,
      "latency_ms": 890
    }
  ]
}
```

### 3. Multi-agent foundation (nửa tuần 7)
**File:** `src/harness/MultiAgentRunner.h/.cpp`

Đây là phần quan trọng để lấy +3đ. Implement foundation:

```cpp
class MultiAgentRunner {
public:
    // Spawn sub-agent trên thread mới
    void spawn_agent(const std::string& task, int agent_id);

    // Chờ tất cả agent xong
    void wait_all();

    // Lấy kết quả từ tất cả agent
    std::vector<std::string> get_results();

private:
    std::vector<std::thread> threads_;
    std::queue<std::string> message_queue_;
    std::mutex queue_mutex_;
    std::vector<std::string> results_;
    std::mutex results_mutex_;
};
```

Implementation:
```cpp
void MultiAgentRunner::spawn_agent(const std::string& task, int agent_id) {
    threads_.emplace_back([this, task, agent_id]() {
        // Mỗi thread tạo AgentLoop riêng
        // Chạy task
        // Ghi kết quả vào results_ (có mutex)
        std::lock_guard<std::mutex> lock(results_mutex_);
        results_.push_back(result);
    });
}
```

### 4. Design test case cho 2-agent scenario
Viết 1 task phức tạp có thể chia cho 2 agent:

```
Task phức tạp: "Tính 47*23 VÀ tìm thủ đô Nhật Bản, lưu cả 2 vào report.txt"

→ Agent 1: tính toán → lưu kết quả tính
→ Agent 2: tìm kiếm → lưu kết quả tìm
→ Main: gộp 2 kết quả vào report.txt
```

**Commit 1:** `[Week6-C] HarnessRunner batch eval + trajectory format`
**Commit 2:** `[Week6-C] MultiAgentRunner foundation + 2-agent test case`

---

## Checkpoint cuối tuần

| | Verify | Người |
|---|---|---|
| `config.json` | Đổi URL không cần recompile | A |
| `LoopDetector` | Agent lặp 5 lần → tự dừng | A |
| C++20 | Ít nhất 1 feature có trong code | A |
| 8 tool đăng ký | `MemoryTool` + 3 tool bổ sung load được | B |
| Tool polish | Mỗi tool có error handling đầy đủ | B |
| Batch eval | Chạy 8 task, in success rate theo category | C |
| Trajectory | JSON output đúng format đề mục 7.1 | C |
| Multi-agent | Spawn 2 thread chạy song song không crash | C |
| Build | `cmake .. && make` không lỗi | Cả nhóm |
| Commit | Mỗi người ≥2 commit | Cả nhóm |

> ⚠️ Multi-agent là phần mới nhất và khó nhất — nếu bị block thì báo nhóm ngay, đừng để đến cuối tuần.
> `std::thread` + `std::mutex` dễ gây deadlock nếu không cẩn thận — test kỹ trước khi merge.
