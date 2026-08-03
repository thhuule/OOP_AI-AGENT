# OOP AI Agent

Framework AI Agent viết bằng C++ với LLM client thay thế được, vòng lặp gọi tool, skill Markdown, phát hiện lặp, benchmark harness và demo phối hợp nhiều agent.

## Tổng quan kiến trúc

| Thành phần | Trách nhiệm | Vị trí |
|---|---|---|
| LLM client | Gửi lịch sử hội thoại tới Gemini hoặc Ollama | `src/client/` |
| Agent loop | Điều phối prompt, phản hồi LLM, tool call và điều kiện dừng | `src/agent/` |
| Tool registry | Đăng ký, tìm và thực thi tool | `src/tools/` |
| Skill system | Nạp hướng dẫn Markdown vào system prompt | `src/skills/` |
| Evaluation harness | Nạp task, chạy agent, chấm điểm và xuất trajectory | `src/harness/`, `benchmark/` |
| Multi-agent | Worker thread, dispatcher và message queue | `src/multiagent/` |

Các lớp chính giao tiếp qua abstraction `LLMClient`, `Tool` và `Evaluator`. `AgentLoop` không phụ thuộc `HarnessRunner`; Harness thu trajectory qua `StepHook`.

## Yêu cầu môi trường

Môi trường build được hỗ trợ là WSL/Linux. Ví dụ trên Ubuntu:

```bash
sudo apt update
sudo apt install cmake g++ libcurl4-openssl-dev nlohmann-json3-dev libsqlite3-dev
```

Project cần CMake 3.28 trở lên. GNU/Clang được biên dịch với `-std=c++26`; vì vậy compiler phải hỗ trợ các tính năng C++ mới được source sử dụng.

## Build

Chạy từ thư mục gốc repository trong WSL:

```bash
cmake -S . -B build
cmake --build build -j2
```

Năm executable được tạo trong `build/`:

- `OopAgent`: smoke test một yêu cầu Gemini. Đây không phải chế độ chat và không hỗ trợ cờ `--chat`.
- `run_eval`: chạy batch 10 task trong `benchmark/tasks.json` và xuất kết quả.
- `test_harness`: focused test local cho task validation, evaluator Strategy, StepHook, cleanup, failure taxonomy và cách tổng hợp điểm; không cần LLM API.
- `test_multi_agent`: test local cho dispatcher, message queue và shutdown; không cần LLM API.
- `demo_multi_agent`: demo hai worker tính toán/tìm kiếm rồi tạo `report.txt`. Nhánh web search có thể dùng mạng.

## Cấu hình

Tạo file cấu hình cục bộ:

```bash
cp config.json.example config.json
```

Ví dụ Gemini:

```json
{
  "provider": "gemini",
  "api_key": "YOUR_API_HERE",
  "model": "gemma-4-31b-it",
  "api_url": "https://generativelanguage.googleapis.com/v1beta",
  "use_mock": false
}
```

Ví dụ Ollama:

```json
{
  "provider": "ollama",
  "api_key": "",
  "model": "gemma4:e4b",
  "api_url": "http://localhost:11434",
  "use_mock": false
}
```

Lưu ý theo code hiện tại:

- `run_eval` đọc `provider`, `api_key`, `model` và `api_url`.
- `OopAgent` luôn tạo `GeminiClient` và chỉ đọc `api_key`, `model`.
- Trường `use_mock` có trong file mẫu nhưng chưa được nối vào đường chạy của executable. Không dùng trường này để tuyên bố một run là mock hoặc real-provider.
- `LLMConfig` có temperature/timeout mặc định trong source, nhưng `run_eval` chưa đọc các giá trị này từ `config.json`.

Không commit `config.json`. File này có thể chứa API key và đã được liệt kê trong `.gitignore`.

## Chạy kiểm thử và demo

Chạy các test local trước:

```bash
./build/test_harness
./build/test_multi_agent
```

Hoặc chạy cả hai qua CTest:

```bash
ctest --test-dir build --output-on-failure
```

Kết quả đạt phải có hai dòng kết thúc tương ứng:

```text
ALL HARNESS TESTS PASSED
ALL PASSED
```

Chạy demo multi-agent khi chấp nhận việc demo có thể truy cập DuckDuckGo:

```bash
./build/demo_multi_agent
```

Demo tạo `report.txt` tại working directory. Đây là phần mở rộng độc lập, chưa phải bằng chứng rằng benchmark harness điều phối sub-agent.

## Chạy benchmark

`run_eval` gọi provider thật theo cấu hình hiện tại, có thể tốn quota, dùng mạng và tạo file. Trước khi chạy:

1. Kiểm tra `config.json` mà không in hoặc chia sẻ API key.
2. Xác nhận model, quota và chi phí được phép sử dụng.
3. Build toàn bộ target, chạy `test_harness` và `test_multi_agent`.
4. Đảm bảo working tree không chứa artifact cũ cần giữ lại.

Sau đó chạy:

```bash
./build/run_eval
```

Harness đọc `benchmark/tasks.json`, dọn danh sách artifact benchmark đã biết trước batch, chạy tuần tự 10 task rồi tạo:

```text
benchmark/results/run_YYYYMMDD_HHMMSS_mmm/
├── eval_results.json
├── benchmark_summary.txt
├── trajectory_task_001.json
└── ...
```

`eval_results.json` chứa điểm evaluator, điểm action-level và success rate cuối. Mỗi trajectory chứa thought, action, tool result và latency của các tool step.

Trường `tokens_used` hiện bằng `0` vì client chưa truyền token metadata về Harness. Giá trị này có nghĩa **chưa đo**, không có nghĩa model không sử dụng token.

## Tiêu chí benchmark

`benchmark/tasks.json` là nguồn sự thật. Bộ hiện tại gồm:

- 4 task simple;
- 4 task medium;
- 2 task hard.

Task yêu cầu tool chỉ đạt kết quả cuối khi evaluator đạt và có ít nhất một tool step liên quan được Harness xem là thành công. Task tạo file còn được FunctionalEvaluator kiểm tra filename/nội dung bằng script hậu điều kiện.

Kết quả trong `benchmark/results/` là bằng chứng lịch sử. Muốn tuyên bố kết quả của phiên bản hiện tại phải dùng một run sạch, mới và đúng provider đã công bố.

## An toàn

- Không commit API key, `config.json`, database, build output hoặc artifact benchmark phát sinh.
- Không sửa task hay evaluator chỉ để tăng điểm.
- `execute_shell` phải giữ policy hạn chế; không dùng benchmark để chạy lệnh ngoài phạm vi task.
- `run_eval` và `demo_multi_agent` có thể dùng mạng. `test_multi_agent` là lựa chọn kiểm tra local trước.

## Xử lý lỗi thường gặp

### CMake không tìm thấy dependency

Cài đủ CURL, SQLite3 và nlohmann-json rồi cấu hình lại:

```bash
sudo apt install libcurl4-openssl-dev libsqlite3-dev nlohmann-json3-dev
cmake -S . -B build
```

### Không tìm thấy `config.json`

Chạy executable từ repository root hoặc thư mục `build/`, và bảo đảm đã copy `config.json.example` thành `config.json`.

### Gemini trả 429 hoặc Resource Exhausted

Dừng benchmark, kiểm tra quota/rate limit và không chạy lặp lại liên tục. Run bị rate limit không được báo cáo như kết quả chất lượng của agent.

### Ollama không kết nối được

Kiểm tra Ollama đang chạy, `api_url` đúng và model đã có trong máy.

### Benchmark pass nhờ file cũ

Không dùng file ở repository root làm bằng chứng. Kiểm tra log cleanup và thư mục run có timestamp. Nếu cleanup thất bại, Harness dừng batch để tránh false positive.

## Tài liệu

- [Báo cáo evaluation](docs/report_evaluation.md)
- [Sequence diagram batch evaluation](docs/sequence_harness.md)
- [Checklist đóng gói Tuần 12](docs/submission_checklist.md)
- [Storyboard video demo](docs/video_storyboard.md)
