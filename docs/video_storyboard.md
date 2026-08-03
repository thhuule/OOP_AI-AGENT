# Storyboard video demo

Mục tiêu: chứng minh yêu cầu đồ án bằng một video ngắn, tái lập được và không lộ secret. Video được tải lên YouTube ở chế độ **Unlisted**.

## Thời lượng đề xuất: 8–10 phút

| Thời gian | Nội dung hình ảnh | Lời trình bày chính | Bằng chứng |
|---|---|---|---|
| 0:00–0:30 | Tiêu đề, thành viên, mục tiêu | AI Agent C++, ReAct, tools, skills và evaluation | Slide mở đầu |
| 0:30–1:30 | Component/class diagram | Ba layer A/B/C và abstraction chính | Mermaid diagram |
| 1:30–2:15 | README + terminal WSL | Dependency và quy trình build | Lệnh CMake |
| 2:15–3:15 | Source `AgentLoop` và một tool call | LLM trả action, registry tìm tool, loop nhận observation | Sequence agent run |
| 3:15–4:00 | Tool inventory | File, exec, web, memory, calculator và tool bổ sung | Báo cáo tools |
| 4:00–5:00 | `benchmark/tasks.json` + sequence harness | 10 task, evaluator Strategy, StepHook, artifact cleanup | Sequence batch eval |
| 5:00–6:15 | Chạy `test_harness`, `test_multi_agent` và mở test source | Harness validation/trajectory đạt; message bus chuyển đúng kết quả và shutdown sạch | Hai test đều pass |
| 6:15–7:30 | Run benchmark xác nhận đã được chuẩn bị | Điểm tổng, category, trajectory task 005/010 | Thư mục run mới |
| 7:30–8:20 | So sánh run 2/10 và 10/10 | Giải thích lỗi parsing artifact và cách evidence thay đổi | Báo cáo evaluation |
| 8:20–9:00 | Limitations/backlog | Token chưa đo, VLM skeleton, Harness chưa tích hợp sub-agent | Phần giới hạn |
| 9:00–9:30 | Kết luận | Tóm tắt OOP, độ tái lập và kết quả đã xác minh | Checklist cuối |

## Kịch bản thao tác terminal

Chuẩn bị trước khi quay; không chạy cài package dài trong video:

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_harness
./build/test_multi_agent
```

Chỉ quay `run_eval` thật nếu:

- nhóm đã đồng ý dùng quota/chi phí;
- `config.json` đã được kiểm tra nhưng không hiển thị;
- artifact cũ không cần giữ;
- đủ thời gian chờ cả 10 task.

Nếu benchmark thật dài, có thể trình bày thư mục của một run xác nhận sạch đã chạy ngay trước buổi quay. Phải nói rõ run ID và thời điểm; không dựng kết quả giả hoặc dùng run lịch sử như thể vừa chạy.

## Hai trajectory nên mở

### Task 005

Chỉ ra ba hành động:

1. `calculator` với `47 * 23` → `1081`;
2. `write_file` với `result.txt,1081`;
3. `read_file` để xác minh lại artifact.

### Task 010

Chỉ ra recovery flow:

1. `read_file(data.txt)` trả `ToolError: NotFound`;
2. tạo file với `initial data`;
3. append dòng `appended`;
4. đọc lại nội dung cuối.

## Điều không được quay hoặc tuyên bố

- Không hiển thị `config.json`, API key, token hay credential.
- Không nói `tokens_used = 0` nghĩa là model không dùng token; phải nói chưa đo.
- Không gọi `VLMEvaluator` là evaluator ảnh hoàn chỉnh.
- Không gọi demo `MultiAgentRunner` là tích hợp sub-agent trong Harness.
- Không nói `OopAgent --chat`; executable hiện chỉ là smoke test Gemini.
- Không dùng action-level score một mình làm bằng chứng hoàn thành task.
- Không đưa demo live Tuần 13 trở lại như yêu cầu bắt buộc.

## Checklist trước upload

- [ ] Chữ terminal đủ lớn và không có thông tin cá nhân nhạy cảm.
- [ ] Âm thanh nghe rõ; tên class/file đọc đúng.
- [ ] Run ID và model trong phần benchmark khớp artifact.
- [ ] Video không có đoạn chờ dài hoặc lỗi chưa giải thích.
- [ ] Link đặt ở chế độ Unlisted và mở được khi không đăng nhập.
- [ ] Link cuối được thêm vào báo cáo/nơi nộp theo yêu cầu giảng viên.
