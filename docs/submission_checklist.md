# Checklist đóng gói và nộp bài

Mốc nộp theo kế hoạch hiện hành: **trước 21:00 Chủ nhật Tuần 12**. Sản phẩm gồm thiết kế, source code, báo cáo hoàn chỉnh và link video YouTube ở chế độ Unlisted. Không còn mốc nộp Tuần 11 hoặc demo live Tuần 13.

## 1. Nội dung bắt buộc

- [ ] Class diagram Mermaid đã render và khớp source.
- [ ] Sequence diagram agent run đã render.
- [ ] Sequence diagram batch evaluation đã render.
- [ ] Component diagram đã render và không có dependency ngược sai layer.
- [ ] Source build được trên WSL/Linux.
- [ ] Báo cáo OOP có bằng chứng cho pattern và kỹ thuật C++.
- [ ] Báo cáo tools có canonical name, alias, args, policy, dependency và test.
- [ ] Báo cáo evaluation có run hỏng/run đạt, scoring, trajectory và failure taxonomy.
- [ ] README cho phép người mới build, cấu hình và chạy đúng executable.
- [ ] Link YouTube Unlisted mở được bằng cửa sổ không đăng nhập.

## 2. Kiểm chứng kỹ thuật

- [ ] `cmake -S . -B build` thành công.
- [ ] `cmake --build build -j2` thành công cho mọi target.
- [ ] `./build/test_harness` in `ALL HARNESS TESTS PASSED`.
- [ ] `./build/test_multi_agent` in `ALL PASSED`.
- [ ] Benchmark xác nhận cuối là run mới từ trạng thái sạch và được nhóm cho phép dùng quota.
- [ ] Run có 10 task: 4 simple, 4 medium, 2 hard.
- [ ] Task yêu cầu tool có tool step thật và phù hợp.
- [ ] Artifact được tạo đúng filename/content trong run hiện tại.
- [ ] Trajectory task 005 và 010 giữ args thật.
- [ ] Kết quả provider/model được ghi đúng, không lộ API key.
- [ ] Token bằng `0` được chú thích là chưa đo.

## 3. Review chéo

- [ ] A review sequence harness và nội dung LLM/AgentLoop trong README.
- [ ] B review tên tool, alias, args và policy trong báo cáo/README.
- [ ] C review số liệu benchmark, output path và lệnh chạy.
- [ ] Mọi link Markdown tương đối đều mở đúng file.
- [ ] Bốn Mermaid diagram render không lỗi.
- [ ] Không mô tả `VLMEvaluator` như tính năng hoàn chỉnh.
- [ ] Không mô tả multi-agent demo như benchmark harness integration.

## 4. Kiểm tra secret và artifact

- [ ] Không có `config.json` hoặc API key trong staged files.
- [ ] Không có `build/`, database hoặc file task phát sinh trong staged files.
- [ ] Không có run benchmark mới trong commit nếu nhóm chưa thống nhất lưu.
- [ ] ZIP không chứa `.git`, cache, compiler output hoặc secret.
- [ ] README chỉ dùng placeholder như `YOUR_API_HERE`.

Lệnh kiểm tra trước khi đóng gói:

```bash
git status --short
git diff --check
git ls-files config.json build memory.db notes.txt result.txt capital.txt calc.txt data.txt output.txt
```

## 5. Tên và nội dung ZIP

Tên đề xuất:

```text
MSSV1_MSSV2_MSSV3_OopAgent.zip
```

Theo kế hoạch, hậu tố `OopAgent` có thể bỏ nếu tên vẫn chứa đủ ba MSSV. ZIP phải có tối thiểu source, `CMakeLists.txt`, `benchmark/tasks.json`, tài liệu và README.

## 6. Xác nhận cuối

- [ ] Một thành viên giải nén ZIP vào thư mục mới.
- [ ] Build lại theo đúng README.
- [ ] Chạy `test_multi_agent` từ bản giải nén.
- [ ] Kiểm tra video, link nộp và quyền truy cập repository private.
- [ ] Nộp trước thời hạn, không đợi sát 21:00.
