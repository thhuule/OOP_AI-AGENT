# Project Status — AI-AGENT OOP 2026

Cập nhật lần cuối: 2026-08-05

> Đây là bộ nhớ ngắn của dự án. Khi bắt đầu phiên làm việc mới, đọc file này, kế hoạch tuần hiện tại và mục mới nhất trong `LICH_SU_LOI_THEO_TUAN_ROLE_ABC.md`.

## Giai đoạn hiện tại

Tuần 9.5 — hoàn thành UML/báo cáo draft, đóng blocker bắt buộc ở mức tối thiểu và chuẩn bị backlog Tuần 10.

## Trạng thái theo Role

### Role A — IN PROGRESS

- Environment hierarchy đã có và đã được Role C tích hợp vào Harness.
- Guarded C++26 fallback đã build được trên WSL bằng nhánh `std::vector`.
- Cần chờ B freeze Registry/Factory contract rồi audit/chốt bốn UML và `docs/report_oop_design.md`.
- Cần focused subclass test cho Template Method nếu muốn đóng claim bắt buộc.
- Parser/fallback/CMake/MSVC mở rộng không mặc định thuộc Tuần 9.5; chuyển Tuần 10 nếu không phải blocker tài liệu.

### Role B — BLOCKED

- Tool source và built-in registration đã có.
- Hai commit đã xác định: `f2cff55` thêm Registry/Factory source; `4a9f959` thêm `docs/report_tools.md`.
- Factory/alias/policy đã có source nhưng chưa có focused evidence đầy đủ và chưa chứng minh đường dùng runtime.
- `test_harness` segfault sau khi đi vào đường đăng ký Tool; nguyên nhân nghi ngờ nằm ở việc đọc `tool->get_name()` và move `tool` trong cùng lời gọi.
- `docs/report_tools.md` đã có nhưng còn mâu thuẫn về Factory/FileTool, thiếu nguồn ba nhóm tool và test evidence.
- Việc tiếp theo: sửa regression + test chống tái diễn trước, sau đó chốt contract và báo cáo.

### Role C — PARTIALLY DONE cho phạm vi Tuần 9.5

- Harness dùng `Environment` abstraction; Native chạy thật, Sandbox dùng trong focused test.
- Phần run/evidence/fallback trong `docs/report_evaluation.md` đã audit theo source và artifact.
- README đã phân biệt test offline, benchmark thật và fallback-assisted pipeline evidence.
- Submission checklist và video storyboard đã đối chiếu yêu cầu Tuần 12 và trajectory thật.
- C-9.5-01 đang chờ patch B; C chưa thể xác nhận lại toàn bộ offline integration gate.
- Checklist A/B/C được gom vào `KH_TUAN9.5.md`; không duy trì checklist riêng của C.

## Xác minh gần nhất

Ngày 2026-08-05:

- `cmake --build build -j2`: PASS, đủ 5 target.
- `./build/test_harness`: FAIL, pass hai fixture đầu rồi segfault trước fixture StepHook/tool registration.
- `./build/test_multi_agent`: `ALL PASSED`.
- `ctest --test-dir build --output-on-failure`: 1/2 PASS; `harness` segfault.
- Warning còn lại đến từ deprecated trait trong nlohmann vendored header; không có build error.
- Không chạy `run_eval` thật trong lượt này.

## Việc tiếp theo

1. Role B sửa regression đăng ký Tool và thêm focused Registry/Factory tests.
2. Role B freeze contract; Role A cập nhật UML/OOP report theo contract cuối.
3. Role C chạy lại toàn bộ offline integration gate và ghi exact result.
4. A/B/C review chéo tài liệu theo checklist trong `KH_TUAN9.5.md`.
5. Chỉ chạy benchmark provider thật khi offline gate pass và người dùng xác nhận quota/network/artifact.

## Blocked / Cần xác nhận

- Tool registration regression: chờ Role B patch và regression test.
- Benchmark sạch bằng provider thật: chờ offline gate, A/B freeze và xác nhận người dùng.
- Rubric có bắt buộc ToolRegistry dùng trực tiếp `Registry<T>` hay Factory hiện tại kèm test là đủ.
- Có môi trường MSVC để xác minh `/std:c++latest` cho mọi target hay không.
- Có chọn bonus Harness→MultiAgentRunner hay không; mặc định chưa làm.

## Backlog Tuần 10

- Parser/fallback refactor và focused regression tests.
- Failure taxonomy: rate limit, timeout, loop, incomplete task và parser signal.
- Token telemetry thật; không suy token từ độ dài chuỗi.
- Ghi nguồn action `llm` hoặc `fallback` vào trajectory.
- Artifact isolation nâng cao, sanitizer/Valgrind và bug-fix rộng.
- Benchmark sạch cuối cùng sau code freeze.

## Tài liệu cần đọc tiếp

- Kế hoạch chuyển tiếp: `filephanchiacv/KH_TUAN9.5.md`.
- Lịch sử thay đổi/lỗi: `filephanchiacv/LICH_SU_LOI_THEO_TUAN_ROLE_ABC.md`.
- Role C Environment fix: `filephanchiacv/ROLE_C_ENVIRONMENT_FIX_LOG_20260805.md`.
- Báo cáo Evaluation: `docs/report_evaluation.md`.
