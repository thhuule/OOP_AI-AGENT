# KH_TUAN10 — AI-AGENT OOP 2026

Ngày chuẩn hóa: 2026-08-07  
Mục tiêu: bug fix, polish, video/slide và chuẩn bị đóng gói Tuần 12.  
Tuần 10 là tuần cuối để sửa bug; Tuần 11 freeze nội bộ; Tuần 12 nộp bài.

> **Quy ước:** `[x]` = đã có bằng chứng; `[ ]` = chưa đóng; `CẦN XÁC NHẬN` = chưa đủ thông tin để tự quyết định. Không gọi benchmark 10/10 là bằng chứng model reasoning khi AgentLoop còn deterministic fallback.

## 1. Nguồn và trạng thái đầu tuần

### 1.1 Nguồn đối chiếu

- `KH_TUAN9.5.md` — checklist gần nhất và backlog đã chốt.
- `KH_Tuan9_ChiTiet.md` — yêu cầu bắt buộc, Definition of Done và cấu trúc role.
- `../history/FIX_LOI_ROLE_ABC_BENCHMARK_TUAN89.md` — danh sách lỗi gốc; phần 3/10 là lịch sử, không phải trạng thái hiện tại.
- `docs/report_oop_design.md`, `docs/report_tools.md`, `docs/report_evaluation.md`, `README.md`.
- Source/test hiện tại và CMake trên HEAD `f5af96c`.
- Clean benchmark hiện tại: `benchmark/results/run_20260807_085427_143/`.

### 1.2 Trạng thái đã có bằng chứng

| Hạng mục | Bằng chứng | Trạng thái | Phần còn lại |
|---|---|---|---|
| Offline integration | 7 target build; 4 executable; CTest 4/4 | `DONE` | Xác minh lại khi đóng gói |
| Benchmark provider thật | Run `run_20260807_085427_143`, 10/10 task PASS | `DONE` | Công khai fallback limitation; không suy ra model reasoning |
| Role A report/ownership | UML/report đã sửa các claim chính | `PARTIALLY DONE` | Chưa có MSVC build thật; parser/telemetry cần audit tiếp |
| Role B Registry/Factory | Duplicate creator, alias, policy và registration tests pass | `DONE` | Web/timeout focused test |
| Role B report | URL OpenClaw/Hermes đã bổ sung trong `a0a5012` | `PARTIALLY DONE` | Một số câu evidence còn ghi HEAD `a679a54` |
| Harness clean state | Cleanup và failure fixtures đã có | `DONE` | Kiểm tra artifact trước gói nộp |
| Video/đóng gói | Kịch bản và ZIP checklist cần hoàn thiện | `IN PROGRESS` | Link video, dry-run ZIP và owner confirmation |

### 1.3 Nguyên tắc kế thừa

- Không tạo lại task đã đóng ở Tuần 9.5 nếu không có regression.
- Các mục Web/timeout, MSVC, token telemetry, sanitizer, video và đóng gói được giữ vì còn tồn đọng hoặc được yêu cầu trong Tuần 9/9.5.
- `FIX_LOI_ROLE_ABC_BENCHMARK_TUAN89.md` được xem là nhật ký lỗi lịch sử; các mục đã được sửa chỉ dùng làm căn cứ audit, không đánh dấu lại là lỗi hiện tại.

## 2. Task Role A — Systems/Core

### A-10-01 — Chốt report OOP và bằng chứng build đa compiler

- **Task ID:** `A-10-01`.
- **Mục tiêu:** bảo đảm UML/report OOP không còn claim sai về ownership, Environment, Adapter, test path và CMake.
- **Lý do cần làm:** Tuần 9.5 đã sửa các claim chính nhưng chưa có build MSVC thật; `docs/bao_cao_du_an.md` còn limitation cần giữ đúng.
- **Input/dependency:** `docs/report_oop_design.md`, `docs/class_diagram.md`, `docs/component_diagram.md`, `CMakeLists.txt`.
- **Các bước chính:** đối chiếu source → UML → report; kiểm tra 7 target có `/std:c++latest`; nếu có máy MSVC thì configure/build; nếu không, ghi limitation.
- **Deliverable:** report OOP, UML và bảng limitation khớp source; log MSVC hoặc ghi `CẦN XÁC NHẬN`.
- **Acceptance criteria:** không còn link test tới `src/tests`; không mô tả `SharedToolWrapper` là class thực; Environment dùng `shared_ptr`; không tuyên bố MSVC pass khi chưa chạy.
- **Owner/phối hợp:** A chủ trì; B cung cấp ownership contract; C review claim benchmark.
- **Ưu tiên:** P1.
- **Có thể dời:** Có, nếu không có môi trường MSVC; phải giữ limitation trong báo cáo.

### A-10-02 — Audit parser và deterministic fallback

- **Task ID:** `A-10-02`.
- **Mục tiêu:** model không kết thúc bằng planning text khi task yêu cầu tool; parser nhận đúng protocol hiện tại.
- **Lý do cần làm:** danh sách lỗi Tuần 8/9 nêu các dạng `JSON`, fenced JSON, `ACTION:`, `call:provider:tool{...}` và Gemini `functionCall`.
- **Input/dependency:** `src/agent/agent_loop.cpp`, `src/client/gemini_client.cpp`, `FIX_LOI_ROLE_ABC_BENCHMARK_TUAN89.md`.
- **Các bước chính:** kiểm tra system prompt canonical tool; test raw/fenced JSON, ACTION, provider-call và malformed retry; kiểm tra function-call normalization; không đưa Harness dependency vào AgentLoop.
- **Deliverable:** focused parser tests hoặc log kiểm chứng; report nêu rõ fallback limitation.
- **Acceptance criteria:** tool-intent sai format không bị coi là final answer; tool call hợp lệ được thực thi; fallback được ghi rõ là pipeline aid, không phải model reasoning.
- **Owner/phối hợp:** A chủ trì; B xác nhận canonical tool names; C kiểm tra trajectory.
- **Ưu tiên:** P0.
- **Có thể dời:** Không nếu còn lỗi parser; chỉ dời phần parser format chưa được backend sử dụng sau khi ghi limitation.

### A-10-03 — Failure taxonomy và token telemetry

- **Task ID:** `A-10-03`.
- **Mục tiêu:** trajectory phân biệt lỗi parser/tool/loop/rate-limit/timeout và không coi `tokens=0` là model không dùng token.
- **Lý do cần làm:** Tuần 9 yêu cầu failure taxonomy; report hiện ghi token chưa đo được.
- **Các bước chính:** kiểm tra các mã `PARSER_FAIL`, `TOOL_NOT_FOUND`, `INVALID_ARGS`, `LOOP_DETECTED`, `NO_TOOL_EXECUTION`, `RATE_LIMIT`, `TIMEOUT`; nếu client trả usage thì truyền vào trajectory; nếu chưa có metadata thì ghi `not measured`.
- **Deliverable:** schema/summary cập nhật và test hoặc log cho từng nhánh có thể chạy offline.
- **Acceptance criteria:** failure reason không bị bỏ trống khi có lỗi; token zero được giải thích đúng; không tự tạo số token.
- **Owner/phối hợp:** A và C.
- **Ưu tiên:** P1.
- **Có thể dời:** Token provider metadata có thể dời Tuần 11 nếu API client chưa cung cấp.

### A-10-04 — Sanitizer và regression check

- **Task ID:** `A-10-04`.
- **Mục tiêu:** tìm memory leak/undefined behavior trong phần core và test.
- **Các bước chính:** build với AddressSanitizer trên WSL; chạy `test_harness`, `test_tools`, `test_multi_agent`, `test_template_method`; lưu log.
- **Deliverable:** sanitizer log hoặc limitation nếu toolchain không hỗ trợ.
- **Acceptance criteria:** không có lỗi sanitizer mới; mọi failure phải có owner và reason.
- **Owner/phối hợp:** A chủ trì; cả nhóm xử lý regression.
- **Ưu tiên:** P1.
- **Có thể dời:** Có nếu WSL/compiler không hỗ trợ sanitizer; phải ghi `CẦN XÁC NHẬN`.

### A-10-05 — Kiểm tra C++26 feature và fallback portable

- **Task ID:** `A-10-05`.
- **Mục tiêu:** giữ yêu cầu C++26 nhưng không làm hỏng compiler chưa có `<inplace_vector>`.
- **Lý do cần làm:** `FIX_LOI_ROLE_ABC_BENCHMARK_TUAN89.md` yêu cầu guarded fallback nếu source dùng `std::inplace_vector`.
- **Các bước chính:** tìm include/feature C++26 trong source; nếu dùng `inplace_vector`, dùng `__has_include`/feature macro và fallback có `reserve`; nếu không dùng, ghi rõ feature C++26 thực tế trong ma trận report.
- **Deliverable:** source/comment/report hoặc ghi nhận `CẦN XÁC NHẬN` nếu chưa có feature cần fallback.
- **Acceptance criteria:** GCC/WSL hiện tại build được; không include header C++26 không tồn tại vô điều kiện; report không nhận vơ feature.
- **Owner/phối hợp:** A; C kiểm tra build.
- **Ưu tiên:** P1.
- **Có thể dời:** Có nếu source không dùng `inplace_vector`, nhưng phải đóng bằng kết luận audit.

### Checklist Role A

- [x] Ownership/Environment/Template Method claims chính đã được sửa trong report.
- [x] CMake có `/std:c++latest` cho 7 target ở source hiện tại.
- [ ] Parser variants và malformed tool-intent có focused evidence đầy đủ.
- [ ] Skill được chọn từ `SkillLoader` được inject vào system prompt trước mỗi run hoặc limitation được ghi rõ.
- [ ] Failure taxonomy có log/test cho các nhánh cần thiết.
- [ ] Token telemetry được đo hoặc ghi rõ `not measured`.
- [ ] Sanitizer chạy và log được lưu.
- [ ] C++26 feature/fallback được audit; nếu không dùng `inplace_vector` thì ghi rõ feature thực tế.
- [ ] MSVC configure/build được xác minh hoặc ghi limitation.

## 3. Task Role B — Tools/Data

### B-10-01 — Chuẩn hóa tool description và canonical names

- **Task ID:** `B-10-01`.
- **Mục tiêu:** prompt chỉ thấy tên tool thật và ví dụ args dùng được.
- **Lý do cần làm:** file lỗi lịch sử ghi model hay sinh `create_file`, `google_search`, `python_interpreter`; canonical hiện tại là `write_file`, `web_search`, `execute_shell`.
- **Các bước chính:** rà `get_name()`/`get_description()` trong `src/tools`; lập bảng canonical/alias; không expose tool không đăng ký; thêm ví dụ string và JSON args.
- **Deliverable:** bảng tool trong `docs/report_tools.md` và description source khớp nhau.
- **Acceptance criteria:** không còn description gọi tên tool không tồn tại; alias có test; AgentLoop không hardcode concrete tool.
- **Owner/phối hợp:** B chủ trì; A review prompt; C review report.
- **Ưu tiên:** P1.
- **Có thể dời:** Không nếu phát hiện mismatch làm benchmark fail.

### B-10-02 — Args parsing đa format

- **Task ID:** `B-10-02`.
- **Mục tiêu:** file/calculator/exec/memory xử lý args rõ ràng và trả `ToolError` thay vì throw.
- **Các bước chính:** test `result.txt,1081`; JSON `{path,content}`; JSON `{filename,content}`; trim calculator; policy Exec; mode Memory save/search; kiểm tra `std::unexpected` cho invalid input.
- **Deliverable:** focused tests và bảng args/error trong report.
- **Acceptance criteria:** valid args tạo đúng artifact; invalid args có lỗi phân loại; không raw `new/delete`.
- **Owner/phối hợp:** B chủ trì; C kiểm tra action-level evidence.
- **Ưu tiên:** P1.
- **Có thể dời:** Chỉ dời format không được benchmark sử dụng sau khi ghi limitation.

### B-10-03 — Web/timeout focused tests

- **Task ID:** `B-10-03`.
- **Mục tiêu:** đóng phần error matrix còn thiếu mà Tuần 9.5 đã ghi backlog.
- **Các bước chính:** mock network failure/HTTP error/timeout cho WebSearchTool; timeout/exit-code/cấm lệnh cho ExecTool; không gọi network thật trong unit test.
- **Deliverable:** `test_tools`/test fixture và report §25 cập nhật.
- **Acceptance criteria:** lỗi trả về rõ, test ổn định offline, không làm CTest phụ thuộc mạng.
- **Owner/phối hợp:** B chủ trì; C review taxonomy.
- **Ưu tiên:** P1.
- **Có thể dời:** Có, nếu mock network chưa sẵn; ghi limitation và chuyển Tuần 11.

### B-10-04 — Đồng bộ evidence report Tools

- **Task ID:** `B-10-04`.
- **Mục tiêu:** mọi claim test trong `docs/report_tools.md` chỉ dẫn đúng revision và artifact hiện tại.
- **Lý do cần làm:** URL OpenClaw/Hermes đã có ở `a0a5012`, nhưng một số mục còn ghi HEAD `a679a54`.
- **Các bước chính:** thay evidence cũ bằng `f5af96c` hoặc ghi rõ historical revision; cập nhật test matrix; giữ URL tham chiếu thật; không gọi Web/timeout đã pass khi chưa có fixture.
- **Deliverable:** report Tools và lịch sử checklist khớp nhau.
- **Acceptance criteria:** không còn claim stale; source URL mở được; limitation Web/timeout và fallback được ghi đúng.
- **Owner/phối hợp:** B chủ trì; A/C review.
- **Ưu tiên:** P1.
- **Có thể dời:** Có, sang đầu Tuần 11 nếu code evidence đã freeze.

### B-10-05 — Memory database lifecycle và artifact packaging

- **Task ID:** `B-10-05`.
- **Mục tiêu:** `memory.db` không bị đưa nhầm vào commit/ZIP và lifecycle của MemoryTool được giải thích.
- **Các bước chính:** kiểm tra file có sinh khi test không; giữ `memory.db` trong `.gitignore`; kiểm tra `git ls-files`; không xóa dữ liệu người dùng nếu chưa xác nhận; cập nhật ZIP checklist.
- **Deliverable:** log `git ls-files`/ignore và hướng dẫn đóng gói.
- **Acceptance criteria:** package không chứa database sinh ra; nếu file tracked thì có owner confirmation trước khi remove khỏi Git.
- **Owner/phối hợp:** B kiểm tra lifecycle; C đóng gói; người dùng xác nhận file cần giữ.
- **Ưu tiên:** P1.
- **Có thể dời:** Không qua mốc freeze Tuần 11.

### Checklist Role B

- [x] Registry/Factory, alias, policy và duplicate creator đã có evidence.
- [x] URL OpenClaw/Hermes đã có trong report.
- [ ] Tool description/args đa format được test độc lập.
- [ ] Web/timeout mock fixture pass offline.
- [ ] Report Tools đồng bộ evidence sang HEAD `f5af96c`.
- [ ] `memory.db` lifecycle và packaging được xác nhận.

## 4. Task Role C — Eval/Infra/Submission

### C-10-01 — Đóng evidence benchmark thật

- **Task ID:** `C-10-01`.
- **Mục tiêu:** lưu run sạch và phân biệt evaluator score với action-level/pipeline evidence.
- **Bằng chứng hiện có:** `run_20260807_085427_143`, 10/10 task PASS, 10 trajectory files.
- **Các bước chính:** giữ summary/run ID; kiểm tra category 4/4/2; kiểm tra tool steps/artifact; ghi fallback limitation; không dùng run cũ thay run mới. Nếu cần artifact tổng hợp riêng, tạo `eval_results_FINAL.json` từ run mới chỉ sau khi owner xác nhận; không tự commit bản sao generated này.
- **Deliverable:** `docs/report_evaluation.md`, README/checklist và link run.
- **Acceptance criteria:** run ID/model/provider khớp artifact; không lộ API key; 10/10 chỉ gọi pipeline evidence nếu fallback có thể tham gia.
- **Owner/phối hợp:** C chủ trì; A/B review.
- **Ưu tiên:** P0.
- **Trạng thái:** `[x]` run đã có; còn kiểm tra đóng gói evidence.

### C-10-02 — Clean state và action-level score

- **Task ID:** `C-10-02`.
- **Mục tiêu:** artifact cũ không tạo false pass; task cần tool phải có tool step thật.
- **Các bước chính:** kiểm tra `HarnessRunner::cleanArtifacts`; đối chiếu `requires_tool`, `category`, `tool_steps_count`, `failure_reason`, `evaluator_score`, `action_level_score`; kiểm tra post-condition trong run hiện tại. Chạy `test_multi_agent` và `demo_multi_agent` khi đóng regression; nếu demo sinh `report.txt`, kiểm tra dữ liệu mong đợi như `1081`, `Tokyo` và không nhầm demo với benchmark đơn-agent.
- **Deliverable:** test/log và bảng failure taxonomy.
- **Acceptance criteria:** task cần tool nhưng `steps=[]` bị đánh dấu `NO_TOOL_EXECUTION`; FunctionalEvaluator không pass do artifact cũ; cleanup không đụng source/config/report.
- **Owner/phối hợp:** C chủ trì; B kiểm tra tool result; A kiểm tra parser.
- **Ưu tiên:** P0.
- **Có thể dời:** Không nếu có benchmark mới hoặc trước khi đóng report.

### C-10-03 — Video demo và slide

- **Task ID:** `C-10-03`.
- **Mục tiêu:** tạo video 5–7 phút và slide theo đúng artifact hiện tại.
- **Các bước chính:** quay build; chạy một task CLI; trình bày benchmark JSON/summary; chạy multi-agent demo; giải thích StepHook/Observer hoặc pattern khác; không đọc API key trên màn hình.
- **Deliverable:** storyboard, file slide, video YouTube Unlisted và link.
- **Acceptance criteria:** link hoạt động; số liệu/video khớp run hiện tại; không mô tả fallback là reasoning; không dùng mốc demo live Tuần 13 đã hủy.
- **Owner/phối hợp:** C chủ trì; A/B review nội dung kỹ thuật.
- **Ưu tiên:** P0.
- **Cần xác nhận:** tài khoản upload và link video.

### C-10-04 — ZIP/package checklist Tuần 12

- **Task ID:** `C-10-04`.
- **Mục tiêu:** tạo gói nộp sạch, không secret/build/database/artifact thừa.
- **Các bước chính:** dry-run từ worktree sạch; gồm `src/`, `benchmark/tasks.json`, `docs/`, `skills/`, `CMakeLists.txt`, README, `config.json.example`, `.gitignore`; loại `build/`, `config.json`, `memory.db`, `*.txt` artifact, `.git` và cache.
- **Deliverable:** checklist đóng gói và danh sách file trước khi ZIP.
- **Acceptance criteria:** không có API key; không mất tài liệu lịch sử cần nộp; tên ZIP đúng quy ước MSSV.
- **Owner/phối hợp:** C chủ trì; A/B xác nhận file thuộc role; người dùng xác nhận tên MSSV.
- **Ưu tiên:** P0.
- **Cần xác nhận:** tên ZIP cuối và file nào bắt buộc phải giữ.

### C-10-05 — Review chéo và freeze nội bộ

- **Task ID:** `C-10-05`.
- **Mục tiêu:** mọi claim/code/docs dùng chung được A/B/C review trước Tuần 11.
- **Các bước chính:** rà README, 4 UML, report OOP/Tools/Eval, submission checklist; kiểm tra git diff/status; ghi owner cho file đổi/xóa; chạy build/CTest lần cuối.
- **Deliverable:** review log, commit list và freeze note.
- **Acceptance criteria:** không còn dependency BLOCKED bắt buộc; mỗi thành viên có commit; thay đổi ngoài scope được xác nhận.
- **Owner/phối hợp:** cả nhóm; C điều phối.
- **Ưu tiên:** P0.
- **Có thể dời:** Không qua mốc Tuần 11 freeze.

### Checklist Role C

- [x] Clean provider benchmark 10/10 đã tạo và ghi rõ pipeline evidence.
- [x] Offline build/test gate đã pass 7 target, CTest 4/4.
- [ ] Action-level score/failure taxonomy được kiểm tra trên run cuối.
- [ ] Video/slide hoàn tất và link được xác nhận.
- [ ] ZIP dry-run không chứa secret/build/database/artifact.
- [ ] Review chéo và freeze note hoàn tất.

## 5. Dependency và thứ tự thực hiện

| Thứ tự | Task | Owner | Phụ thuộc | Bằng chứng bàn giao |
|---:|---|---|---|---|
| 1 | A-10-01, B-10-04 | A/B | HEAD `f5af96c` | UML/report/evidence đồng bộ |
| 2 | A-10-02, B-10-01, B-10-02 | A/B | Canonical tool contract | Parser/tool fixture |
| 3 | B-10-03, A-10-03, A-10-04 | A/B | Offline test/build | Error/telemetry/sanitizer log |
| 4 | C-10-02 | C | A/B parser/tool contract | Run summary không false pass |
| 5 | C-10-03 | C | Evidence benchmark đã freeze | Video/slide/link |
| 6 | B-10-05, C-10-04 | B/C | Owner confirmation | ZIP file list |
| 7 | C-10-05 | Cả nhóm | Tất cả task trên | Freeze note + commit list |

## 6. Definition of Done Tuần 10

### Code và test

- [x] Không có conflict marker trong source/build files ở lần audit HEAD `f5af96c`.
- [x] Build 7 target và CTest 4/4 pass trên WSL/Linux.
- [ ] Parser/tool-intent test pass hoặc limitation được ghi rõ.
- [ ] Web/timeout fixture pass offline hoặc được đánh dấu backlog có owner.
- [ ] Sanitizer chạy hoặc ghi `CẦN XÁC NHẬN` do toolchain.
- [ ] MSVC build được xác minh hoặc report ghi rõ chưa có môi trường.

### Benchmark và report

- [x] Có run provider thật mới với đủ 10 task.
- [x] Summary tách evaluator score/action-level score và công khai fallback limitation.
- [ ] Failure reason và post-condition được kiểm tra trên run cuối.
- [ ] Report Tools không còn HEAD evidence stale.
- [ ] Report Eval/README dùng đúng chữ “pipeline evidence”, không quảng cáo model reasoning.

### Tài liệu, video và đóng gói

- [ ] 4 UML và 3 report chính được review chéo.
- [ ] Video Unlisted và slide có link hoạt động.
- [ ] ZIP dry-run không chứa API key, `config.json`, `build/`, database, artifact hoặc `.git`.
- [ ] Mỗi thành viên có ít nhất 1 commit và owner xác nhận file đổi/xóa.
- [ ] Checklist Tuần 12 đúng deadline, không khôi phục mốc demo live Tuần 13.

## 7. Các mục cần xác nhận

| Mục | Câu hỏi | Người xác nhận |
|---|---|---|
| MSVC | Có máy/môi trường MSVC để chạy configure/build thật không? | A/người dùng |
| Token | Client có được phép/khả năng trả usage metadata không? | A |
| Video | Ai upload video và URL YouTube Unlisted nào là URL chính thức? | C/người dùng |
| ZIP | Tên ZIP theo 3 MSSV và file nào bắt buộc giữ? | C/người dùng |
| Web mock | Có chấp nhận dời Web/timeout test sang Tuần 11 nếu chưa có mock network không? | B/C |

## 8. Lịch Tuần 11–12

### Tuần 11 — Freeze nội bộ

- Không thêm feature mới.
- Review 4 UML, 3 report, README và checklist lần cuối.
- Dựng/chỉnh video; chạy quy trình đóng gói từ môi trường sạch.
- Chỉ sửa blocker build/test/secret/package; mọi việc khác ghi backlog.

### Tuần 12 — Nộp bài

- Class Diagram, Sequence Diagram, Component Diagram.
- Source code, README, báo cáo và slide.
- Link YouTube Unlisted.
- ZIP đúng tên, không secret/build/artifact cấm.
- Nộp trước deadline chính thức của nhóm/đề bài.

> Không tự khôi phục mốc demo live Tuần 13 hoặc deadline Tuần 11 đã bị hủy; nếu đề/giảng viên thay đổi, ghi thành mục `CẦN XÁC NHẬN` mới.

## 9. Nhánh tùy chọn — Bonus tối đa +4 điểm

> Chỉ bắt đầu sau khi các bug bắt buộc, Definition of Done Tuần 10, review chéo và đóng gói cơ bản đã hoàn tất. Bonus không được làm chậm việc freeze hoặc nộp bài.

| Mục | Nội dung | Owner | Trạng thái | Tiêu chí nghiệm thu |
|---|---|---|---|---|
| Bonus +1 | **CẦN XÁC NHẬN** — hạng mục bonus số 1 theo rubric/đề bài | CẦN XÁC NHẬN | `NOT STARTED` | CẦN XÁC NHẬN |
| Bonus +1 | **CẦN XÁC NHẬN** — hạng mục bonus số 2 theo rubric/đề bài | CẦN XÁC NHẬN | `NOT STARTED` | CẦN XÁC NHẬN |
| Bonus +1 | **CẦN XÁC NHẬN** — hạng mục bonus số 3 theo rubric/đề bài | CẦN XÁC NHẬN | `NOT STARTED` | CẦN XÁC NHẬN |
| Bonus +1 | **CẦN XÁC NHẬN** — hạng mục bonus số 4 theo rubric/đề bài | CẦN XÁC NHẬN | `NOT STARTED` | CẦN XÁC NHẬN |

### Điều kiện mở khóa bonus

- [ ] Offline build/test vẫn pass sau khi hoàn thành bug fix.
- [ ] Không còn blocker bắt buộc trong Definition of Done.
- [ ] A/B/C đã review chéo và có commit ổn định.
- [ ] Đã xác nhận rubric chính thức: “+4 điểm” gồm đúng bốn hạng mục nào.
- [ ] Có owner và deadline cho từng bonus.
- [ ] Nếu bonus ảnh hưởng source, phải có focused test và cập nhật tài liệu tương ứng.
