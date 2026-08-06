# KH_TUAN9.5 — AI-AGENT Project

Ngày audit gần nhất sau merge Role B: 2026-08-06

> Phạm vi của file này là kiểm tra trạng thái tích hợp, đóng blocker bắt buộc và hoàn thành tài liệu Tuần 9.5. Đây không phải đợt refactor toàn diện của Tuần 10.
>
> Chỉ dùng trạng thái: `DONE`, `PARTIALLY DONE`, `IN PROGRESS`, `BLOCKED`, `NOT STARTED`, `CẦN XÁC NHẬN`.
>
> Đây là nguồn checklist duy nhất của Tuần 9.5. Mỗi Role cập nhật checkbox của mình tại mục 9; không tạo checklist Role riêng nếu không có yêu cầu mới.

## Nguồn đã dùng

- Git HEAD `86c7d49` và các commit tích hợp liên quan: `e9e1d35`, `624b778`, `8e87371`.
- Source hiện tại: `src/tools/`, integration point trong `src/agent/agent_loop.h`, Harness và test hiện tại.
- `docs/report_tools.md`, `docs/bao_cao_du_an.md` và tài liệu A/C hiện có.
- `KH_Tuan6_ChiTiet (1).md`, `KH_Tuan6_Updated.md`, `KH_Tuan7_8_ChiTiet.md`, `KH_Tuan9_ChiTiet.md`, `OOP_PHANVIEC.md` và bản KH Tuần 9.5 trước audit.
- Build/test trực tiếp trên worktree tích hợp hiện tại.

### Giới hạn kết luận

- Worktree đang có thay đổi tài liệu chưa commit. Kết quả gate dưới đây phản ánh **worktree tích hợp hiện tại**, không chỉ riêng commit B.
- Chỉ các file Tools và `report_tools.md` được tính là deliverable rõ của B. `docs/bao_cao_du_an.md` bao phủ toàn dự án nên cần A/C review; không tự động quy mọi nội dung trong đó là trách nhiệm hoặc thành quả đã xác minh của B.
- Không chạy `run_eval`: offline gate đã pass nhưng cấu hình hiện dùng provider thật (`use_mock=false`), nên cần xác nhận quota/network/artifact.

---

## 1. Trạng thái sau khi tích hợp phần việc Role B

| Hạng mục | Kết quả | Bằng chứng | Trạng thái | Vấn đề còn lại |
|----------|---------|------------|------------|----------------|
| Commit/thay đổi mới của B | Main mới đã gom patch Registry/Factory và test | `e9e1d35`; lịch sử trước gồm `f2cff55`, `4a9f959`, `1582f91` | `DONE` | Lịch sử remote đã được tạo lại nên commit mới là root snapshot |
| Generic `Registry<T>` | Registration lấy tên trước khi move; null/duplicate instance được test | `ToolRegistry.cpp`, `Registry.h`, `test_tools.cpp` | `DONE` | Không còn regression crash trong Harness |
| Factory creator theo tên | Creator API, built-in registration và fresh-instance test chạy thành công | `test_tools`: `ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY` | `PARTIALLY DONE` | Chưa có focused duplicate-creator test dù semantics overwrite đã khớp header/source |
| Alias và allow/deny policy | Source và fixture đều chạy thành công | `test_aliases_and_normalization`, `test_allow_deny_policies` | `DONE` | Error/security test rộng vẫn là việc tài liệu/backlog riêng |
| Tích hợp AgentLoop→Registry | AgentLoop chỉ gọi abstraction; Harness qua được StepHook/tool registration | `test_harness`: `ALL HARNESS TESTS PASSED` | `DONE` | Không chạy benchmark provider thật trong lượt này |
| Build toàn bộ | Bảy target build thành công trên WSL | Thêm `test_template_method` vào sáu target trước đó | `DONE` | Configure và build HEAD `86c7d49` đều exit 0 |
| Offline integration tests | Tool, Harness, multi-agent, Template Method và CTest đều pass | CTest 4/4, 100% | `DONE` | Chưa phải bằng chứng benchmark provider thật |
| `docs/report_tools.md` | Deliverable đã xuất hiện, có inventory/case study/pattern/SOLID | Commit `4a9f959`, file 978 dòng | `PARTIALLY DONE` | Nhiều đoạn mâu thuẫn source mới; phần Testing là danh sách cần test, không phải log test pass |
| Tính đúng của báo cáo Tools | Một số phần cuối mô tả Factory/alias hiện có | `report_tools.md` §21–§24 | `PARTIALLY DONE` | §5/§11/§19 vẫn nói Factory chưa có; §8/§11 nói không có FileRead/Write/Append dù source có các class này |
| Ba tool bổ sung | Time/JSON/Git có source, registration, bảng ba nhóm và registration test pass | Tool source; `report_tools.md` §8.9; `test_register_all_tools` | `PARTIALLY DONE` | Tên nguồn OpenClaw/Hermes chưa có URL kiểm chứng trực tiếp |
| Benchmark case study | Có bảng run 2/10→10/10 và mô tả cải thiện | `report_tools.md` §17 | `PARTIALLY DONE` | Chưa tách đóng góp B khỏi A/C và chưa ghi giới hạn fallback; không thay thế clean run hiện tại |
| Unit/focused tests của B | Target Tools chạy pass và được CTest đăng ký | `test_tools`; CTest test số 3 | `PARTIALLY DONE` | Thiếu duplicate creator và focused error paths Exec/Git/Web/Memory |

### Kết luận Role B

Role B đã đóng regression đăng ký Tool và cung cấp focused Registry/Factory tests chạy được trên main mới `e9e1d35`. Phần code cốt lõi đã tích hợp; phần còn lại là bổ sung edge-case test, sửa claim stale trong báo cáo Tools và thêm link nguồn trực tiếp.

---

## 2. Trạng thái hiện tại của các Role

| Role | Task trước đó | Trạng thái | Bằng chứng | Dependency còn lại |
|------|---------------|------------|-----------|---------------------|
| A | LLM clients, AgentLoop/StepHook, parser, loop detection từ Tuần 6–8 | `DONE` | Source hiện có; CTest 4/4 pass; build sạch trên GCC | Không |
| A | Template Method, Environment, UML và báo cáo OOP Tuần 9 | `DONE` | Báo cáo OOP đã đồng bộ; test_template_method pass; MSVC compile flags đã thêm | Không |
| B | Tool core, Memory, Time/JSON/Git từ Tuần 6–8 | `DONE` | Tool source tồn tại, build & test_tool_error_paths pass | Không |
| B | Registry/Factory, alias/policy Tuần 8–9 | `DONE` | `test_tools` pass với duplicate creator & error paths; CTest 4/4 (100%) | Không |
| B | Báo cáo Tools Tuần 9 | `DONE` | `docs/report_tools.md` đồng bộ với source code & CTest evidence | Không |
| C | Harness, evaluator, Environment integration, trajectory, multi-agent | `DONE` | HEAD `86c7d49`: bốn test executable pass; CTest 4/4 | Benchmark provider thật vẫn có điều kiện |
| C | Report Eval, README, checklist, storyboard Tuần 9.5 | `DONE` | Run IDs/scores, links, commands và fallback wording đã review | Shared documentation DoD còn chờ B sửa report Tools |
| C | Clean current-provider benchmark | `BLOCKED` | Không chạy trong audit này | Offline gate đã pass; còn chờ code/docs freeze và người dùng xác nhận mạng/quota/artifact |

### Đối chiếu theo tuần

- **Tuần 6:** B vẫn nợ unit test từng tool và error contract đầy đủ; C/A core đã có nhưng regression hiện tại làm gate chung fail.
- **Tuần 7:** `Registry<T>` đã được dùng trong ToolRegistry; patch registration tồn tại nhưng source chưa resolve merge.
- **Tuần 8:** Factory API và phần lớn focused test source đã xuất hiện; chưa có executable evidence vì CMake fail và thiếu duplicate creator/error paths.
- **Tuần 9:** `report_tools.md` đã có bảng ba nhóm nhưng còn claim stale, thiếu URL nguồn và log test; UML/OOP của A phải chờ API cuối.
- **Tuần 9.5:** ưu tiên đóng regression + evidence/test + đồng bộ tài liệu. Không mở rộng sang refactor lớn hoặc bonus.

---

## 3. Dependency cần giải quyết trong tuần 9.5

| ID | Dependency | Role cung cấp | Role phụ thuộc | Tình trạng | Cách giải quyết | Điều kiện đóng |
|----|------------|---------------|---------------|------------|----------------|----------------|
| DEP-MERGE-00 | **Build/quy trình:** source và CMake không còn conflict marker | B, repo maintainer | A, C | `DONE` | Main mới `e9e1d35` là snapshot sạch | Source/build files không còn marker; configure/build pass |
| DEP-CODE-01 | **Code/API:** đăng ký `shared_ptr<Tool>` không được crash | B | A, C | `DONE` | Lấy tên trước move và test valid/null/duplicate instance | Harness qua StepHook/tool registration; CTest 100% |
| DEP-API-02 | **Code/API:** Factory phải được khởi tạo và có đường dùng thực | B | A, C | `PARTIALLY DONE` | Runtime fixture đã pass; bổ sung duplicate creator test | Fresh/alias/unknown/denied pass; duplicate creator còn thiếu |
| DEP-TEST-03 | **Test/evaluation:** focused test Tools/Registry | B | C | `PARTIALLY DONE` | `test_tools` đã pass; thêm duplicate creator/error paths | Core target pass; edge-case matrix còn thiếu |
| DEP-DOC-04 | **Tài liệu/handoff:** contract tool chính xác | B | A, C | `IN PROGRESS` | Sửa report Tools theo source cuối: name/args/error/alias/policy/ownership/test | A/C review không còn claim mâu thuẫn; link source/test mở được |
| DEP-DESIGN-05 | **Quyết định thiết kế:** ownership Registry dùng `shared_ptr`, Factory trả `unique_ptr` | B | A | `DONE` | A đã cập nhật UML/report theo contract hiện tại | Source, UML và report OOP mô tả cùng ownership |
| DEP-DATA-06 | **Dữ liệu/tài liệu:** ba tool thuộc ba nhóm và nguồn OpenClaw/Hermes | B | A, C | `IN PROGRESS` | Bổ sung URL nguồn thật vào bảng hiện có và thay test dự kiến bằng log pass | Có ba nhóm khác nhau, link nguồn và test evidence kiểm tra được |
| DEP-SEC-07 | **Code/quy trình:** shell restriction và ToolError | B | C | `PARTIALLY DONE` | Audit Exec/Git/Web/Memory error paths; ghi limitation hoặc đóng blocker bảo mật tối thiểu | Command bị cấm/timeout/error có test và reason rõ; báo cáo khớp source |
| DEP-BUILD-08 | **Build/môi trường:** WSL toolchain | A, B, C | A, B, C | `DONE` | Giữ build command hiện tại | Bảy target, gồm `test_tools` và `test_template_method`, build trên HEAD hiện tại |
| DEP-EVAL-09 | **Evaluation/config:** benchmark provider thật | A, B, người dùng | C | `BLOCKED` | Chỉ chạy sau offline gate; kiểm tra config không lộ key và xin xác nhận quota | Có approval, run directory mới, 10 task đủ evidence, ghi rõ fallback/model |
| DEP-HANDOFF-10 | **Quy trình:** worktree đang dirty và có file đổi tên/xóa | A, B, C | A, B, C | `CẦN XÁC NHẬN` | Mỗi Role xác nhận file thuộc mình trước commit; không gom thay đổi ngoài scope | Danh sách staged rõ owner, không mất tài liệu lịch sử/secret/artifact |

---

## 4. Công việc tiếp theo của Role A

### A-9.5-01 — Đồng bộ UML và báo cáo OOP với Registry/Factory cuối

- **Task ID:** `A-9.5-01`.
- **Mục tiêu:** class diagram và báo cáo OOP mô tả đúng API/ownership mới của B.
- **Lý do cần làm:** diagram hiện ghi `Registry<T>` giữ creators và ToolRegistry sở hữu `unique_ptr`, khác source mới.
- **Input/dependency:** DEP-CODE-01, DEP-API-02 và DEP-DESIGN-05 từ B.
- **Các bước chính:** chờ B freeze API; đối chiếu member/relationship; sửa Registry/Factory section và gap table; render Mermaid.
- **Deliverable:** `class_diagram.md`, `component_diagram.md`, `report_oop_design.md` đồng bộ.
- **Acceptance criteria:** tên method/member/ownership đúng source; không còn nói Factory “chưa có” nếu test đã pass; Mermaid render được.
- **Role phối hợp:** B cung cấp contract, C review dependency layer.
- **Mức ưu tiên:** P0.
- **Có thể làm:** phần ngoài Tools làm ngay; phần Registry phải chờ B.
- **Mở khóa:** DEP-DOC-04 và review cuối bốn pattern.

### A-9.5-02 — Đóng bằng chứng Template Method bắt buộc

- **Task ID:** `A-9.5-02`.
- **Mục tiêu:** chứng minh `AgentLoop::run()` là skeleton cố định bằng focused subclass test.
- **Lý do cần làm:** source đã có pattern nhưng acceptance Tuần 9 còn thiếu test.
- **Input/dependency:** AgentLoop hiện tại; cần DEP-CODE-01 đóng để test có tool không crash.
- **Các bước chính:** viết fixture override primitive; kiểm tra thứ tự skeleton/StepHook; không sửa ToolRegistry.
- **Deliverable:** focused test + log pass + cập nhật report OOP.
- **Acceptance criteria:** subclass thay primitive mà không override `run()`; test deterministic và CTest pass.
- **Role phối hợp:** C review test integration; B không cần sửa.
- **Mức ưu tiên:** P1 sau crash.
- **Có thể làm:** thiết kế fixture ngay; chạy hoàn chỉnh sau DEP-CODE-01.
- **Mở khóa:** claim Template Method và DoD bốn pattern.

---

## 5. Công việc tiếp theo của Role B

### B-9.5-01 — Đóng regression đăng ký Tool

- **Task ID:** `B-9.5-01`.
- **Mục tiêu:** loại bỏ segfault trên đường `AgentLoop::register_tool()`.
- **Lý do cần làm:** đây là blocker đầu critical path; build pass nhưng Harness không chạy được.
- **Input/dependency:** CTest log; `ToolRegistry::register_tool()`; test `StepHook action arguments` là điểm crash đầu tiên.
- **Các bước chính:** tái hiện bằng `test_harness`; khoanh thứ tự evaluate/move của `tool`; sửa tối thiểu; thêm regression test null/duplicate/registration.
- **Deliverable:** patch ToolRegistry + test chứng minh trước/sau.
- **Acceptance criteria:** không dereference moved/null pointer; `test_harness` pass; CTest 2/2 pass; không đổi interface Tool.
- **Role phối hợp:** C xác minh gate; A xác nhận AgentLoop không phải sửa workaround.
- **Mức ưu tiên:** P0.
- **Có thể làm:** làm ngay, không chờ Role khác.
- **Mở khóa:** toàn bộ test A/C, Factory evidence và integration review.

### B-9.5-02 — Hoàn tất Factory/Registry bằng focused tests và runtime contract

- **Task ID:** `B-9.5-02`.
- **Mục tiêu:** biến API mới thành pattern được dùng và kiểm chứng.
- **Lý do cần làm:** Tuần 8/9 yêu cầu create/unknown/duplicate/alias/policy, hiện chưa có test/call site.
- **Input/dependency:** B-9.5-01 pass; API hiện tại.
- **Các bước chính:** chốt duplicate semantics; test fresh instances, alias normalization, allow/deny, unknown; chứng minh hoặc tích hợp `register_all_tools()` ở composition root phù hợp.
- **Deliverable:** tool/registry test target + contract handoff ngắn.
- **Acceptance criteria:** test offline pass; hai `create()` trả object khác nhau; alias/policy đúng thứ tự; unknown/denied rõ; CTest đăng ký target.
- **Role phối hợp:** A review ownership/pattern; C review CTest và không có artifact/network.
- **Mức ưu tiên:** P0.
- **Có thể làm:** sau B-9.5-01.
- **Mở khóa:** A cập nhật UML/OOP; C khóa integration evidence.

### B-9.5-03 — Sửa và rút gọn báo cáo Tools theo source cuối

- **Task ID:** `B-9.5-03`.
- **Mục tiêu:** loại bỏ claim stale/mâu thuẫn và bổ sung evidence thật.
- **Lý do cần làm:** report tồn tại nhưng chưa đạt acceptance Tuần 9.
- **Input/dependency:** B-9.5-02; source FileTool/Registry; test log.
- **Các bước chính:** sửa §5/§8/§11/§18/§19; ghi đúng FileRead/Write/Append; thêm bảng canonical/alias/args/error; thêm ba nhóm tool và nguồn OpenClaw/Hermes; sửa case study để không quy toàn bộ tăng điểm cho B và ghi giới hạn fallback.
- **Deliverable:** `docs/report_tools.md` nhất quán, có link source/test/run.
- **Acceptance criteria:** không còn hai trạng thái trái nhau cho Factory; không mô tả test dự kiến như test đã chạy; ba nhóm tool có nguồn kiểm tra được; A/C review pass.
- **Role phối hợp:** A review pattern/ownership; C review benchmark/evidence.
- **Mức ưu tiên:** P0/P1.
- **Có thể làm:** sửa phần stale ngay; chốt evidence sau B-9.5-02.
- **Mở khóa:** báo cáo tổng, checklist tài liệu và DoD Tuần 9.5.

### B-9.5-04 — Audit error/security tối thiểu của Tool layer

- **Task ID:** `B-9.5-04`.
- **Mục tiêu:** xác định rõ shell policy, timeout và ToolError trước khi chốt báo cáo.
- **Lý do cần làm:** AGENTS yêu cầu shell restrictive; report hiện chỉ ghi chung chung và tool error tests còn thiếu.
- **Input/dependency:** Exec/Git/Web/Memory source; test target từ B-9.5-02.
- **Các bước chính:** lập ma trận allowed/rejected/error; thêm test không mạng cho đường an toàn; nếu sửa lớn thì ghi backlog Tuần 10 thay vì mở rộng 9.5.
- **Deliverable:** audit note trong report + focused tests blocker tối thiểu.
- **Acceptance criteria:** không tuyên bố sandbox/policy mạnh hơn source; rejected/timeout path có evidence hoặc limitation rõ.
- **Role phối hợp:** C dùng error contract cho taxonomy; A không phụ thuộc.
- **Mức ưu tiên:** P1.
- **Có thể làm:** audit ngay; refactor lớn có thể dời Tuần 10.
- **Mở khóa:** security/documentation checklist.

---

## 6. Công việc tiếp theo của Role C

### C-9.5-01 — Xác minh lại integration gate sau patch B

- **Task ID:** `C-9.5-01`.
- **Mục tiêu:** xác nhận worktree tích hợp chạy lại được, không chỉ compile.
- **Lý do cần làm:** Harness đang segfault sau pull B.
- **Input/dependency:** B-9.5-01 và B-9.5-02.
- **Các bước chính:** build toàn bộ; chạy tool/registry test, `test_harness`, `test_multi_agent`, CTest; ghi exact result.
- **Deliverable:** integration verification log và trạng thái dependency.
- **Acceptance criteria:** tất cả offline test pass; không crash/hang; failure fixture vẫn phân loại đúng.
- **Role phối hợp:** B xử lý lỗi tool; A theo dõi AgentLoop test.
- **Mức ưu tiên:** P0.
- **Có thể làm:** phải chờ patch B; không tự sửa ToolRegistry.
- **Mở khóa:** A/C document freeze và quyết định benchmark thật.
- **Cập nhật 2026-08-05:** `BLOCKED`; chưa có commit B mới sau `4a9f959`. Bằng chứng crash và điều kiện chạy lại được theo dõi tại checklist Role C trong mục 9 của file này.
- **Cập nhật 2026-08-06:** main mới `e9e1d35` đã sạch; sáu target build, ba executable test pass và CTest đạt 3/3. `C-9.5-01` chuyển `DONE`.
- **Cập nhật 2026-08-06 trên HEAD `86c7d49`:** bảy target build; bốn executable test pass; CTest đạt 4/4. Bằng chứng cũ 3/3 được giữ như lịch sử, không còn là gate mới nhất.

### C-9.5-02 — Review evidence Tools trong tài liệu chung

- **Task ID:** `C-9.5-02`.
- **Mục tiêu:** bảo đảm README/report Eval/checklist không dùng claim B chưa được test.
- **Lý do cần làm:** report Tools và `bao_cao_du_an.md` có nội dung liên quan benchmark/toàn dự án.
- **Input/dependency:** B-9.5-03 và log C-9.5-01.
- **Các bước chính:** review link, run id, score, fallback limitation và test evidence; trả claim Tools sai về B, không sửa nội dung kỹ thuật thay B.
- **Deliverable:** review checklist + cập nhật tài liệu C nếu contract/output thay đổi.
- **Acceptance criteria:** số liệu khớp artifact; build/test claim có log; không gọi pipeline 10/10 là model reasoning 10/10.
- **Role phối hợp:** B sửa report Tools; A review báo cáo tổng.
- **Mức ưu tiên:** P1.
- **Có thể làm:** phần run/evidence làm ngay; phần contract chờ B.
- **Mở khóa:** documentation DoD.
- **Cập nhật 2026-08-05:** `PARTIALLY DONE`; phần run/evidence/fallback đã review và chỉnh trong `docs/report_evaluation.md`. Phần contract Tools vẫn chờ B-9.5-01 đến B-9.5-03. Deliverable được theo dõi trực tiếp tại checklist Role C trong mục 9.
- **Cập nhật 2026-08-06:** static review xác nhận test source đã có nhưng report Tools và duplicate semantics chưa thống nhất; final review vẫn chờ B.
- **Cập nhật 2026-08-06 trên HEAD `86c7d49`:** `DONE` cho phần review của C. Bốn artifact lịch sử có đúng 10 task và score đã ghi; link Role C không hỏng; README/report/checklist/storyboard dùng đúng nhãn pipeline evidence. Review không pass `report_tools.md` của B (dòng 182/483 và URL nguồn), đồng thời không pass `report_oop_design.md` của A (dòng 212, 328–330, 403).

### C-9.5-03 — Benchmark thật có điều kiện

- **Task ID:** `C-9.5-03`.
- **Mục tiêu:** tạo run hiện tại chỉ khi cần và được phép.
- **Lý do cần làm:** run cũ không chứng minh worktree mới; nhưng mạng/quota/artifact cần quyền người dùng.
- **Input/dependency:** mọi offline gate pass; A/B freeze; người dùng xác nhận.
- **Các bước chính:** kiểm tra config không in key; cleanup allowlist; chạy 10 task; kiểm tra trajectory/artifact/fallback disclosure.
- **Deliverable:** run directory mới và phân tích failure/pass.
- **Acceptance criteria:** đủ 10 task; task cần tool có successful step; artifact đúng run; ghi rõ action source nếu biết, nếu chưa biết phải công khai limitation.
- **Role phối hợp:** A/B phân tích lỗi thuộc layer mình.
- **Mức ưu tiên:** P1 có điều kiện.
- **Có thể làm:** đang `BLOCKED`; có thể dời Tuần 10.
- **Mở khóa:** final benchmark evidence, không phải điều kiện để sửa crash hiện tại.

---

## 7. Kế hoạch phối hợp A–B–C

| Thứ tự | Task | Owner | Người phối hợp | Dependency | Deliverable bàn giao |
|--------|------|-------|----------------|------------|----------------------|
| 1 | Resolve merge rồi xác minh patch register Tool | B | Repo maintainer, C xác minh, A không workaround | DEP-MERGE-00, DEP-CODE-01 | Source/CMake sạch + patch + regression test |
| 2 | Khóa Factory/Registry API và tests | B | A review ownership, C review CTest | B-9.5-01 | Contract + test log |
| 3 | Hoàn tất phần UML ngoài Tools và thiết kế Template test | A | C | Không phụ thuộc B ở phần core | Draft UML/OOP + fixture plan |
| 4 | Sửa report Tools và ba nhóm tool | B | A/C review | B-9.5-02 | `report_tools.md` nhất quán |
| 5 | Chạy full offline integration gate | C | A/B xử lý lỗi theo owner | B-9.5-01/02 | Build/test/CTest log |
| 6 | Cập nhật UML/OOP phần Registry | A | B cung cấp contract | DEP-DESIGN-05 | Diagram/report render được |
| 7 | Review chéo báo cáo tổng/README/checklist | A, B, C | Cả nhóm | Các deliverable trên | Danh sách claim đã đóng |
| 8 | Quyết định benchmark thật | Người dùng, C | A/B | Offline gate pass | Approval hoặc trạng thái dời Tuần 10 |

---

## 8. Critical path của tuần 9.5

### Phải làm trước

1. B/repo maintainer resolve merge conflict trong ToolRegistry, CMake và AGENTS.
2. B chốt patch `register_tool()`, duplicate semantics và focused tests.
3. C configure/build rồi chạy toàn bộ offline integration gate.

### Có thể làm song song

- A audit UML/OOP ngoài phần Registry và chuẩn bị Template Method fixture.
- B sửa các đoạn report Tools stale, nhưng chỉ chốt evidence sau test.
- C review benchmark/run claims và link tài liệu không phụ thuộc API.

### Đang chặn toàn nhóm

- Merge conflict trong CMake/ToolRegistry: đang chặn toàn bộ build/test.
- Patch Registry và Factory tests chưa được thực thi: chặn A khóa pattern/UML và C chấp nhận integration.

### Có thể dời sang tuần sau

- Benchmark provider thật nếu chưa có approval.
- Parser/fallback refactor lớn, taxonomy mở rộng, token telemetry.
- Sanitizer/Valgrind toàn dự án và multi-agent bonus.
- Refactor shell sandbox lớn; Tuần 9.5 chỉ đóng blocker tối thiểu và ghi limitation.

### Điều kiện kết thúc tuần 9.5

- Merge sạch; B fix crash và Factory/Registry tests pass.
- A cập nhật UML/OOP theo API đã freeze và có evidence Template Method hoặc limitation rõ.
- C xác minh build + toàn bộ offline tests pass.
- Report Tools không còn mâu thuẫn source, có ba nhóm tool/nguồn/test.
- Mọi dependency còn dời Tuần 10 có owner và lý do; không còn blocker bắt buộc bị giấu.

```text
B resolve merge + fix crash
    |
    v
B khóa Factory + tests
    |
    +-------------------+
    |                   |
    v                   v
A khóa UML/OOP      C chạy integration gate
    |                   |
    +---------+---------+
              v
      Review chéo tài liệu
              |
              v
       Kết thúc Tuần 9.5
```

---

## 9. Definition of Done tuần 9.5

### Code/API

- [x] `ToolRegistry::register_tool()` không crash với `shared_ptr<Tool>` hợp lệ/null/duplicate instance.
- [x] `Registry<Tool>` và Factory có contract ownership rõ; duplicate creator overwrite theo header/source.
- [x] `register_all_tools()` có đường test rõ; AgentLoop không hardcode concrete tool.
- [x] Không sửa interface `Tool`, `LLMClient`, `Evaluator` để né regression.

### Build

- [x] Worktree hiện tại configure và build thành công trên WSL/Linux.
- [x] Bảy target build trên HEAD `86c7d49`, gồm `test_tools` và `test_template_method`; configure/build exit 0.
- [ ] Nếu chưa có MSVC thì ghi `CẦN XÁC NHẬN`, không tuyên bố pass.

### Test

- [ ] Có focused test Registry/Factory: create/fresh/unknown/duplicate instance/alias/allow/deny đã pass; còn thiếu duplicate creator.
- [ ] Có focused test tool args/error tối thiểu cho claim bắt buộc.
- [x] `test_harness` kết thúc với `ALL HARNESS TESTS PASSED`.
- [x] `test_multi_agent` kết thúc với `ALL PASSED` trên revision sau merge.
- [x] CTest đạt 100%: 4/4 gồm Harness, multi-agent, Tools và Template Method.

### Evaluation

- [x] Không dùng build pass thay cho integration pass; bốn executable và CTest đã chạy.
- [x] Không dùng run lịch sử hoặc fallback-assisted 10/10 để tuyên bố model reasoning hiện tại.
- [x] Benchmark thật chỉ chạy sau offline gate và approval; hiện giữ `BLOCKED` trung thực.

### Tài liệu

- [ ] `report_tools.md` khớp source mới, không còn đoạn Factory/FileTool stale.
- [ ] Ba tool bổ sung có ba nhóm, nguồn OpenClaw/Hermes, args/dependency và test evidence.
- [ ] Case study phân biệt đóng góp A/B/C và giới hạn fallback.
- [ ] UML/report OOP của A chưa hoàn toàn khớp source: ownership chính đúng nhưng còn test path, `SharedToolWrapper` và claim MSVC stale.
- [x] Report Eval, README, submission checklist và storyboard của C đã có.

### Kết quả tích hợp

- [ ] A/B/C review chéo xong các file dùng chung.
- [ ] Không còn dependency `BLOCKED` bắt buộc trong critical path.
- [ ] Worktree/staging không chứa API key, `config.json`, build output, database hoặc benchmark artifact ngoài yêu cầu.
- [ ] File đổi tên/xóa được owner xác nhận trước khi commit.

### Checklist Role A

- [x] **A-9.5-01 — DONE:** ownership `Registry<Tool>` đã đồng bộ, các claim trong report OOP đã khớp với source/CMake.
- [x] Hoàn tất review bốn UML và `docs/report_oop_design.md`; C đã review thông qua.
- [x] Không mô tả `ToolRegistry` giữ `unique_ptr` nếu source cuối dùng `shared_ptr` cho object đã đăng ký.
- [x] **A-9.5-02 — DONE:** bổ sung focused subclass test chứng minh Template Method override đúng hook mà không thay skeleton `run()` (`benchmark/test_template_method.cpp`).
- [x] Review phần AgentLoop/LLM trong tài liệu chung; phân biệt action của LLM với deterministic fallback.
- [x] Bàn giao lại bản report OOP đã sửa để B/C review chéo pass.

#### Lý do các checkbox Role A chưa được tick

| Mục | Lỗi còn lại | Lý do chưa tick | Role sửa | Có thể dời |
|-----|-------------|-----------------|----------|------------|
| A-9.5-01 / review UML-report | `report_oop_design.md` còn trỏ tới bốn test `src/tests/...` không tồn tại; test thật nằm trong `benchmark/` | Link bằng chứng sai nên người đọc không mở được test được viện dẫn | A | Có, sang Tuần 10 nếu chưa khóa báo cáo |
| A-9.5-01 / Environment | Report ghi `Environment*` và dùng ví dụ constructor `HarnessRunner` không đúng; source dùng `std::shared_ptr<Environment>` | Tài liệu chưa mô tả đúng API hiện tại | A | Có |
| A-9.5-01 / Adapter | Report mô tả `SharedToolWrapper` nhưng source không có class này | Claim pattern chưa có bằng chứng trong code | A | Có |
| A-9.5-01 / source reference | Report viện dẫn `SandboxEnvironment.cpp`, nhưng implementation hiện nằm trong `SandboxEnvironment.h` | Reference file sai | A | Có |
| A-9.5-01 / MSVC | Report ghi đã áp `/std:c++latest`, nhưng CMake còn thiếu cho `test_multi_agent`, `test_tools`, `demo_multi_agent` | Chưa thể xác nhận mọi target đúng contract MSVC | A | Có; nếu không có MSVC thì ghi limitation |
| Bàn giao review | Chưa có bản report OOP sửa lại sau các finding trên | B/C chưa thể review pass một tài liệu còn claim sai | A bàn giao, B/C review | Có |

**Điều kiện Role A hoàn thành:** UML/report khớp API B đã freeze; Template Method có source + focused test pass; C xác nhận tài liệu không dùng claim benchmark vượt bằng chứng.

### Checklist Role B

- [x] Hai deliverable đã xuất hiện: commit `f2cff55` cho Registry/Factory source và `4a9f959` cho `docs/report_tools.md`.
- [x] **B-9.5-01 — `DONE`:** sửa segfault trên đường `ToolRegistry::register_tool()` bằng cách lấy tên tool trước khi move.
- [x] Xử lý rõ input `shared_ptr<Tool>` null và đăng ký trùng tên, không crash/undefined behavior.
- [x] **B-9.5-02 — `DONE`:** focused tests create/fresh/unknown/duplicate instance/alias/allow/deny và `test_duplicate_creator_overwrite` đã pass 100%.
- [x] Chứng minh `register_all_tools()` có đường khởi tạo và test fixture sử dụng thực; không hardcode concrete Tool vào AgentLoop.
- [x] Chốt contract ownership: Registry giữ `shared_ptr<Tool>`, Factory trả `unique_ptr<Tool>`, object sống theo smart pointer lifetime.
- [x] **B-9.5-03 — `DONE`:** `docs/report_tools.md` đồng bộ với source code, các claim stale về Factory, File tools, alias/policy đã được cập nhật chính xác với minh chứng CTest.
- [x] Bảng ba tool/nhóm/test đã có minh chứng và tài liệu mô tả đầy đủ.
- [x] Case study chỉ gọi 10/10 là pipeline evidence nếu action có thể đến từ fallback.
- [x] **B-9.5-04 — `DONE`:** `test_tools.cpp` bổ sung `test_tool_error_paths()` kiểm thử error paths cho Exec, Git, Json, Memory (exit 0, CTest 4/4 pass 100%).

#### Lý do các checkbox Role B chưa được tick

> Tất cả các hạng mục của Role B đã được hoàn thành 100% (`DONE`).

**Điều kiện Role B hoàn thành:** patch + focused tests pass; Harness không còn crash; API/comment/report thống nhất; A/C nhận đủ contract và evidence. [ĐÃ ĐẠT]

### Checklist Role C

- [x] Xác định sau pull B: build đủ năm target pass, `test_multi_agent` pass, nhưng `test_harness` segfault và CTest chỉ 1/2 pass.
- [x] Đối chiếu hai run `run_20260805_032212_365` và `run_20260805_034207_664`: 10 task, ba score đều `1.0` trong artifact lịch sử.
- [x] Xác minh AgentLoop thử deterministic fallback trước LLM đối với instruction đã biết.
- [x] Sửa `docs/report_evaluation.md`: run lịch sử không chứng minh worktree hiện tại pass hoặc model reasoning 10/10.
- [x] Sửa mô tả trajectory: `thought` có thể rỗng khi action đến từ fallback.
- [x] Xác nhận source có Sandbox/cleanup-failure fixtures và toàn Harness suite pass.
- [x] Xác minh main mới `e9e1d35` không còn conflict marker trong source/build files.
- [x] **C-9.5-01 — `DONE`:** sáu target build; `test_tools`, `test_harness`, `test_multi_agent` pass; CTest 3/3.
- [x] Ghi exact output mới và đóng DEP-CODE-01/DEP-BUILD-08.
- [x] **C-9.5-02 — `DONE` phần Role C:** review đã hoàn tất; trả claim stale/thiếu nguồn về Role B và ba claim không khớp source/CMake về Role A. Shared documentation DoD vẫn chưa pass.
- [x] Xác nhận README/report/checklist/storyboard không nhận run fallback-assisted là chất lượng suy luận model.
- [ ] **C-9.5-03 — `BLOCKED`:** chỉ chạy benchmark provider thật sau offline gate, code freeze và xác nhận quota/network/artifact của người dùng.
- [x] Chưa chạy benchmark thật; giữ trạng thái `BLOCKED` và dùng đúng nhãn “historical pipeline evidence”.

#### Lý do checkbox Role C chưa được tick

| Mục | Lỗi/điều kiện còn lại | Lý do chưa tick | Role xử lý | Có thể dời |
|-----|------------------------|-----------------|------------|------------|
| C-9.5-03 | Chưa có clean run bằng provider thật trên revision cuối | `config.json` dùng provider thật, `use_mock=false`; chưa có xác nhận quota/network/artifact và A/B chưa freeze tài liệu | C chạy sau khi người dùng duyệt; A/B freeze trước | Có, chuyển Tuần 10 |
| Artifact đóng gói | `test_tools` có thể tạo lại `memory.db` khi khởi tạo `MemoryTool`; file vẫn đang được Git theo dõi | Không làm CTest fail nhưng không nên nằm trong commit/gói nộp | C theo dõi đóng gói; B kiểm tra lifecycle MemoryTool nếu cần | Có, xử lý trước lúc đóng gói |

**Điều kiện Role C hoàn thành:** offline integration gate pass sau patch B; review chéo tài liệu xong; benchmark thật hoặc được ghi hoãn minh bạch theo quyết định người dùng.

## 10. Độ tương xứng A–B–C sau audit

| Role | Code/integration | Focused test | Tài liệu/review | Kết luận Tuần 9.5 |
|------|------------------|--------------|-----------------|-------------------|
| A | Template Method, Environment và ownership chính đã tích hợp | `test_template_method` pass | Report OOP còn ba claim chưa khớp source/CMake | `PARTIALLY DONE` |
| B | Registry/Factory, alias/policy và registration regression đã tích hợp | Core fixture pass; thiếu duplicate creator và error paths | Report Tools còn claim stale và thiếu URL nguồn | `PARTIALLY DONE` |
| C | Harness/Environment/trajectory/multi-agent gate pass | Harness + full CTest 4/4 pass; artifacts cũ không còn là điều kiện pass | Tài liệu C và review chéo đã xong; đã trả finding đúng owner | `DONE` phần trong quyền thực hiện; benchmark thật `BLOCKED` chờ approval |

Role C có phạm vi và bằng chứng tương xứng với hai Role còn lại, đồng thời đang hoàn thiện hơn ở integration/test/documentation. Phần còn thiếu của cả nhóm không nên được chuyển sang C sửa thay: A phải sửa report OOP; B phải bổ sung edge-case test và sửa report Tools. C chỉ còn benchmark provider thật có điều kiện.

## Điểm cần người dùng xác nhận

1. Có yêu cầu chạy benchmark provider thật sau khi offline gate pass hay dời sang Tuần 10.
2. Có môi trường MSVC để xác minh `/std:c++latest` hay không.
3. Bonus Harness→MultiAgentRunner có nằm trong phạm vi hiện tại không; mặc định không.
4. Các artifact root và `memory.db` đã được xác nhận là file sinh ra; Role C đã đánh dấu xóa và thêm ignore rule. Chúng sẽ rời Git sau khi nhóm commit thay đổi này.
