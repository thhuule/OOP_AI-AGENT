# Project Status — AI-AGENT OOP 2026

Cập nhật lần cuối: 2026-08-06

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

### Role B — PARTIALLY DONE

- Tool source và built-in registration đã có.
- Main snapshot `e9e1d35` đã sửa registration bằng cách lấy tên Tool trước khi move.
- `benchmark/test_tools.cpp` build và pass các fixture instance registration, Factory create/fresh/unknown, alias, allow/deny và `register_all_tools`.
- Header/source thống nhất duplicate creator theo semantics overwrite; còn thiếu focused duplicate-creator test.
- `docs/report_tools.md` vẫn còn hai claim Factory stale và thiếu URL nguồn OpenClaw/Hermes trực tiếp.
- Error-path tests riêng cho Exec/Git/Web/Memory chưa có trong `test_tools.cpp`.

### Role C — DONE integration gate, PARTIALLY DONE documentation

- Harness dùng `Environment` abstraction; Native chạy thật, Sandbox dùng trong focused test.
- Phần run/evidence/fallback trong `docs/report_evaluation.md` đã audit theo source và artifact.
- README đã phân biệt test offline, benchmark thật và fallback-assisted pipeline evidence.
- Submission checklist và video storyboard đã đối chiếu yêu cầu Tuần 12 và trajectory thật.
- C-9.5-01 đã hoàn thành trên `e9e1d35`: Tool/Harness/multi-agent pass và CTest 3/3.
- C-9.5-02 còn review cuối sau khi B sửa report Tools stale và bổ sung link nguồn.
- Checklist A/B/C được gom vào `KH_TUAN9.5.md`; không duy trì checklist riêng của C.

## Xác minh gần nhất

Ngày 2026-08-06:

- `cmake -S . -B build`: configure thành công; lượt build đầu timeout, incremental build hoàn tất exit 0.
- Sáu target build: `OopAgent`, `run_eval`, `test_multi_agent`, `test_harness`, `test_tools`, `demo_multi_agent`.
- `./build/test_tools`: `ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY`.
- `./build/test_harness`: `ALL HARNESS TESTS PASSED`.
- `./build/test_multi_agent`: `ALL PASSED`.
- CTest: 3/3, 100% pass.
- Không chạy `run_eval` thật trong lượt này.

## Việc tiếp theo

1. Role B thêm duplicate-creator/error-path tests và sửa hai claim Factory stale + URL nguồn trong report Tools.
2. Role A cập nhật UML/OOP report theo Registry giữ `shared_ptr` và Factory trả `unique_ptr`.
3. Role C review cuối tài liệu Tools/evidence.
4. A/B/C review chéo tài liệu theo checklist trong `KH_TUAN9.5.md` và `docs/DOCUMENTATION_GUIDE.md`.
5. Chỉ chạy benchmark provider thật khi code/docs freeze và người dùng xác nhận quota/network/artifact.

## Blocked / Cần xác nhận

- Benchmark sạch bằng provider thật: offline gate đã pass; còn chờ A/B freeze và xác nhận người dùng.
- Report Tools còn claim stale/link nguồn; chưa đóng documentation DoD.
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
