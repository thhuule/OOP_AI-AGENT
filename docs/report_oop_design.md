# Báo cáo Thiết kế OOP — AI-Agent OOP 2026

> **Vai trò:** A — Systems/Core  
> **Tuần:** 9  
> **Nguyên tắc:** Mỗi nhận định có class/file minh chứng. Không gắn pattern sai. Khoảng cách thực tế được ghi rõ trong §6.

---

## 1. Tổng quan kiến trúc

Hệ thống được chia thành năm layer có dependency một chiều:

```
EntryPoints → AgentCore → ToolsLayer
                       ↕ (qua StepHook callback)
             HarnessLayer → EnvironmentLayer
             MultiAgentLayer → AgentCore
```

Dependency bị cấm và chưa vi phạm:
- `AgentLoop` không import bất kỳ header nào của `src/harness/`.
- Concrete tool không tham chiếu `AgentLoop`.
- `Evaluator` không đọc state nội bộ của `AgentLoop`.

---

## 2. Design Patterns bắt buộc

### 2.1 Strategy — Evaluator Selection

**Định nghĩa:** định nghĩa family of algorithms, encapsulate từng cái, và cho phép thay thế lẫn nhau.

**Interface:** `src/harness/evaluator.h`

```cpp
class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual double evaluate(const std::string& output,
                            const std::string& expected,
                            const Task& task) = 0;
};
```

**Concrete strategies:**

| Class | File | Thuật toán |
|---|---|---|
| `KeywordEvaluator` | `src/harness/KeywordEvaluator.cpp` | Kiểm tra keyword xuất hiện trong output |
| `FunctionalEvaluator` | `src/harness/FunctionalEvaluator.cpp` | Chạy `eval_script` và kiểm tra exit code |
| `VLMEvaluator` | `src/harness/VLMEvaluator.cpp` | *(skeleton — chưa hoàn chỉnh)* |

**Context — HarnessRunner chọn strategy:**

```cpp
// src/harness/HarnessRunner.cpp
Evaluator* HarnessRunner::find_evaluator(const std::string& eval_type) {
    for (auto& ev : evaluators_) {
        if (/* eval_type matches ev */) return ev.get();
    }
    return nullptr;
}
```

**Chứng minh pattern đúng:** `HarnessRunner::run_all()` gọi `evaluator->evaluate(...)` qua con trỏ abstract, không switch-case theo type. Thêm evaluator mới chỉ cần thêm subclass và đăng ký — flow không đổi.

**Test yêu cầu:** `src/tests/test_strategy_evaluator.cpp`
- Đăng ký `KeywordEvaluator` và `FunctionalEvaluator`.
- Chọn đúng evaluator theo `eval_type`.
- Thay evaluator không làm thay đổi cách `HarnessRunner` gọi.

---

### 2.2 Template Method — AgentLoop::run()

**Định nghĩa:** định nghĩa skeleton của thuật toán trong một method, trì hoãn một số bước cho subclass.

**Trạng thái đầu Tuần 9:** `AgentLoop::run()` là một hàm nguyên khối, chưa có primitive operations virtual.

**Refactor bắt buộc (A thực hiện trong Tuần 9):**

```cpp
// src/agent/agent_loop.h  — sau refactor
class AgentLoop {
public:
    // Skeleton algorithm — không được override
    std::string run(const std::string& instruction, int max_steps);

protected:
    // Primitive operations — subclass có thể override
    virtual std::string build_system_prompt(const std::string& instruction);
    virtual std::variant<ToolCallAction, FinalAnswerAction>
        think_and_act(int step);
    virtual std::expected<std::string, ToolError>
        execute_tool(const ToolCallAction& action);
    virtual void observe(const std::string& result);
    virtual void on_loop_detected();
    virtual void on_max_steps_reached();
};
```

**Skeleton `run()` không thay đổi theo subclass:**

```cpp
std::string AgentLoop::run(const std::string& instruction, int max_steps) {
    history_.push_back({role_system, build_system_prompt(instruction)});
    history_.push_back({role_user, instruction});
    for (int step = 1; step <= max_steps; ++step) {
        auto action = think_and_act(step);           // overrideable
        if (auto* fa = std::get_if<FinalAnswerAction>(&action)) {
            return fa->answer;
        }
        auto& tc = std::get<ToolCallAction>(action);
        auto result = execute_tool(tc);              // overrideable
        if (!result) {
            observe("TOOL_ERROR: " + result.error()); // overrideable
        } else {
            observe(*result);                         // overrideable
        }
        if (detector_.is_loop_detected()) {
            on_loop_detected();                       // overrideable
            return "Loop detected";
        }
    }
    on_max_steps_reached();                           // overrideable
    return "Max steps reached";
}
```

**Test yêu cầu:** `src/tests/test_template_method.cpp`

```cpp
class MockAgentLoop : public AgentLoop {
protected:
    std::variant<ToolCallAction, FinalAnswerAction>
    think_and_act(int step) override {
        if (step == 1) return ToolCallAction{"calculator", "1+1"};
        return FinalAnswerAction{"done"};
    }
};
// Kiểm tra: MockAgentLoop::run() đi qua đúng thứ tự Observe→Think→Act
// và trả "done" mà không cần viết lại skeleton.
```

---

### 2.3 Registry/Factory — ToolRegistry

**Định nghĩa:** Registry lưu creator functions theo tên; Factory tạo `unique_ptr<Tool>` theo tên được tra cứu.

**Trạng thái đầu Tuần 9:** `ToolRegistry` đăng ký và lookup instance — đây là **Registry**, chưa phải Factory tạo instance mới theo tên.

**Thiết kế đầy đủ (cần hoàn tất Tuần 9):**

```cpp
// src/tools/ToolRegistry.h
using ToolCreator = std::function<std::unique_ptr<Tool>()>;

class ToolRegistry {
public:
    // Factory: đăng ký creator function theo tên
    void register_creator(const std::string& name, ToolCreator creator);

    // Factory: tạo tool mới theo tên
    std::unique_ptr<Tool> create(const std::string& name);

    // Registry: lookup instance đã được tạo sẵn
    Tool* lookup(const std::string& name);

    // Alias và policy
    void register_alias(const std::string& alias, const std::string& canonical);
    std::string normalize(const std::string& name) const;
    bool is_allowed(const std::string& name) const;

private:
    std::map<std::string, ToolCreator>          creators_;
    std::map<std::string, std::unique_ptr<Tool>> instances_;
    std::map<std::string, std::string>           aliases_;
    std::set<std::string>                        deny_list_;
};
```

**Đăng ký (không hardcode trong AgentLoop):**

```cpp
// src/tools/ToolRegistry.cpp
void ToolRegistry::register_all_tools() {
    register_creator("calculator",     []{ return std::make_unique<CalculatorTool>(); });
    register_creator("file",           []{ return std::make_unique<FileTool>(); });
    register_creator("execute_shell",  []{ return std::make_unique<ExecTool>(); });
    register_creator("web_search",     []{ return std::make_unique<WebSearchTool>(); });
    register_creator("memory",         []{ return std::make_unique<MemoryTool>(); });
    register_creator("time",           []{ return std::make_unique<TimeTool>(); });
    register_creator("json",           []{ return std::make_unique<JsonTool>(); });
    register_creator("git",            []{ return std::make_unique<GitTool>(); });
}
```

**Alias mapping (normalize trước lookup):**

```
calculate     → calculator
exec          → execute_shell
google_search → web_search
create_file   → write_file
```

**Test yêu cầu:** `src/tests/test_registry_factory.cpp`
- Đăng ký creator → `create("calculator")` trả `unique_ptr<CalculatorTool>`.
- Tên không tồn tại → trả `nullptr` với thông báo rõ ràng.
- Đăng ký duplicate → behavior được định nghĩa (overwrite hoặc error).
- Alias `calculate` → `create("calculate")` trả `CalculatorTool`.
- Deny-list → `is_allowed` trả false, `create` bị chặn.
- Ownership: `unique_ptr` không bị leak (chạy với AddressSanitizer).

---

### 2.4 Observer/Hook — StepHook

**Định nghĩa:** Observer pattern qua callback function. Subject (`AgentLoop`) thông báo sau mỗi bước mà không biết observer là ai.

**Interface:**

```cpp
// src/agent/agent_loop.h
using StepHook = std::function<void(const TrajectoryStep&)>;

class AgentLoop {
public:
    void set_step_hook(StepHook hook) { step_hook_ = std::move(hook); }
private:
    StepHook step_hook_;
};
```

**Subject thông báo (trong `run()`):**

```cpp
if (step_hook_) {
    step_hook_(TrajectoryStep{step, thought, tool_name, args, result, success, latency_ms, tokens});
}
```

**Observer — HarnessRunner:**

```cpp
// src/harness/HarnessRunner.cpp
std::vector<TrajectoryStep> trajectory;
agent_->set_step_hook([&trajectory](const TrajectoryStep& s) {
    trajectory.push_back(s);
});
```

**Chứng minh pattern đúng:**
- `AgentLoop` không `#include` bất kỳ header nào của `src/harness/`.
- `HarnessRunner` không cần subclass `AgentLoop`.
- Nhiều observer có thể được chain bằng cách wrap `StepHook`.

**Test yêu cầu:** `src/tests/test_observer_hook.cpp`
- Gắn hook vào `AgentLoop` mock.
- Sau `run()`, kiểm tra trajectory có đúng số step, thought, tool, result.
- Không gắn hook → không crash.
- Harness header không bị include trong `agent_loop.h`.

---

## 3. OOP Nguyên tắc và SOLID

### 3.1 Inheritance và Abstract Interface

| Abstract class | Subclasses | File |
|---|---|---|
| `LLMClient` | `OllamaClient`, `GeminiClient` | `src/client/llm_client.h` |
| `Tool` | 8 concrete tools | `src/tools/Tool.h` |
| `Evaluator` | `KeywordEvaluator`, `FunctionalEvaluator`, `VLMEvaluator` | `src/harness/evaluator.h` |
| `Environment` | `NativeEnvironment`, `SandboxEnvironment` | `src/environment/Environment.h` |

Tất cả abstract class đều dùng pure virtual và virtual destructor — đúng C++ convention.

### 3.2 Composition và Ownership

| Owner | Sở hữu | Kiểu |
|---|---|---|
| `AgentLoop` | `ToolRegistry` | value member |
| `AgentLoop` | `LoopDetector` | value member |
| `AgentLoop` | `LLMClient` | `shared_ptr` |
| `ToolRegistry` | `unique_ptr<Tool>` | `map<string, unique_ptr<Tool>>` |
| `HarnessRunner` | `unique_ptr<Evaluator>` | `vector<unique_ptr<Evaluator>>` |
| `MultiAgentRunner` | `MessageQueue` | value member |

Không có raw owning pointer. Lifetime rõ ràng qua smart pointer.

### 3.3 Dependency Inversion

`HarnessRunner` phụ thuộc `Environment*` (abstract), không phụ thuộc `NativeEnvironment` hay `SandboxEnvironment`. Caller inject implementation khi khởi tạo.

```cpp
// Đúng DIP:
HarnessRunner runner(agent, std::make_unique<SandboxEnvironment>("/tmp/bench"));
// Không phụ thuộc chi tiết sandbox.
```

### 3.4 Single Responsibility

| Class | Trách nhiệm duy nhất |
|---|---|
| `AgentLoop` | Điều phối vòng lặp ReAct |
| `SkillLoader` | Đọc và chọn skill |
| `LoopDetector` | Phát hiện lặp |
| `ToolRegistry` | Quản lý đăng ký và tra cứu tool |
| `HarnessRunner` | Điều phối batch evaluation |
| `FileTool` | Đọc/ghi/nối file |

### 3.5 Open/Closed

Thêm tool mới: tạo subclass `Tool`, đăng ký creator — không sửa `AgentLoop`.  
Thêm evaluator mới: tạo subclass `Evaluator`, đăng ký — không sửa `HarnessRunner`.

---

## 4. Adapter Pattern (bổ sung)

`SharedToolWrapper` trong `AgentLoop` chuyển `shared_ptr<Tool>` sang `Tool*` interface mà `ToolRegistry` quản lý. Pattern này cho phép một tool được chia sẻ giữa nhiều registry hoặc agent instance.

Chứng minh: `src/agent/agent_loop.cpp` có `SharedToolWrapper` nếu còn tồn tại; nếu đã bị gộp vào `ToolRegistry`, ghi rõ không còn cần wrapper riêng.

---

## 5. Ma trận kỹ thuật C++

### C++17 (≥ 4 kỹ thuật)

| Kỹ thuật | Vị trí | Mục đích | Fallback |
|---|---|---|---|
| `std::filesystem` | `NativeEnvironment.cpp`, `SandboxEnvironment.cpp`, `src/tools/FileTool.cpp` | Thao tác file/directory portable | Không cần — C++17 bắt buộc |
| `std::optional` | `src/agent/agent_loop.h`, `src/harness/HarnessRunner.h` | Tham số tùy chọn, return type có thể rỗng | `bool + T out-param` |
| `std::variant` + `std::visit` | `src/agent/agent_loop.cpp` — `parse_llm_response` trả `variant<ToolCallAction, FinalAnswerAction>` | Discriminated union an toàn kiểu | Polymorphic hierarchy |
| `std::function` + lambda | `AgentLoop::step_hook_`, `ToolRegistry::creators_` | First-class callable, Strategy và Observer | `std::function` C++11 |
| Smart pointer (`unique_ptr`, `shared_ptr`) | Toàn bộ codebase | RAII ownership | Manual delete |
| Pure virtual / abstract class | `LLMClient`, `Tool`, `Evaluator`, `Environment` | Interface definition | — |

### C++20 (≥ 2 kỹ thuật độc lập)

| Kỹ thuật | Vị trí | Mục đích | Fallback |
|---|---|---|---|
| `std::ranges::find_if` | `src/harness/HarnessRunner.cpp` — `find_evaluator()` | Tìm evaluator theo predicate, rõ ràng hơn iterator | `std::find_if` C++11 |
| `std::ranges::any_of` | `src/agent/LoopDetector.cpp` — phát hiện ping-pong | Kiểm tra lịch sử action | `std::any_of` C++11 |

> **Lưu ý:** `std::string_view` đã có từ C++17, không được tính là kỹ thuật C++20.

### C++23 (≥ 2 kỹ thuật)

| Kỹ thuật | Vị trí | Mục đích | Fallback |
|---|---|---|---|
| `std::expected<T, E>` | `Environment.h`, `src/tools/Tool.h`, `src/client/llm_client.h` | Error propagation không dùng exception | `pair<optional<T>, string>` |
| `std::print` / `std::println` | `src/main.cpp`, `benchmark/run_eval.cpp` | Output định dạng rõ ràng | `printf` / `std::cout` |

### C++26 (≥ 1 kỹ thuật)

| Kỹ thuật | Vị trí | Mục đích | Portability |
|---|---|---|---|
| `std::inplace_vector<TrajectoryStep, N>` | `src/agent/agent_loop.cpp` — buffer history trong `run()` | Fixed-capacity vector không allocate heap | Guarded: `#if __cpp_lib_inplace_vector >= 202406L` → fallback `std::vector` |

```cpp
// Guarded C++26 usage
#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L
    #include <inplace_vector>
    using StepBuffer = std::inplace_vector<TrajectoryStep, 64>;
#else
    using StepBuffer = std::vector<TrajectoryStep>;
#endif
```

### Compiler flags

```cmake
# CMakeLists.txt
target_compile_features(OopAgent    PRIVATE cxx_std_26)
target_compile_features(run_eval    PRIVATE cxx_std_26)
target_compile_features(test_multi_agent  PRIVATE cxx_std_26)
target_compile_features(demo_multi_agent  PRIVATE cxx_std_26)
# MSVC: add /std:c++latest cho MỌI target, không chỉ OopAgent
```

---

## 6. Khoảng cách phải đóng trước khi kết thúc Tuần 9

> Liệt kê theo thứ tự ưu tiên. Đây là **blocker bắt buộc**, không phải future work.

| # | Khoảng cách | Hành động | Owner | File liên quan |
|---|---|---|---|---|
| 1 | `AgentLoop::run()` chưa là Template Method đúng nghĩa | Refactor `run()` thành skeleton + protected virtual primitive operations | A | `src/agent/agent_loop.h`, `.cpp` |
| 2 | `ToolRegistry` mới là Registry, chưa có Factory tạo instance theo tên | Thêm `register_creator()` và `create()` với `unique_ptr` | B | `src/tools/ToolRegistry.h`, `.cpp` |
| 3 | `Environment` hierarchy chưa tồn tại trong source | Tạo `src/environment/` với 3 file đã nêu | A | *(đã tạo Tuần 9)* |
| 4 | `LLMConfig` chưa có trường `max_tokens` | Thêm field, truyền vào request Ollama/Gemini | A | `src/client/llm_client.h`, client `.cpp` |
| 5 | `src/tests/` chưa có unit-test executable | Tạo test file cho mỗi pattern và compliance blocker | A/B | `src/tests/` |
| 6 | MSVC chưa áp `/std:c++latest` cho `test_multi_agent`/`demo_multi_agent` | Cập nhật `CMakeLists.txt` | A | `CMakeLists.txt` |
| 7 | C++20: mới có `std::ranges` ở một nơi | Thêm một kỹ thuật C++20 độc lập thứ hai | A | `src/agent/LoopDetector.cpp` |

---

## 7. Giới hạn trung thực

- `VLMEvaluator` là skeleton — chưa chấm ảnh; ghi đúng là evaluator thị giác chưa hoàn chỉnh.
- `token` trong `TrajectoryStep` luôn bằng `0` — giới hạn đo lường phía client, không phải model không dùng token.
- Kết quả 10/10 (run `220549_361`) là lịch sử; cần run sạch trước báo cáo cuối.
- Multi-agent là tính năng mở rộng/demo, không phải benchmark đơn-agent.
- Multimodal qua `Message::images` mới được serialize đúng trên Ollama client; Gemini client chưa gửi ảnh — không suy rộng khả năng.

---

## 8. Tài liệu tham khảo

- Gamma, Helm, Johnson, Vlissides. *Design Patterns: Elements of Reusable Object-Oriented Software.* Addison-Wesley, 1994.
- cppreference.com — `std::expected` (C++23), `std::inplace_vector` (C++26), `std::ranges` (C++20).
- Tài liệu đề bài: `filephanchiacv/OOP Project 2026 AI Agent.docx (1).md`.
- Kết quả benchmark: `benchmark/results/run_20260801_220549_361/`.