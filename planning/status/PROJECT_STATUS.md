# Project Status — AI-AGENT OOP 2026

Cập nhật lần cuối: 2026-08-06

> Đây là bộ nhớ ngắn của dự án. Khi bắt đầu phiên làm việc mới, đọc file này, kế hoạch tuần hiện tại và mục mới nhất trong `../history/LICH_SU_LOI_THEO_TUAN_ROLE_ABC.md`.

## Giai đoạn hiện tại

Tuần 9.5 — hoàn thành UML/báo cáo draft, đóng blocker bắt buộc ở mức tối thiểu và chuẩn bị backlog Tuần 10.

## Trạng thái theo Role

### Role A — DONE core, pending documentation commit

- Environment hierarchy đã có và đã được Role C tích hợp vào Harness.
- Guarded C++26 fallback đã build được trên WSL bằng nhánh `std::vector`.
- UML, report OOP, ownership Registry/Factory và MSVC flags đã được Role A sửa; focused Template Method test pass.
- Worktree còn thay đổi `component_diagram.md` và xóa checklist trùng chưa commit; Mermaid render vẫn là bước đóng tài liệu.
- Parser/fallback/CMake/MSVC mở rộng không mặc định thuộc Tuần 9.5; chuyển Tuần 10 nếu không phải blocker tài liệu.

### Role B — DONE core, PARTIALLY DONE documentation/error matrix

- Tool source và built-in registration đã có.
- Main snapshot `e9e1d35` đã sửa registration bằng cách lấy tên Tool trước khi move.
- `benchmark/test_tools.cpp` pass instance registration, Factory create/fresh/unknown, alias, allow/deny, duplicate-creator overwrite và `register_all_tools`.
- Factory/File/alias/policy claim stale đã được sửa trong `docs/report_tools.md`.
- URL nguồn OpenClaw/Hermes vẫn chưa có; report còn viện dẫn test trên HEAD cũ `86c7d49`.
- Error-path fixture đã có cho Exec/Git/Json/Memory; Web/timeout path được ghi backlog Tuần 10.

### Role C — DONE phần có thể thực hiện, BLOCKED benchmark thật

- Harness dùng `Environment` abstraction; Native chạy thật, Sandbox dùng trong focused test.
- Phần run/evidence/fallback trong `docs/report_evaluation.md` đã audit theo source và artifact.
- README đã phân biệt test offline, benchmark thật và fallback-assisted pipeline evidence.
- Submission checklist và video storyboard đã đối chiếu yêu cầu Tuần 12 và trajectory thật.
- C-9.5-01 đã xác minh lại trên HEAD `08202f5`: bảy target build, bốn executable test pass và CTest 4/4.
- C-9.5-02 đã review lại sau Role B: claim Factory/File/alias/policy đã sửa; còn URL nguồn, HEAD evidence và Web/timeout matrix được chuyển đúng owner/backlog.
- C-9.5-03 chưa chạy vì cấu hình dùng provider thật (`use_mock=false`) và chưa có xác nhận quota/network/artifact.
- Checklist A/B/C được gom vào `KH_TUAN9.5.md`; không duy trì checklist riêng của C.

## Xác minh gần nhất

Ngày 2026-08-06:

- `cmake -S . -B build` và `cmake --build build -j2`: thành công trên HEAD `08202f5`.
- Bảy target build: `OopAgent`, `run_eval`, `test_multi_agent`, `test_harness`, `test_tools`, `test_template_method`, `demo_multi_agent`.
- `./build/test_tools`: `ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY`.
- `./build/test_harness`: `ALL HARNESS TESTS PASSED`.
- `./build/test_multi_agent`: `ALL PASSED`.
- `./build/test_template_method`: `ALL ROLE A TESTS PASSED SUCCESSFULLY`.
- CTest: 4/4, 100% pass.
- Không chạy `run_eval` thật trong lượt này.

## Việc tiếp theo

1. Role B bổ sung URL nguồn trực tiếp, cập nhật HEAD evidence và để Web/timeout tests trong backlog Tuần 10 nếu chưa cần.
2. Role A commit hai thay đổi tài liệu đang ở worktree và cung cấp render evidence khi đóng gói.
3. Chỉ chạy benchmark provider thật khi code/docs freeze và người dùng xác nhận quota/network/artifact.

## Blocked / Cần xác nhận

- Benchmark sạch bằng provider thật: offline gate đã pass; còn chờ A/B freeze và xác nhận người dùng.
- Report Tools còn thiếu link nguồn trực tiếp và dùng HEAD evidence cũ; Web/timeout error paths chưa có focused test.
- Tám artifact root đã được đánh dấu xóa và thêm ignore rule; cần commit để chúng thực sự rời Git.
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

- Kế hoạch chuyển tiếp: [`../weekly-plans/KH_TUAN9.5.md`](../weekly-plans/KH_TUAN9.5.md).
- Lịch sử thay đổi/lỗi: [`../history/LICH_SU_LOI_THEO_TUAN_ROLE_ABC.md`](../history/LICH_SU_LOI_THEO_TUAN_ROLE_ABC.md).
- Role C Environment fix: ghi nhận trong lịch sử lỗi; file log riêng không có trong worktree hiện tại.
- Báo cáo Evaluation: `docs/report_evaluation.md`.
