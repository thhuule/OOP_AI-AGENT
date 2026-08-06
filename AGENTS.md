<<<<<<< HEAD
# AGENTS.md — Hướng Dẫn Phát Triển & Quy Chuẩn Dự Án OOP AI Agent

## 1. Tổng Quan & Phân Chia Trách Nhiệm (Role Ownership)

Dự án **OopAiAgent** là một hệ thống AI Agent đa tác vụ phát triển bằng C++26/C++23 theo hướng đối tượng (OOP). Dự án được phân chia thành 3 vùng kiến trúc chính (Role Ownership):

- **Role A — Systems/Core**: Quản lý vòng lặp agent (`AgentLoop`), tích hợp LLM Clients (`LLMClient`, `GeminiClient`, `OllamaClient`), nạp kịch bản/skill (`SkillLoader`), phát hiện vòng lặp vô tận (`LoopDetector`), và theo dõi lịch sử thực thi (`Trajectory`).
- **Role B — Tools/Data**: Định nghĩa giao diện công cụ trừu tượng (`Tool`), đăng ký và phân giải công cụ (`ToolRegistry`, `Registry<T>`), xử lý tham số, và hiện thực các công cụ hệ thống (`CalculatorTool`, `ExecTool`, `FileTool`, `WebSearchTool`, `MemoryTool`, `TimeTool`, `JsonTool`, `GitTool`).
- **Role C — Evaluation/Infra**: Khung đánh giá benchmark (`HarnessRunner`, `Task`), các bộ kiểm thử kết quả (`Evaluator`, `KeywordEvaluator`, `FunctionalEvaluator`, `VLMEvaluator`), giao tiếp đa Agent (`MultiAgentRunner`, `MessageQueue`), và lưu trữ kết quả benchmark (`benchmark/tasks.json`, `benchmark/results/`).

> **Lưu ý**: Nguồn dữ liệu benchmark chuẩn (Source of Truth) nằm tại `benchmark/tasks.json`. Lịch sử đánh giá và kế hoạch làm việc của nhóm nằm trong `filephanchiacv/`; đặc biệt, hãy tham khảo file `PHAN_TICH_RUN_*.md` mới nhất trước khi thay đổi logic cho tác vụ bị thất bại.

---

## 2. Cấu Trúc Dự Án (Project Structure)

```text
.
├── CMakeLists.txt              # Script cấu hình biên dịch CMake toàn hệ thống
├── AGENTS.md                   # Quy chuẩn phát triển, quy định kiến trúc & hướng dẫn làm việc
├── README.md                   # Tài liệu giới thiệu tổng quan dự án
├── config.json.example         # File mẫu cấu hình API key (Copy thành config.json)
├── benchmark/                  # Dữ liệu & chương trình đánh giá benchmark
│   ├── tasks.json              # Source-of-truth chứa danh sách tác vụ kiểm thử benchmark
│   ├── run_eval.cpp            # Chạy đánh giá toàn bộ benchmark tasks với LLM thực tế
│   ├── test_harness.cpp        # Integration test cho HarnessRunner & Evaluator
│   ├── test_multi_agent.cpp    # Unit test cho hệ thống Multi-Agent Threading
│   ├── demo_multi_agent.cpp    # Chương trình demo luồng tương tác giữa nhiều Agent
│   └── results/                # Nơi chứa kết quả & trajectory xuất ra từ run_eval
├── docs/                       # Sơ đồ kiến trúc UML & báo cáo thiết kế (Class, Sequence, Component)
├── filephanchiacv/             # Kế hoạch công việc theo tuần & báo cáo phân tích lỗi (PHAN_TICH_RUN_*.md)
├── include/                    # Thư viện bên ngoài được bao đóng (nlohmann/json.hpp)
├── skills/                     # Các file định nghĩa Skill dạng Markdown cho Agent (task_planner, error_recovery, step_verifier)
├── src/                        # Mã nguồn C++ chính của dự án
│   ├── main.cpp                # Điểm khởi chạy CLI chính của OopAgent
│   ├── agent/                  # Role A: AgentLoop, SkillLoader, LoopDetector, Trajectory
│   ├── client/                 # Role A: LLMClient, GeminiClient, OllamaClient
│   ├── environment/            # Role C: NativeEnvironment, SandboxEnvironment
│   ├── harness/                # Role C: HarnessRunner, Task, Evaluator (Keyword, Functional, VLM)
│   ├── multiagent/             # Role C: MultiAgentRunner, Message, MessageQueue
│   └── tools/                  # Role B: Tool, ToolRegistry, Registry, các Tool cụ thể
└── tests/                      # Thư mục kiểm thử bổ sung
```

---

## 3. Cách Build và Chạy (Build & Run Instructions)

### 3.1 Yêu Cầu Môi Trường
- **Hệ điều hành**: WSL / Linux (môi trường được hỗ trợ chính thức).
- **Trình biên dịch**: GCC / Clang hỗ trợ C++26 (`-std=c++26`) hoặc MSVC (`/std:c++latest`).
- **Build tool**: CMake >= 3.28.
- **Thư viện phụ thuộc**:
  - `CURL` (Thực hiện HTTP REST API requests)
  - `SQLite3` (Cơ sở dữ liệu bộ nhớ persistent memory)
  - `Threads` (POSIX Threads cho Multi-agent concurrency)
  - `nlohmann_json` (Có sẵn fallback tại `include/nlohmann/` nếu môi trường thiếu)

### 3.2 Cấu Hình API Key
Tạo file `config.json` từ file mẫu:
```bash
cp config.json.example config.json
```
Cấu hình các thông số bảo mật trong `config.json`:
```json
{
  "gemini_api_key": "YOUR_GEMINI_API_KEY",
  "ollama_url": "http://localhost:11434"
}
```

### 3.3 Hướng Dẫn Biên Dịch (Build)
```bash
# 1. Tạo thư mục build và cấu hình CMake
cmake -S . -B build

# 2. Biên dịch dự án (dùng j2 hoặc nproc để tối ưu thời gian)
cmake --build build -j2
```

Các file thực thi sinh ra trong thư mục `build/`:
- `build/OopAgent`: Ứng dụng CLI tương tác chính với Agent.
- `build/run_eval`: Chương trình chạy đánh giá bộ Benchmark.
- `build/test_harness`: Bộ kiểm thử tự động cho Harness và Evaluator.
- `build/test_multi_agent`: Bộ kiểm thử hệ thống giao tiếp Đa Agent.
- `build/demo_multi_agent`: Demo minh họa luồng giao tiếp đa agent.

### 3.4 Hướng Dẫn Chạy Chương Trình
```bash
# Chạy ứng dụng CLI Agent tương tác
./build/OopAgent

# Chạy Demo Đa Agent
./build/demo_multi_agent

# Chạy Đánh Giá Benchmark (Gửi request thật đến LLM Provider)
./build/run_eval
```

---

## 4. Cách Chạy Test (Testing Instructions)

Dự án cung cấp cả CTest tích hợp và các executable test độc lập.

### 4.1 Chạy Qua CTest
```bash
cd build
ctest --output-on-failure
```

### 4.2 Chạy Trực Tiếp Các Binary Test
```bash
# Test 1: Kiểm thử Multi-Agent Threading & MessageQueue
./build/test_multi_agent

# Test 2: Kiểm thử HarnessRunner & các Evaluator
./build/test_harness
```

> **Lưu ý**: Trước khi khẳng định kết quả benchmark thành công, luôn chạy kiểm thử trên bản build mới nhất để tránh báo cáo kết quả dựa trên các file artifacts cũ.

---

## 5. Coding Conventions & Quy Định Kiến Trúc

1. **Chuẩn C++ & Quản Lý Tài Nguyên**:
   - Biên dịch với chuẩn C++26 (`-std=c++26` / `/std:c++latest`) và tương thích fallback C++23.
   - Bắt buộc tuân thủ mẫu **RAII** và con trỏ thông minh (`std::unique_ptr`, `std::shared_ptr`). **Không** tự ý dùng `new`/`delete` thủ công.
2. **Áp Dụng Thiết Kế Hướng Đối Tượng (OOP Design Patterns)**:
   - **Strategy Pattern**: Sử dụng cho bộ đánh giá `Evaluator` (`KeywordEvaluator`, `FunctionalEvaluator`, `VLMEvaluator`).
   - **Registry / Factory Pattern**: `ToolRegistry` và template `Registry<T>` quản lý khởi tạo công cụ linh hoạt.
   - **Observer / Hook Pattern**: `StepHook` trong `AgentLoop` giúp lắng nghe và theo dõi từng bước thực thi của agent.
   - **Polymorphism & Interface Isolation**: Duy trì các lớp trừu tượng `LLMClient`, `Tool`, `Evaluator`, `Environment` độc lập.
3. **Mã Nguồn Bên Thứ Ba (Vendored Code)**:
   - **Tuyệt đối không chỉnh sửa** các header thư viện được vendor sẵn dưới thư mục `include/nlohmann/`.
4. **An Toàn Đa Luồng (Thread Safety)**:
   - Các cấu trúc dùng chung giữa các agent như `MessageQueue` phải được bảo vệ bằng `std::mutex` và `std::lock_guard` / `std::unique_lock`.

---

## 6. Các File & Thư Mục Quan Trọng (Important Paths)

| Thư mục / File | Vai trò & Trách nhiệm |
| :--- | :--- |
| `CMakeLists.txt` | Cấu hình build chính cho toàn bộ 5 mục tiêu thực thi (targets). |
| `AGENTS.md` | File quy chuẩn gốc cho toàn bộ quy trình phát triển và làm việc nhóm. |
| `src/agent/agent_loop.h/.cpp` | Mạch điều khiển trung tâm (AgentLoop) xử lý giao tiếp LLM, gọi Tool và Hook. |
| `src/client/gemini_client.h/.cpp` | Client giao tiếp với Google Gemini REST API hỗ trợ Function Calling. |
| `src/client/ollama_client.h/.cpp` | Client giao tiếp với Ollama server chạy cục bộ. |
| `src/tools/ToolRegistry.h/.cpp` | Nơi đăng ký, quản lý alias và điều phối thực thi tất cả Tool. |
| `src/harness/HarnessRunner.h/.cpp` | Động cơ chạy tác vụ benchmark và đối chiếu với Evaluator. |
| `src/multiagent/MultiAgentRunner.h/.cpp` | Quản lý vòng đời và luồng giao tiếp giữa nhiều Agent. |
| `benchmark/tasks.json` | **Source-of-Truth** lưu giữ danh sách các bài test benchmark. |
| `filephanchiacv/` | Chứa tài liệu lập kế hoạch theo tuần & nhật ký phân tích lỗi (`PHAN_TICH_RUN_*.md`). |
| `docs/` | Báo cáo chi tiết kiến trúc OOP, sơ đồ lớp UML, sequence diagrams. |

---

## 7. Những Lưu Ý Quan Trọng Khi Chỉnh Sửa Code

### 7.1 Bảo Tồn Ranh Giới Mô-đun (Architectural Constraints)
- **Không hardcode công cụ vào `AgentLoop`**: Tất cả công cụ phải được đăng ký qua `ToolRegistry`.
- **Độc lập mô-đun**: Giữ `AgentLoop` độc lập với `HarnessRunner`, Tool độc lập với vòng lặp agent, và Evaluator độc lập với cấu trúc bên trong của agent.

### 7.2 Đảm Bảo Tính Đúng Đắn Của Benchmark
- **Không nới lỏng tiêu chí test**: Tuyệt đối không sửa Evaluator hoặc nới lỏng bài test chỉ để tăng điểm benchmark ảo.
- **Cô lập & dọn dẹp file rác**: Trước khi chạy benchmark, cần dọn dẹp hoặc cô lập các file kết quả cũ trong thư mục gốc (như `notes.txt`, `result.txt`, `capital.txt`, `calc.txt`, `data.txt`, `output.txt`) để không gây ra lỗi pass giả lập (false passes).
- **Lưu trữ chính xác tham số Tool**: Trong `Trajectory`, bắt buộc ghi lại đầy đủ tham số (`args`) thực sự đã truyền cho Tool, không để rỗng.
- **Lý do thất bại rõ ràng**: Trả về failure reasons chi tiết (vd: `invalid arguments`, `missing artifacts`, `content mismatch`, `loop detection`, `incomplete task`).

### 7.3 Bảo Mật & An Toàn
- **Bảo mật `config.json`**: File `config.json` chứa API key bí mật. Không bao giờ in (log), commit, hay hiển thị API key.
- **An toàn câu lệnh**: Giữ chính sách thực thi lệnh shell trong `ExecTool` nghiêm ngặt và luôn trả về `ToolError` khi lệnh bị từ chối hoặc lỗi.
- **Xác nhận từ người dùng**: Khi thực hiện các bài benchmark tốn chi phí network/API key thực tế, phải xác nhận với người dùng trước nếu cần.

### 7.4 Quy Trình Xác Minh & Commit
- Luôn giữ thay đổi theo đúng phạm vi Role được phân công (Role A / Role B / Role C).
- Bắt buộc phải biên dịch sạch (`cmake --build build`) và chạy các test suite (`./build/test_multi_agent`, `./build/test_harness`) thành công trước khi kết luận hoàn thành tác vụ.
- Không commit `config.json`, API keys, file build output, cơ sở dữ liệu `memory.db`, hay các file kết quả benchmark ngoại trừ khi có yêu cầu cụ thể.
=======
# AGENTS.md

## Project overview

This repository is a C++ AI-agent project with three main ownership areas:

- Role A — systems/core: LLM clients, `AgentLoop`, parsing, skills, and loop detection.
- Role B — tools/data: tool interfaces, registry, aliases, argument parsing, and tool implementations.
- Role C — evaluation/infra: harness, evaluators, benchmark tasks/results, trajectories, and multi-agent demos.

The benchmark source of truth is `benchmark/tasks.json`. Historical benchmark evidence and team plans live under `filephanchiacv/`; in particular, consult the newest `PHAN_TICH_RUN_*.md` before changing behavior for a failed run.

## Build and verification

Use WSL/Linux for the supported build because the project depends on CURL, SQLite3, Threads, and nlohmann_json:

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_multi_agent
./build/run_eval
```

`run_eval` can call a real LLM provider and writes benchmark artifacts. Inspect `config.json` without exposing its API key, and obtain user confirmation before a real network-backed benchmark when cost, quota, or generated artifacts matter.

For changes to one component, build all targets and run the smallest relevant executable first. A claimed benchmark success must come from a clean, current run rather than old files in the repository root.

## Architecture constraints

- Preserve the abstract `LLMClient`, `Tool`, and `Evaluator` interfaces.
- Keep tool registration in `ToolRegistry`; do not hardcode tool implementations into `AgentLoop`.
- Keep `AgentLoop` independent of `HarnessRunner`, tools independent of the agent loop, and evaluators independent of agent internals.
- Preserve the intended patterns: Strategy (`Evaluator`), Registry/Factory (`ToolRegistry`/`Registry<T>`), Observer/Hook (`StepHook`), and the agent-loop orchestration flow.
- Use RAII and smart pointers; do not introduce raw owning `new`/`delete`.
- Maintain the guarded C++26 feature path and its portability fallback. GNU/Clang builds are explicitly compiled with `-std=c++26`; MSVC uses `/std:c++latest` where configured.

## Benchmark correctness

- Do not weaken task post-conditions or evaluators merely to raise the score.
- Tasks that require tools must contain a real, successful tool step.
- File-producing tasks must verify the exact filename and content created during the current run.
- Clean or isolate known task artifacts before evaluation so stale `notes.txt`, `result.txt`, `capital.txt`, `calc.txt`, `data.txt`, `output.txt`, or similarly malformed files cannot cause false passes.
- Preserve actual tool arguments in trajectories; do not report an empty `args` field when arguments were supplied.
- Prefer specific failure reasons such as invalid arguments, missing artifacts, content mismatch, loop detection, or incomplete task.

## Editing conventions

- Read the relevant role plan and latest run analysis before editing role-owned code.
- Keep changes scoped to the requested role/task and preserve unrelated user modifications in a dirty worktree.
- Update or add focused tests for parsers, tool arguments, loop detection, and evaluator behavior when those areas change.
- Never commit `config.json`, API keys, generated benchmark runs, build output, databases, or task artifacts unless the user explicitly requests it.
- Do not edit vendored headers under `include/nlohmann/`.

## Security

- Treat `config.json` as secret-bearing. Never print, log, or commit its API key.
- Keep shell execution policy restrictive and return explicit `ToolError` values for rejected or failed operations.
- Do not make benchmark success depend on mocks when reporting a real provider success rate.
>>>>>>> cf111427bf1483acb031b067d6a52f319e93a40f
