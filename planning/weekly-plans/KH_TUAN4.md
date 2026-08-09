# Kế hoạch chi tiết Tuần 4 — AI-AGENT OOP 2026
 
> **Mục tiêu tuần 4:** Core loop + Tools batch 1 + HarnessRunner skeleton  
> **Deadline commit:** Cuối tuần 4, mỗi người ≥1 commit  
> **Checkpoint:** Cuối tuần 4, A demo Ollama connect được từ Colab
 
---
 
## A — Systems / Core
 
### 1. `OllamaClient` hoàn chỉnh
**File:** `src/client/OllamaClient.h` + `OllamaClient.cpp`
 
Yêu cầu:
- Kế thừa `LLMClient` (đã lock interface tuần 3)
- POST request đến `/api/chat` dùng `libcurl`
- Parse JSON response dùng `nlohmann_json`
- Hỗ trợ text-only và multimodal (base64 ảnh) qua cùng 1 interface
- Config được: `base_url`, `model_name`, `temperature`, `max_tokens`
Error handling phải có:
- Timeout → throw hoặc return `std::unexpected(LLMError::Timeout)`
- Connection refused → `std::unexpected(LLMError::ConnectionRefused)`
- Malformed JSON → `std::unexpected(LLMError::MalformedResponse)`
Ví dụ request format:
```json
{
  "model": "gemma3",
  "messages": [
    {"role": "system", "content": "..."},
    {"role": "user", "content": "..."}
  ],
  "stream": false
}
```
 
### 2. `AgentLoop` skeleton
**File:** `src/agent/AgentLoop.h` + `AgentLoop.cpp`
 
Yêu cầu:
- Conversation history: `std::vector<Message>` (Message có role + content)
- Config: `max_steps` (default 10)
- Method `run(std::string task)` — gọi LLM, trả về response string
- Method `reset()` — clear history, bắt đầu task mới
- Chưa cần ReAct đầy đủ — chỉ cần gọi được LLM và nhận response
### 3. Test Ollama từ Colab
- Chạy Ollama server trên Colab + expose port bằng ngrok/localtunnel
- Test gọi `/api/chat` từ `OllamaClient` → nhận được response thật
- **Bắt buộc demo được cuối tuần 4** — cả nhóm cần xác nhận Ollama connect được trước khi tuần 5
### Rủi ro A tuần 4:
- `libcurl` API phức tạp → xem example trên docs.libcurl.se trước khi code
- Nếu ngrok free bị rate limit → thử localtunnel hoặc cloudflared
**Commit:** `[Week4-A] OllamaClient + AgentLoop skeleton + Ollama connect test`
 
---
 
## B — Tools / Data
 
### 1. `ExecTool`
**File:** `src/tools/ExecTool.h` + `ExecTool.cpp`
 
Yêu cầu:
- Kế thừa `Tool` (đã lock interface tuần 3)
- Chạy shell command bằng `popen()`
- Capture đủ stdout + stderr
- Timeout: kill process sau 10 giây (dùng `alarm()` hoặc thread)
- Return `std::optional<std::string>` — rỗng nếu lỗi
Ví dụ:
```cpp
ExecTool exec;
auto result = exec.execute("ls -la");
// result = "total 32\ndrwxr-xr-x ..."
```
 
### 2. `WebSearchTool`
**File:** `src/tools/WebSearchTool.h` + `WebSearchTool.cpp`
 
Yêu cầu:
- Kế thừa `Tool`
- Gọi DuckDuckGo Instant Answer API (free, không cần API key):
  ```
  GET https://api.duckduckgo.com/?q=<query>&format=json&no_html=1
  ```
- Dùng `libcurl` để HTTP GET
- Parse JSON, trả về field `AbstractText` hoặc `Answer`
- Nếu response rỗng → thử field `RelatedTopics[0].Text`
### 3. Tool policy trong `ToolRegistry`
**File:** `src/tools/ToolRegistry.h` + `ToolRegistry.cpp` (cập nhật)
 
Thêm vào `ToolRegistry`:
- `void setAllowList(std::vector<std::string> names)` — chỉ cho phép tool trong list
- `void setDenyList(std::vector<std::string> names)` — block tool trong list
- `bool isAllowed(std::string name)` — check trước khi execute
### Rủi ro B tuần 4:
- DuckDuckGo API đôi khi trả về `AbstractText` rỗng → cần fallback sang `RelatedTopics`
- `popen()` không capture stderr riêng → dùng `2>&1` trong command để merge
**Commit:** `[Week4-B] ExecTool + WebSearchTool + ToolRegistry policy`
 
---
 
## C — Eval / Infra (bạn)
 
> ✅ Đã xong từ tuần này: `KeywordEvaluator`, `tasks.json` (4 task), `run_eval.cpp` placeholder
 
### 1. `Task` struct
**File:** `src/harness/Task.h`
 
```cpp
#pragma once
#include <string>
 
namespace oop_agent {
 
struct Task {
    std::string id;             // "task_001"
    std::string category;       // "simple" | "medium" | "hard"
    std::string description;    // mô tả ngắn
    std::string input;          // câu hỏi/yêu cầu gửi cho agent
    std::string expected_output;// keyword hoặc script để evaluate
    std::string evaluator_type; // "keyword" | "functional"
    int max_steps = 10;         // giới hạn số bước agent được chạy
};
 
} // namespace oop_agent
```
 
### 2. `HarnessRunner` skeleton
**File:** `src/harness/HarnessRunner.h` + `HarnessRunner.cpp`
 
Header:
```cpp
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Task.h"
#include "Evaluator.h"
#include "Trajectory.h"
 
namespace oop_agent {
 
class HarnessRunner {
public:
    // Load danh sách task từ file JSON
    void setup(const std::string& tasks_file);
 
    // Chạy tất cả task (tuần 5 mới connect AgentLoop thật)
    void run();
 
    // Evaluate tất cả trajectory đã record
    void evaluate();
 
    // Lưu 1 trajectory vào results_
    void record(const Trajectory& trajectory);
 
    // In kết quả ra console
    void printSummary() const;
 
private:
    std::vector<Task> tasks_;
    std::vector<Trajectory> results_;
    std::shared_ptr<Evaluator> evaluator_;
};
 
} // namespace oop_agent
```
 
Implementation tuần 4 cần làm được:
- `setup()`: đọc `tasks.json` bằng `nlohmann_json`, parse thành `vector<Task>`
- `printSummary()`: in ra console từng task id + PASS/FAIL
- `run()` và `evaluate()`: để trống hoặc placeholder — tuần 5 mới implement
### 3. Load `tasks.json` test được
Sau khi implement `setup()`, chạy thử từ `run_eval.cpp`:
```cpp
HarnessRunner harness;
harness.setup("../benchmark/tasks.json");
// In ra: Loaded 4 tasks
```
 
Đảm bảo không crash khi load file → build pass → commit.
 
### Rủi ro C tuần 4:
- `nlohmann_json` parse lỗi nếu format JSON sai → dùng try/catch bắt `json::parse_error`
- `tasks.json` path tương đối phụ thuộc vào chỗ chạy binary → hardcode absolute path khi test, fix sau
**Commit:** `[Week4-C] HarnessRunner skeleton + Task struct + load tasks.json`
 
---
 
## Checkpoint cuối tuần 4
 
Cả nhóm verify được những điều sau trước khi qua tuần 5:
 
| | Verify | Người chịu trách nhiệm |
|---|---|---|
| Ollama connect | Gọi `/api/chat` từ Colab, nhận response thật | A |
| ExecTool | Chạy `ls` hoặc `echo hello`, capture output | B |
| WebSearchTool | Search "capital of Japan", trả về "Tokyo" | B |
| HarnessRunner | Load `tasks.json`, in ra 4 task không crash | C |
| Build pass | `cmake .. && make` không lỗi | Cả nhóm |
 
> ⚠️ Nếu A chưa connect được Ollama cuối tuần 4 → báo nhóm ngay.  
> Tuần 5 là lần đầu 3 tầng chạy cùng nhau — không có Ollama thì không test được gì cả.