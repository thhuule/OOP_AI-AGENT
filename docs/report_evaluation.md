# Báo cáo Evaluation và Benchmark

## 1. Phạm vi

Tài liệu này mô tả phần Evaluation/Infra theo code hiện tại: nguồn task, vòng đời benchmark, evaluator, trajectory, cách tính điểm, bằng chứng run lịch sử, multi-agent và các giới hạn chưa đóng.

Nguồn sự thật:

- Task: [`../benchmark/tasks.json`](../benchmark/tasks.json)
- Harness: [`../src/harness/HarnessRunner.h`](../src/harness/HarnessRunner.h), [`../src/harness/HarnessRunner.cpp`](../src/harness/HarnessRunner.cpp)
- Evaluator: [`../src/harness/evaluator.h`](../src/harness/evaluator.h)
- Entrypoint: [`../benchmark/run_eval.cpp`](../benchmark/run_eval.cpp)
- Kết quả lịch sử: `../benchmark/results/run_20260801_212302_253/` và `../benchmark/results/run_20260801_220549_361/`

Các số liệu bên dưới là bằng chứng lịch sử trong repository. Chúng không thay cho run xác nhận cuối từ trạng thái code sạch hiện tại.

## 2. Bộ benchmark

`benchmark/tasks.json` hiện có 10 task:

| Nhóm | Số task | Task |
|---|---:|---|
| Simple | 4 | `task_001`–`task_004` |
| Medium | 4 | `task_005`–`task_008` |
| Hard | 2 | `task_009`–`task_010` |

Mỗi task khai báo `id`, instruction, loại evaluator, category, `requires_tool`, danh sách tool được chấp nhận, artifact và `max_steps`. `HarnessRunner::loadTasks()` từ chối task thiếu trường bắt buộc, evaluator spec sai, tool spec rỗng, artifact path không an toàn hoặc ID trùng.

Hai evaluator hoàn chỉnh đang được dùng:

- `KeywordEvaluator`: tách danh sách keyword theo dấu phẩy, tính tỉ lệ khớp và chỉ pass khi đủ tất cả keyword.
- `FunctionalEvaluator`: chạy `eval_script` qua `ExecTool` và pass khi output chứa `PASS`.

`VLMEvaluator` hiện chỉ trả fail với thông báo chưa triển khai. Vì vậy tài liệu không xem đây là evaluator thị giác hoàn chỉnh.

## 3. Vòng đời Harness

Luồng thực tế:

1. `run_eval` đọc `config.json` và tạo `GeminiClient` hoặc `OllamaClient` qua `LLMClient`.
2. Đăng ký tool và nạp skill Markdown.
3. `HarnessRunner` nạp `benchmark/tasks.json`.
4. `StepHook` được inject vào `AgentLoop` để Harness thu tool step mà không tạo dependency ngược từ Agent sang Harness.
5. `runAll()` dọn các artifact benchmark đã biết một lần trước batch.
6. Mỗi task chạy tuần tự qua `AgentLoop::run()`; giữa hai task có khoảng chờ ba giây.
7. Harness chọn evaluator theo `eval_type`, tính evaluator score, action-level score và kết quả cuối.
8. `exportResults()` tạo thư mục run có timestamp, summary JSON, summary text và trajectory từng task.

Sequence chi tiết nằm tại [`sequence_harness.md`](sequence_harness.md).

### 3.1 Artifact cleanup và isolation

Harness hiện xóa `notes.txt`, `result.txt`, `capital.txt`, `output.txt`, `calc.txt`, `data.txt` cùng các artifact khai báo trong task. Path tuyệt đối hoặc path chứa `..` bị từ chối. Nếu xóa thất bại, batch dừng để tránh file cũ làm task pass giả.

Giới hạn cần nói rõ: cleanup hiện diễn ra **một lần trước cả batch**, chưa có workspace riêng cho từng task. Các task chạy cùng working directory; `task_003` và `task_007` còn sử dụng `notes.txt` do `task_002` tạo trong cùng batch. Vì vậy đây là batch cleanup, chưa phải isolation độc lập theo từng task.

## 4. Cách chấm điểm

Với mỗi task, Harness lưu ba lớp kết quả:

| Chỉ số | Điều kiện hiện tại |
|---|---|
| Evaluator score | `KeywordEvaluator` hoặc `FunctionalEvaluator` trả pass |
| Action-level score | Task không yêu cầu tool, hoặc có tool step thuộc `required_tools` và result không chứa dấu hiệu lỗi phổ biến |
| Final success | Evaluator pass **và** action-level pass |

Điểm toàn batch là số task đạt từng tiêu chí chia cho tổng số task.

Action-level hiện là heuristic: nó kiểm tra tên tool và chuỗi result, chưa tự xác minh toàn bộ hậu điều kiện artifact. Hậu điều kiện file được bảo vệ ở lớp evaluator và chỉ ảnh hưởng `final_success`. Vì vậy không được dùng action-level score một mình để tuyên bố task đã hoàn thành đúng.

## 5. Trajectory và export

Mỗi tool step hiện có:

- `thought`: phản hồi LLM tạo ra tool call;
- `action`: object gồm `type`, `tool`, `args`;
- `tool_result`: kết quả hoặc `ToolError`;
- `latency_ms`: thời gian từ lần hook trước;
- `tokens_used`: trường dự phòng, hiện chưa được đo.

Các file xuất:

| File | Nội dung |
|---|---|
| `eval_results.json` | Điểm tổng, điểm theo category và kết quả từng task |
| `trajectory_task_XXX.json` | Step, action args, tool result, latency và token field |
| `benchmark_summary.txt` | Tóm tắt pass/fail dễ đọc |

### 5.1 Giới hạn token

`HarnessRunner::createStepHook()` hiện gán `tokens_used = 0`. `LLMClient::generate_chat()` chỉ trả nội dung dạng chuỗi; Gemini `usageMetadata` và các trường đếm token của Ollama chưa được truyền qua interface.

Do đó:

> `tokens_used = 0` nghĩa là **chưa đo**, không có nghĩa model dùng 0 token.

Đo token thật được để vào backlog Tuần 10: bổ sung response metadata cho `LLMClient`, parse usage của từng provider, truyền qua `AgentLoop`/hook và cộng cả lần gọi tạo final answer. Không dùng ước lượng ký tự như số token chính thức.

## 6. Failure taxonomy

`HarnessRunner::classifyFailure()` hiện có thể xuất:

| Mã | Ý nghĩa |
|---|---|
| `NONE` | Task đạt kết quả cuối |
| `RATE_LIMIT` | Có dấu hiệu 429/resource exhausted |
| `TIMEOUT` | LLM/tool/evaluator hết thời gian |
| `TOOL_NOT_FOUND` | Agent gọi tool không tồn tại |
| `INVALID_ARGS` | Args tool không hợp lệ |
| `TOOL_EXECUTION_FAILED` | Tool trả ExecutionFailed, AccessDenied hoặc UnknownError |
| `LOOP_DETECTED` | Agent bị loop detector dừng |
| `EVALUATOR_ERROR` | Evaluator không thể tạo kết quả chấm hợp lệ |
| `NO_TOOL_EXECUTION` | Task yêu cầu tool nhưng không có tool step phù hợp thành công |
| `ARTIFACT_MISSING` | Artifact bắt buộc không tồn tại |
| `ARTIFACT_CONTENT_MISMATCH` | Có artifact nhưng nội dung không đạt evaluator |
| `INCOMPLETE_TASK` | Agent dừng mà chưa hoàn thành yêu cầu |
| `POST_CONDITION_FAIL` | Evaluator fail nhưng chưa phân loại cụ thể hơn |
| `PARSER_FAIL` | Nhánh fallback khi evaluator đạt nhưng kết quả tổng vẫn lỗi |

Classifier đọc evidence đã normalize, bao gồm cả dạng enum liền chữ như `InvalidArgument` và `ExecutionFailed`. Việc phân loại vẫn dựa trên evidence text; về dài hạn nên truyền lỗi có kiểu xuyên suốt thay vì chỉ dựa vào chuỗi.

## 7. So sánh hai run lịch sử

| Chỉ số | `run_20260801_212302_253` | `run_20260801_220549_361` |
|---|---:|---:|
| Task pass | 2/10 | 10/10 |
| Evaluator score | 0.2 | 1.0 |
| Action-level score | 1.0 | 1.0 |
| Final success rate | 0.2 | 1.0 |
| Simple | 1/4 | 4/4 |
| Medium | 1/4 | 4/4 |
| Hard | 0/2 | 2/2 |

### 7.1 Run lỗi `212302_253`

Run cũ cho thấy action-level score 1.0 nhưng final success chỉ 0.2. Nguyên nhân chính được truy vết trong phân tích lịch sử là `FileWriteTool` parse sai args: tool báo `OK` nhưng tạo sai filename/nội dung. Điều này chứng minh action-level score cũ quá lạc quan nếu đứng riêng.

Các triệu chứng quan sát được:

- `task_002`, `005`, `006`, `009`: artifact sai tên hoặc sai nội dung;
- `task_003`, `007`: lỗi dây chuyền từ `notes.txt`;
- `task_009`: agent dừng do loop;
- `task_010`: chưa hoàn tất append;
- `task_001`: instruction/evaluator phiên bản cũ không đồng bộ với danh sách file gốc.

### 7.2 Run đạt `220549_361`

Artifact lịch sử ghi nhận 10/10. Hai trajectory quan trọng:

- `task_005`: giữ args `47 * 23`, `result.txt,1081`, `result.txt`; calculator, write và read đều thành công.
- `task_010`: giữ `ToolError: NotFound`, sau đó write, append và read lại `initial data\nappended`.

Run mới cung cấp bằng chứng rằng args thật đã có trong trajectory và luồng recovery của task 010 hoàn tất. Tuy nhiên thư mục run vẫn chỉ là bằng chứng lịch sử; không dùng nó để khẳng định commit hiện tại đạt 10/10 nếu chưa chạy lại sạch.

## 8. Multi-agent

`MultiAgentRunner` cung cấp đăng ký worker, thread riêng, dispatcher, message queue, timeout nhận tin và `stopAndJoinAll()`. `test_multi_agent` kiểm tra message `ping` được chuyển thành `RESULT:ping` và runner dừng hoàn toàn. `demo_multi_agent` chạy worker calculator và search rồi gộp kết quả vào `report.txt`.

Đây là phần mở rộng độc lập. `HarnessRunner` hiện chưa gọi `MultiAgentRunner`; vì vậy demo này chưa đủ bằng chứng cho bonus tích hợp sub-agent trong benchmark harness.

## 9. Quy trình xác nhận cuối

Không chạy real-provider benchmark chỉ để cập nhật tài liệu. Khi A/B đã freeze code và nhóm đồng ý quota/chi phí:

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_harness
./build/test_multi_agent
./build/run_eval
```

Checklist sau run:

1. Run mới có đúng 10 task và đúng phân bố 4/4/2.
2. Log xác nhận cleanup thành công trước batch.
3. Task yêu cầu tool có tool step thật, tool phù hợp và result không lỗi.
4. Task file có đúng filename và content trong chính run hiện tại.
5. Trajectory task 005/010 giữ args thật.
6. Báo cáo provider/model đúng với config đã dùng nhưng không lộ API key.
7. Không gọi token bằng `0` là mức sử dụng thật.

## 10. Focused test hiện có

`benchmark/test_harness.cpp` là executable không mạng, dùng fake LLM/evaluator và thư mục tạm. Bộ test hiện kiểm tra:

- task hợp lệ được load;
- artifact path có `..` và task ID trùng bị từ chối;
- evaluator Strategy đăng ký theo tên được chọn và giữ nguyên score/feedback;
- StepHook giữ đúng action type, tool name, args, result và latency;
- batch cleanup xóa artifact cũ trong thư mục test cô lập;
- phân biệt `ARTIFACT_MISSING` và `ARTIFACT_CONTENT_MISMATCH`;
- phân biệt `INVALID_ARGS`, `TOOL_EXECUTION_FAILED` và `EVALUATOR_ERROR`;
- task bắt buộc tool không thể pass chỉ bằng final answer;
- tổng hợp evaluator/action/final score.

Lệnh xác minh:

```bash
./build/test_harness
```

Kết quả đạt kết thúc bằng `ALL HARNESS TESTS PASSED`. Test hiện chưa chứng minh Harness dùng `Environment` abstraction vì hierarchy này chưa tồn tại; phần đó phải bổ sung sau khi A cung cấp interface.

CMake cũng đăng ký `harness` và `multi_agent` với CTest:

```bash
ctest --test-dir build --output-on-failure
```

## 11. Backlog sau Tuần 9

- Thu thập token metadata thật cho Gemini/Ollama.
- Bổ sung test Harness qua `Environment` abstraction và cleanup failure được inject có chủ đích.
- Mở rộng failure taxonomy test cho rate limit, timeout và loop detection.
- Tách `tool_steps_count` khỏi tổng số LLM step khi trajectory bắt đầu ghi cả final answer.
- Cân nhắc workspace riêng hoặc fixture setup rõ ràng để giảm phụ thuộc thứ tự task.
- Chỉ làm integration `HarnessRunner → MultiAgentRunner → MessageQueue` nếu nhóm chọn mục tiêu bonus.
