# Kế hoạch chi tiết Tuần 6 + Nửa Tuần 7 — AI-AGENT OOP 2026

> **Thay đổi quan trọng:** Chuyển từ Ollama API → **Google Gemini API** (Google AI Studio)  
> **Mục tiêu:** Gemini Client + Loop detection + Harness hoàn chỉnh + C++ modern + Multi-agent foundation  
> **Deadline commit:** Cuối tuần, mỗi người ≥2 commit (vì gộp 1.5 tuần)  

---

## 📌 Bảng so sánh: Ollama vs Gemini API

Để cả nhóm hiểu rõ sự thay đổi, đây là bảng mapping giữa 2 API:

| | **Ollama (cũ)** | **Gemini (mới)** |
|---|---|---|
| **Endpoint** | `http://<colab-url>/api/chat` | `https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent?key={API_KEY}` |
| **Auth** | Không cần (local server) | API Key qua query param `?key=` hoặc header `x-goog-api-key` |
| **Model name** | `gemma4:e4b` | `gemini-2.5-flash` (hoặc model khác trên AI Studio) |
| **Request format** | `{"model":"...", "messages":[{"role":"user","content":"..."}], "stream":false}` | `{"contents":[{"role":"user","parts":[{"text":"..."}]}], "generationConfig":{"temperature":0.7}}` |
| **Role mapping** | `system`, `user`, `assistant`, `tool` | `user`, `model` (system prompt → `systemInstruction`) |
| **Response path** | `response["message"]["content"]` | `response["candidates"][0]["content"]["parts"][0]["text"]` |
| **Multimodal** | `"images": ["base64..."]` trong message | `"parts": [{"text":"..."}, {"inlineData":{"mimeType":"image/png","data":"base64..."}}]` |
| **Thư viện C++** | libcurl + nlohmann/json (giữ nguyên) | libcurl + nlohmann/json (**giữ nguyên, không cần thêm gì**) |

> [!IMPORTANT]
> **Tin tốt**: Vì cả 2 API đều là REST + JSON, nên chỉ cần viết thêm 1 class `GeminiClient` kế thừa `LLMClient` — toàn bộ code còn lại (AgentLoop, Tools, Harness) **không cần sửa gì** nhờ thiết kế abstract interface tốt!

---

## A — Systems / Core

### 1. ⭐ Tạo `GeminiClient` (thay thế OllamaClient)
**File mới:** `src/client/gemini_client.h` + `gemini_client.cpp`

Đây là việc quan trọng nhất tuần này. Class `GeminiClient` kế thừa `LLMClient` (đã có), chỉ khác cách build JSON request và parse response.

**gemini_client.h:**
```cpp
#pragma once
#include "client/llm_client.h"

namespace oop_agent {

class GeminiClient : public LLMClient {
public:
    GeminiClient(const std::string& api_key, const std::string& model = "gemini-2.5-flash");
    ~GeminiClient() override = default;

    std::expected<std::string, LLMError> generate_chat(
        const std::vector<Message>& conversation_history,
        const LLMConfig& config = LLMConfig{}
    ) override;

private:
    std::string api_key_;
    std::string model_name_;
    
    // Build URL: https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent?key={key}
    std::string build_url() const;
    
    // Convert Message vector → Gemini JSON format
    nlohmann::json build_request_body(
        const std::vector<Message>& history, 
        const LLMConfig& config
    ) const;
    
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace oop_agent
```

**Hướng dẫn implement `gemini_client.cpp`:**

```cpp
// ── Build URL ──
std::string GeminiClient::build_url() const {
    return "https://generativelanguage.googleapis.com/v1beta/models/" 
           + model_name_ + ":generateContent?key=" + api_key_;
}

// ── Convert conversation history → Gemini format ──
// Ollama format:  {"role":"user",      "content":"Hello"}
// Gemini format:  {"role":"user",      "parts":[{"text":"Hello"}]}
//
// Lưu ý mapping role:
//   Ollama "system"    → Gemini "systemInstruction" (tách riêng, không nằm trong contents)
//   Ollama "assistant" → Gemini "model"
//   Ollama "tool"      → Gemini "user" (đưa observation vào user turn)
//   Ollama "user"      → Gemini "user"

json GeminiClient::build_request_body(
    const std::vector<Message>& history,
    const LLMConfig& config
) const {
    json body;
    json contents = json::array();
    
    for (const auto& msg : history) {
        if (msg.role == "system") {
            // System prompt → systemInstruction (ngoài contents)
            body["systemInstruction"] = {
                {"parts", {{{"text", msg.content}}}}
            };
            continue;
        }
        
        // Map role: assistant → model, tool → user
        std::string gemini_role = msg.role;
        if (gemini_role == "assistant") gemini_role = "model";
        if (gemini_role == "tool")      gemini_role = "user";
        
        json parts = json::array();
        parts.push_back({{"text", msg.content}});
        
        // Multimodal: nếu có ảnh base64
        if (msg.images.has_value()) {
            for (const auto& img : msg.images.value()) {
                parts.push_back({
                    {"inlineData", {
                        {"mimeType", "image/png"},
                        {"data", img}
                    }}
                });
            }
        }
        
        contents.push_back({{"role", gemini_role}, {"parts", parts}});
    }
    
    body["contents"] = contents;
    body["generationConfig"] = {
        {"temperature", config.temperature}
    };
    
    return body;
}

// ── Parse response ──
// Gemini trả về:
// {
//   "candidates": [{
//     "content": {
//       "parts": [{"text": "...câu trả lời..."}]
//     }
//   }]
// }
// → Lấy: response["candidates"][0]["content"]["parts"][0]["text"]
```

**Phần gọi HTTP (libcurl):** Gần giống `OllamaClient`, chỉ khác:
- URL = `build_url()` thay vì `config.api_url`
- Request body = `build_request_body(...)` thay vì format Ollama
- Parse response theo path `candidates[0].content.parts[0].text`

### 2. Cập nhật `config.json` cho Gemini
**File:** `config.json`

```json
{
  "provider": "gemini",
  "api_key": "AIzaSy..._your_key_here",
  "model": "gemini-2.5-flash",
  "api_url": "https://generativelanguage.googleapis.com/v1beta",
  "use_mock": false
}
```

### 3. Cập nhật `LLMConfig` để hỗ trợ cả 2 provider
**File:** `src/client/llm_client.h`

```cpp
struct LLMConfig {
    std::string provider = "gemini";           // "gemini" hoặc "ollama"
    std::string model_name = "gemini-2.5-flash";
    std::string api_url = "https://generativelanguage.googleapis.com/v1beta";
    std::string api_key = "";                  // MỚI: cho Gemini
    float temperature = 0.7f;
    int timeout_seconds = 60;                  // Tăng lên 60s vì Gemini có thể chậm hơn
};
```

### 4. Cập nhật `run_eval.cpp` — đọc config + khởi tạo đúng client
**File:** `benchmark/run_eval.cpp`

```cpp
// Đọc config.json
std::ifstream config_file("config.json");
auto config = nlohmann::json::parse(config_file);

std::shared_ptr<oop_agent::LLMClient> client;
std::string provider = config.value("provider", "gemini");

if (provider == "gemini") {
    client = std::make_shared<oop_agent::GeminiClient>(
        config["api_key"].get<std::string>(),
        config.value("model", "gemini-2.5-flash")
    );
} else if (provider == "ollama") {
    client = std::make_shared<oop_agent::OllamaClient>(
        config.value("api_url", "http://localhost:11434"),
        config.value("model", "gemma4:e4b")
    );
}
// → Polymorphism! Phần còn lại không cần sửa gì
```

> [!TIP]
> Giữ lại `OllamaClient` trong codebase — không xóa! Điều này:
> - Thể hiện **OOP polymorphism** (cùng interface, nhiều implementation)
> - Giảng viên có thể yêu cầu demo đổi provider khi demo live tuần 13
> - Chỉ cần sửa `config.json` là chuyển được

### 5. `LoopDetector`
**File:** `src/agent/LoopDetector.h` + `LoopDetector.cpp`

Phát hiện 2 loại loop — **giữ nguyên** spec kế hoạch gốc:

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

### 6. Tích hợp `LoopDetector` vào `AgentLoop`
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

### 7. C++20 feature
Thêm ít nhất 1 C++20 feature vào codebase:
- **`std::span`** cho buffer trong `GeminiClient`
- **Concepts** để constraint template trong `Registry`
- **Ranges** để duyệt tool list

**Commit:** `[Week6-A] GeminiClient + LoopDetector + config.json + C++20`

---

## B — Tools / Data

### 1. ~~Hoàn thiện MemoryTool~~ ✅ Đã xong từ tuần 5

### 2. ~~Hoàn thiện 3 tool bổ sung~~ ✅ Đã xong (TimeTool, JsonTool, GitTool)

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

Cập nhật `AgentLoop::run()` để dùng `std::visit`:
```cpp
Action action = parse_llm_response(llm_text);

std::visit([&](auto&& act) {
    using T = std::decay_t<decltype(act)>;
    if constexpr (std::is_same_v<T, ToolCallAction>) {
        // Gọi tool...
    } else if constexpr (std::is_same_v<T, FinalAnswerAction>) {
        // Trả lời cuối...
    }
}, action);
```

> [!NOTE]
> Cái này cũng tick thêm 2 feature C++17: `std::variant` + `if constexpr` / `std::visit`

### 5. Smart pointer audit
Review toàn bộ tools:
- `unique_ptr` cho object chỉ có 1 owner
- `shared_ptr` cho object nhiều chỗ dùng
- Không dùng raw `new`/`delete`
- Không có memory leak

### 6. ⭐ Hỗ trợ A test GeminiClient
- Viết test case đơn giản: gọi Gemini → calculator → trả kết quả
- Verify rằng tất cả 8 tools vẫn hoạt động bình thường với Gemini response format
- Nếu Gemini trả JSON tool call hơi khác → điều chỉnh `parse_tool_call()` regex

**Commit:** `[Week6-B] Tool polish + std::variant + Gemini integration test`

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
  "model": "gemini-2.5-flash",
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

> [!IMPORTANT]
> Cập nhật `"model"` trong trajectory JSON từ `"gemma4:e4b"` → lấy từ `config.json` (hoặc truyền vào HarnessRunner).

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

> [!WARNING]
> Khi dùng Gemini API với multi-agent, cẩn thận **rate limit**! Google AI Studio free tier giới hạn RPM (requests per minute). Nên thêm sleep giữa các request hoặc dùng mutex để serialize API calls.

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

## 📋 Cập nhật CMakeLists.txt

Thêm `gemini_client.cpp` vào danh sách source:

```cmake
add_executable(OopAgent
    src/main.cpp
    src/agent/agent_loop.cpp
    src/agent/SkillLoader.cpp
    src/agent/LoopDetector.cpp          # MỚI
    src/client/ollama_client.cpp
    src/client/gemini_client.cpp        # MỚI
    src/tools/CalculatorTool.cpp
    # ... (giữ nguyên phần còn lại)
)
```

---

## Checkpoint cuối tuần

| | Verify | Người |
|---|---|---|
| `GeminiClient` | Gọi Gemini API thành công, nhận response | A |
| `config.json` | Đổi provider gemini↔ollama không cần recompile | A |
| `LoopDetector` | Agent lặp 5 lần → tự dừng | A |
| C++20 | Ít nhất 1 feature có trong code | A |
| `std::variant` | Action type dùng variant + std::visit | B |
| Tool polish | Mỗi tool có error handling đầy đủ | B |
| Tools + Gemini | 8 tools chạy đúng với Gemini response | B |
| Batch eval | Chạy 8 task, in success rate theo category | C |
| Trajectory | JSON output đúng format, model name = gemini | C |
| Multi-agent | Spawn 2 thread chạy song song không crash | C |
| Build | `cmake .. && make` không lỗi | Cả nhóm |
| Commit | Mỗi người ≥2 commit | Cả nhóm |

> [!WARNING]
> **Ưu tiên #1 tuần này:** Người A hoàn thành `GeminiClient` **trước** — vì B và C đều cần client hoạt động để test tools và chạy benchmark.
> Nếu A bị block về Gemini API format → dùng tạm `OllamaClient` để B/C không bị chờ.

> [!CAUTION]  
> **Bảo mật API Key:** KHÔNG commit API key lên GitHub! Dùng `config.json` và thêm nó vào `.gitignore`. Khi demo, nhập key thủ công.
