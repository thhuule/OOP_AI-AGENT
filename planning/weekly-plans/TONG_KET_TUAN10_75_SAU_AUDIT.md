# Tổng kết Tuần 10.75 sau audit

**Ngày kiểm tra:** 2026-08-20  
**Phạm vi bonus được chốt:** Vector Search và Multi-agent. GUI/VLM không nằm trong điều kiện freeze.

## 1. Đã sửa những gì?

### Agent và skill

- Agent chỉ chọn skill phù hợp với nội dung task; nếu không khớp thì dùng `task_planner`.
- Danh sách tool trong prompt được lấy trực tiếp từ `ToolRegistry`, nên tên và mô tả tool không còn bị chép cứng ở nhiều nơi.
- Lịch sử hội thoại lưu đúng vai trò: câu trả lời của model là `assistant`, kết quả tool là `tool`.
- JSON tool call sai định dạng được trả về dưới dạng lỗi rõ ràng, không bị xem nhầm là câu trả lời thành công.

### Token và trajectory

- Gemini và Ollama đọc số token từ metadata thật của provider.
- Final answer được lưu vào trajectory.
- Final answer không bị tính nhầm là một tool step.
- File kết quả có `final_answer`, token từng bước và `total_tokens`. Nếu provider không gửi metadata thì giá trị bằng `0` và được hiểu là “không đo được”.

### Persistent memory và Vector Search

- Có hai tool production rõ ràng: `memory_save` và `memory_search`.
- Luồng chính `save/search` dùng embedding và cosine similarity.
- Production dùng Ollama `nomic-embed-text`; test offline dùng `HashEmbedder` để chạy ổn định, không phụ thuộc mạng.
- Keyword search cũ vẫn được giữ dưới tên `legacy_save/legacy_search` để tương thích, nhưng không được dùng để claim Vector bonus.

### Multi-agent, C++26 và tài liệu

- Báo cáo đã mô tả đúng production path của Multi-agent: Harness → MultiAgentRunner → hai worker thread → queue → kết quả.
- C++26 được chứng minh bằng deleted-function reason trên class sở hữu thread; `inplace_vector` chỉ là fallback portability và không được claim là feature đang chạy.
- README, report, diagram, checklist và project status đã đồng bộ với code hiện tại.
- Success rate thật gần nhất được ghi là **7/10**, không dùng fallback để ép thành 10/10.
- Các link tài liệu bị gắn đường dẫn máy cá nhân hoặc sai cấp thư mục đã được đổi thành đường dẫn tương đối trong repository.

## 2. Đã kiểm tra như thế nào?

| Gate | Kết quả |
|---|---|
| Focused tests Role A/B/C | PASS |
| CTest | `5/5 PASS` |
| Live Vector với `nomic-embed-text` | PASS |
| Multi-agent focused test | PASS |
| Benchmark Gemini thật | `7/10`, action source là `llm`, không có fixture |
| Package hygiene | PASS |
| Clean build từ bản sao chỉ chứa file tracked | PASS |
| CTest trong bản sao sạch | `5/5 PASS` |

Clean-package gate cuối dùng một thư mục tạm gồm **170 file của candidate**, bao gồm hai file evidence/tổng kết mới. Sau khi hygiene, build và test hoàn tất, thư mục tạm được xóa.

## 3. Vì sao benchmark không cần 10/10?

Đề yêu cầu có tối thiểu 10 task theo tỷ lệ 4 đơn giản, 4 trung bình, 2 khó và phải ghi success rate của model đã chọn. Model thật có thể chọn sai tool, bị timeout hoặc lặp hành động. Vì vậy:

- test code dùng để chứng minh implementation đúng;
- `run_eval` dùng để đo chất lượng model thật;
- không được thêm fallback/hard-code chỉ để làm điểm benchmark đẹp hơn.

Kết quả hiện tại `7/10` là evidence hợp lệ và trung thực.

## 4. Còn gì trước khi freeze?

- [ ] Role A review độc lập một production trajectory và contract được giao.
- [ ] Role B review độc lập một production trajectory và contract được giao.
- [ ] Cả nhóm xử lý rule Git với giảng viên. Phép đo đã hoàn tất: tỷ lệ committed `src/` là 91.62% / 6.20% / 2.18%; số commit là 101 / 66 / 50; khoảng cách lớn nhất là 8.76 / 20.85 / 10.91 ngày. Kết quả hiện tại FAIL rule 20% và 7 ngày; không rewrite/recommit giả.
- [ ] Team commit/push candidate đã review.
- [ ] Team cập nhật revision/tag và tuyên bố code freeze.

Các bước kỹ thuật đã PASS, nhưng **Code freeze vẫn là NO/PENDING** cho tới khi A/B ký review, Git contribution gate có kết luận trung thực và team tuyên bố freeze. Không nên thêm feature mới trong lúc chờ review.

## 5. Quy tắc sau freeze

```text
No feature changes unless Critical Fix.
Critical Fix → targeted test → full regression → re-freeze.
```

Tuần sau chỉ merge/format report, làm PowerPoint, chuẩn bị script/oral và quay/chỉnh video dựa trên revision đã freeze.
