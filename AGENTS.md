# AGENTS.md

## Tổng quan dự án

Đây là dự án **C++ AI Agent** xây dựng theo mô hình OOP, triển khai vòng lặp ReAct agent (Reason + Act) có thể gọi nhiều công cụ (tools), đánh giá kết quả tự động qua harness, và hỗ trợ chạy nhiều agent song song.

Dự án chia làm ba nhóm sở hữu chính:

| Nhóm | Phạm vi |
|------|---------|
| **Role A — systems/core** | LLM clients, `AgentLoop`, parsing, skills, loop detection |
| **Role B — tools/data** | Tool interfaces, registry, aliases, argument parsing, tool implementations |
| **Role C — evaluation/infra** | Harness, evaluators, benchmark tasks/results, trajectories, multi-agent demos |

---

## Cấu trúc dự án

```
OOP_AI-AGENT/
├── CMakeLists.txt              # Build system (CMake ≥ 3.28)
├── config.json                 # Cấu hình API key & model (KHÔNG commit)
├── config.json.example         # Template cấu hình mẫu
├── hello.sh                    # Script test đơn giản dùng trong benchmark
├── AGENTS.md                   # File này
├── README.md                   # Tài liệu tổng quan
├── .clangd                     # Cấu hình clangd / IDE (dùng -std=c++2c)
│
├── src/                        # Toàn bộ source code chính
│   ├── main.cpp                # Entry point của OopAgent (test GeminiClient)
│   │
│   ├── agent/                  # Role A – core agent logic
│   │   ├── agent_loop.h/.cpp   # AgentLoop: vòng lặp ReAct (Template Method)
│   │   ├── LoopDetector.h/.cpp # Phát hiện vòng lặp lặp lại của agent
│   │   ├── SkillLoader.h/.cpp  # Load file skill .md vào system prompt
│   │   └── Trajectory.h        # Cấu trúc dữ liệu Step & Trajectory
│   │
│   ├── client/                 # Role A – LLM client abstraction
│   │   ├── llm_client.h        # Abstract LLMClient + Message + LLMConfig
│   │   ├── gemini_client.h/.cpp# GeminiClient: gọi Gemini API qua CURL
│   │   └── ollama_client.h/.cpp# OllamaClient: gọi Ollama local qua CURL
│   │
│   ├── tools/                  # Role B – tool framework
│   │   ├── Tool.h              # Abstract Tool interface (Strategy Pattern)
│   │   ├── Registry.h          # Generic Registry<T> template
│   │   ├── ToolRegistry.h/.cpp # ToolRegistry: factory + registry + alias + policy
│   │   ├── CalculatorTool.h/.cpp
│   │   ├── ExecTool.h/.cpp     # Thực thi lệnh shell
│   │   ├── FileTool.h/.cpp     # Đọc/ghi/thêm nội dung file
│   │   ├── GitTool.h/.cpp
│   │   ├── JsonTool.h/.cpp
│   │   ├── MemoryTool.h/.cpp   # Lưu trữ bộ nhớ dùng SQLite3
│   │   ├── TimeTool.h/.cpp
│   │   └── WebSearchTool.h/.cpp
│   │
│   ├── harness/                # Role C – evaluation harness
│   │   ├── Task.h              # Struct Task & TaskList (parse từ tasks.json)
│   │   ├── evaluator.h         # Abstract Evaluator interface (Strategy Pattern)
│   │   ├── HarnessRunner.h/.cpp# Điều phối benchmark: load → run → evaluate → export
│   │   ├── KeywordEvaluator.h/.cpp
│   │   ├── FunctionalEvaluator.h/.cpp
│   │   └── VLMEvaluator.h/.cpp
│   │
│   ├── environment/            # Role C – môi trường file system
│   │   ├── Environment.h       # Abstract Environment interface
│   │   ├── NativeEnvironment.h/.cpp  # File I/O thật trên hệ thống
│   │   └── SandboxEnvironment.h      # Môi trường sandbox (stub/mock)
│   │
│   └── multiagent/             # Role C – multi-agent framework
│       ├── Message.h           # AgentMessage struct (giao tiếp giữa agents)
│       ├── MessageQueue.h      # Thread-safe message queue
│       └── MultiAgentRunner.h/.cpp  # Điều phối nhiều agent song song (std::thread)
│
├── benchmark/                  # Role C – benchmark & evaluation
│   ├── tasks.json              # *** Nguồn sự thật benchmark (KHÔNG chỉnh sửa tùy tiện) ***
│   ├── run_eval.cpp            # Executable chạy toàn bộ benchmark với LLM thật
│   ├── test_harness.cpp        # Unit/integration test cho HarnessRunner
│   ├── test_multi_agent.cpp    # Smoke test cho MultiAgentRunner
│   ├── demo_multi_agent.cpp    # Demo minh họa multi-agent workflow
│   └── results/                # Thư mục xuất kết quả JSON sau mỗi lần chạy eval
│
├── skills/                     # Skill files inject vào system prompt agent
│   ├── task_planner.md
│   ├── step_verifier.md
│   └── error_recovery.md
│
├── include/                    # Vendored headers (chỉ đọc)
│   └── nlohmann/               # nlohmann/json.hpp (KHÔNG chỉnh sửa)
│
├── docs/                       # Tài liệu bổ sung
└── filephanchiacv/             # Kế hoạch và lịch sử phân tích lỗi theo tuần
    ├── LICH_SU_LOI_THEO_TUAN_ROLE_ABC.md   # Lịch sử lỗi benchmark theo tuần
    ├── KH_TUAN9_ChiTiet.md                 # Kế hoạch tuần mới nhất
    └── ...                                 # Các file kế hoạch/phân tích khác
```

---

## Cách build và chạy

### Yêu cầu hệ thống

- **OS**: Linux / WSL (Windows Subsystem for Linux)
- **Compiler**: GCC hoặc Clang có hỗ trợ `-std=c++26`; MSVC dùng `/std:c++latest`
- **CMake**: ≥ 3.28
- **Thư viện bắt buộc**: `libcurl-dev`, `libsqlite3-dev`, `nlohmann-json` (hoặc dùng vendored `include/`)

### Cài đặt dependencies (Ubuntu/Debian)

```bash
sudo apt-get install -y libcurl4-openssl-dev libsqlite3-dev cmake build-essential
```

### Build

```bash
# Cấu hình
cmake -S . -B build

# Build toàn bộ targets
cmake --build build -j$(nproc)
```

Sau khi build thành công sẽ có các executables:

| Executable | Mục đích |
|-----------|---------|
| `./build/OopAgent` | Agent chính, hiện test GeminiClient |
| `./build/run_eval` | Chạy toàn bộ benchmark (gọi LLM thật) |
| `./build/test_harness` | Test HarnessRunner + Evaluators |
| `./build/test_multi_agent` | Smoke test MultiAgentRunner |
| `./build/demo_multi_agent` | Demo workflow multi-agent |

### Cấu hình API key

```bash
# Sao chép template
cp config.json.example config.json

# Chỉnh sửa config.json với API key thật
{
  "provider": "gemini",
  "api_key": "YOUR_GEMINI_API_KEY",
  "model": "gemma-4-31b-it",
  "api_url": "https://generativelanguage.googleapis.com/v1beta",
  "use_mock": false
}
```

> **CANH BAO**: Không bao giờ commit `config.json`. File này chứa API key bí mật.

### Chạy agent

```bash
# Chạy từ thư mục gốc (để agent tìm đúng config.json và skills/)
./build/OopAgent
```

### Chạy benchmark (cần xác nhận trước — tốn API quota)

```bash
./build/run_eval
```

> **Lưu ý**: `run_eval` gọi LLM thật và sinh artifact. Cần xác nhận của user trước khi chạy để tránh tốn quota/chi phí.

---

## Cách chạy test

### Chạy toàn bộ test suite qua CTest

```bash
cd build
ctest --output-on-failure
```

### Chạy từng test thủ công

```bash
# Test HarnessRunner, evaluators, task parsing
./build/test_harness

# Smoke test MultiAgentRunner
./build/test_multi_agent
```

### Trình tự kiểm tra sau khi thay đổi một component

1. Build tất cả targets: `cmake --build build -j$(nproc)`
2. Chạy test nhỏ nhất liên quan trước: `./build/test_harness` hoặc `./build/test_multi_agent`
3. Chạy `./build/run_eval` chỉ khi cần kiểm chứng benchmark thật (và sau khi xác nhận với user)

> **Quy tắc benchmark**: Một lần chạy benchmark thành công phải đến từ một lần chạy sạch hiện tại — không dùng file artifact cũ trong thư mục gốc để báo cáo kết quả.

---

## Design Patterns trong dự án

| Pattern | Nơi áp dụng |
|---------|------------|
| **Template Method** | `AgentLoop::run()` — skeleton cố định, subclass override các primitive operations |
| **Strategy** | `Evaluator`, `Tool`, `LLMClient` — chọn implementation lúc runtime |
| **Registry/Factory** | `ToolRegistry`: `register_creator()` + `create()` (Factory) và `register_tool()` + `lookup()` (Registry) |
| **Observer/Hook** | `StepHook` — callback inject vào `AgentLoop`, Harness dùng để ghi trajectory mà không cần AgentLoop biết Harness tồn tại |
| **RAII / Smart pointers** | Toàn bộ ownership qua `std::unique_ptr` / `std::shared_ptr` |

---

## Coding conventions

### Namespace

- Toàn bộ code nằm trong namespace `oop_agent`.
- File header dùng `#pragma once` thay vì include guards.

### Naming

| Loại | Convention | Ví dụ |
|------|-----------|-------|
| Class/Struct | PascalCase | `AgentLoop`, `ToolRegistry` |
| Method | snake_case | `register_tool()`, `get_name()` |
| Member variable | `snake_case_` (trailing underscore) | `llm_`, `registry_`, `step_hook_` |
| Enum | PascalCase, values PascalCase | `ToolError::InvalidArgument` |
| File | Khớp tên class | `AgentLoop` → `agent_loop.h` / `agent_loop.cpp` |

### C++ version và tính năng

- Build chính dùng **C++26** (`-std=c++26` trên GCC/Clang, `/std:c++latest` trên MSVC).
- Dùng `std::expected<T, E>` (C++23) cho error handling — **không dùng exceptions**.
- Dùng `std::string_view` cho tham số không cần ownership.
- Dùng `[[nodiscard]]` cho getter và pure query methods.
- Không dùng raw owning `new`/`delete` — luôn dùng `std::make_unique` / `std::make_shared`.

### Error handling

```cpp
// Đúng: dùng std::expected
std::expected<std::string, ToolError> execute(const std::string& args);

// Đúng: trả về ToolError cụ thể
return std::unexpected(ToolError::InvalidArgument);

// Sai: ném exception hoặc dùng error code trần
```

### Tách biệt các layer

- `AgentLoop` **không** `#include` bất cứ thứ gì từ `harness/` — coupling duy nhất là qua `StepHook`.
- `Tool` implementations **không** phụ thuộc `AgentLoop` — chỉ nhận `arguments` và trả về kết quả.
- `Evaluator` implementations **không** biết internal của agent — chỉ so sánh output vs expected.
- Đăng ký tool trong `ToolRegistry` (`register_all_tools()`), **không** hardcode vào `AgentLoop`.

### Comments

- Comment bằng tiếng Anh hoặc tiếng Việt đều được — dự án đang dùng song ngữ.
- Dùng Doxygen-style (`/** @brief ... */`) cho public API trong header.
- Comment inline dùng `//` cho logic phức tạp.

---

## Các file và thư mục quan trọng

| File/Thư mục | Vai trò |
|-------------|---------|
| `benchmark/tasks.json` | Nguồn sự thật benchmark — KHÔNG làm yếu post-conditions để tăng điểm |
| `src/agent/agent_loop.h` | Interface AgentLoop (Template Method skeleton) |
| `src/client/llm_client.h` | Abstract LLMClient + Message + LLMConfig |
| `src/tools/Tool.h` | Abstract Tool interface |
| `src/tools/ToolRegistry.h` | ToolRegistry: Factory + Registry + Alias + Policy |
| `src/harness/evaluator.h` | Abstract Evaluator interface |
| `src/harness/HarnessRunner.h` | Benchmark orchestrator |
| `src/harness/Task.h` | Task struct parse từ tasks.json |
| `src/environment/Environment.h` | Abstract Environment (file I/O) |
| `src/agent/LoopDetector.h` | Phát hiện agent lặp vòng |
| `src/agent/SkillLoader.h` | Load skill .md vào system prompt |
| `src/tools/Registry.h` | Generic Registry<T> template |
| `config.json` | API key + model config (secret — không commit) |
| `skills/` | Skill files (.md) inject vào agent |
| `filephanchiacv/` | Lịch sử lỗi benchmark & kế hoạch theo tuần |
| `include/nlohmann/` | Vendored nlohmann/json — KHÔNG chỉnh sửa |

---

## Lưu ý khi chỉnh sửa code

### Trước khi chỉnh sửa

1. Đọc kế hoạch Role và file phân tích run mới nhất trong `filephanchiacv/` (tìm file `PHAN_TICH_RUN_*.md` hoặc `KH_TUAN*.md` mới nhất) trước khi thay đổi code thuộc một Role.
2. Giữ phạm vi thay đổi trong Role/task được yêu cầu — bảo toàn mọi chỉnh sửa không liên quan của user trong worktree.

### Ràng buộc kiến trúc

- Bảo tồn các abstract interface: `LLMClient`, `Tool`, `Evaluator`.
- Không hardcode tool implementation vào `AgentLoop` — luôn qua `ToolRegistry`.
- Tách biệt `AgentLoop` khỏi `HarnessRunner`, tools khỏi agent loop, evaluators khỏi agent internals.
- Bảo tồn các design pattern: Strategy, Registry/Factory, Observer/Hook, Template Method.
- Dùng RAII và smart pointers — không dùng raw owning `new`/`delete`.
- Duy trì đường dẫn tính năng C++26 có guard và fallback portability.

### Benchmark correctness

- Không làm yếu task post-conditions hoặc evaluators chỉ để tăng điểm.
- Task `requires_tool: true` phải có ít nhất một tool step thực sự chạy thành công.
- Task sinh file (artifacts) phải verify đúng filename và content được tạo trong lần chạy hiện tại.
- Dọn dẹp artifact (`notes.txt`, `result.txt`, `capital.txt`, `calc.txt`, `data.txt`, `output.txt`, ...) trước khi eval để tránh false pass từ file cũ.
- Lưu args thật trong trajectory — không báo `args` rỗng khi arguments đã được truyền.
- Dùng lý do lỗi cụ thể: `invalid arguments`, `missing artifacts`, `content mismatch`, `loop detection`, `incomplete task`.

### Thêm Tool mới (Role B)

1. Tạo `src/tools/MyTool.h` và `MyTool.cpp` kế thừa `Tool`.
2. Implement `get_name()`, `get_description()`, `execute()`.
3. Đăng ký trong `ToolRegistry::register_all_tools()`.
4. Thêm file vào danh sách source trong `CMakeLists.txt` (target `OopAgent`).

### Thêm Evaluator mới (Role C)

1. Tạo `src/harness/MyEvaluator.h` và `MyEvaluator.cpp` kế thừa `Evaluator`.
2. Implement `get_name()` và `evaluate()`.
3. Đăng ký trong `HarnessRunner` (trong `run_eval.cpp` hoặc `test_harness.cpp`).

### Thêm Skill mới

1. Tạo file `.md` trong `skills/` với nội dung hướng dẫn cho agent.
2. `SkillLoader::loadAll()` sẽ tự động scan và inject vào system prompt.

### Bảo mật

- `config.json` là file bí mật — không in, log, hoặc commit API key.
- `ExecTool` phải giữ chính sách shell restrictive và trả về `ToolError` rõ ràng cho các lệnh bị từ chối.
- Không dùng mock để báo cáo success rate thật của provider thật.

### Không chỉnh sửa

- `include/nlohmann/` — vendored header, chỉ đọc.
- `benchmark/tasks.json` — nguồn sự thật, chỉ thay đổi khi có yêu cầu rõ ràng về task spec.
- `config.json` — secret file, không commit.
