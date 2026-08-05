# KH_TUAN9.5 — AI-AGENT Project

Ngày audit sau khi pull Role B: 2026-08-05

> Phạm vi của file này là kiểm tra trạng thái tích hợp, đóng blocker bắt buộc và hoàn thành tài liệu Tuần 9.5. Đây không phải đợt refactor toàn diện của Tuần 10.
>
> Chỉ dùng trạng thái: `DONE`, `PARTIALLY DONE`, `IN PROGRESS`, `BLOCKED`, `NOT STARTED`, `CẦN XÁC NHẬN`.
>
> Đây là nguồn checklist duy nhất của Tuần 9.5. Mỗi Role cập nhật checkbox của mình tại mục 9; không tạo checklist Role riêng nếu không có yêu cầu mới.

## Nguồn đã dùng

- Git HEAD và hai commit mới có phạm vi Role B: `f2cff55`, `4a9f959`.
- Source hiện tại: `src/tools/`, integration point trong `src/agent/agent_loop.h`, Harness và test hiện tại.
- `docs/report_tools.md`, `docs/bao_cao_du_an.md` và tài liệu A/C hiện có.
- `KH_Tuan6_ChiTiet (1).md`, `KH_Tuan6_Updated.md`, `KH_Tuan7_8_ChiTiet.md`, `KH_Tuan9_ChiTiet.md`, `OOP_PHANVIEC.md` và bản KH Tuần 9.5 trước audit.
- Build/test trực tiếp trên worktree tích hợp hiện tại.

### Giới hạn kết luận

- Worktree đang có thay đổi chưa commit của A/C và một số file được đổi tên/xóa. Kết quả test dưới đây phản ánh **worktree tích hợp hiện tại**, không chỉ riêng hai commit B.
- Chỉ các file Tools và `report_tools.md` được tính là deliverable rõ của B. `docs/bao_cao_du_an.md` bao phủ toàn dự án nên cần A/C review; không tự động quy mọi nội dung trong đó là trách nhiệm hoặc thành quả đã xác minh của B.
- Không chạy `run_eval` vì gate offline đang fail và benchmark có thể dùng mạng/quota thật.

---

## 1. Trạng thái sau khi tích hợp phần việc Role B

| Hạng mục | Kết quả | Bằng chứng | Trạng thái | Vấn đề còn lại |
|----------|---------|------------|------------|----------------|
| Commit mới của B | Khoanh được hai commit có phạm vi B rõ | `f2cff55` sửa `Registry.h`, `ToolRegistry.*`; `4a9f959` tạo `docs/report_tools.md` | `DONE` | `docs/bao_cao_du_an.md` trong commit đầu là tài liệu toàn dự án, cần review chéo |
| Generic `Registry<T>` | `ToolRegistry` đã dùng `Registry<Tool>` thay cho map instance riêng | `ToolRegistry.h`: member `Registry<Tool> registry_`; `register_tool()`/`lookup()` gọi Registry | `PARTIALLY DONE` | Đường đăng ký tool làm `test_harness` segfault; chưa có focused test cho Registry |
| Factory creator theo tên | Có `ToolCreator`, `register_creator()`, `create()` và creator cho built-in tools | `ToolRegistry.h/.cpp` | `PARTIALLY DONE` | Không tìm thấy call site của `register_all_tools()` ngoài định nghĩa; Factory chưa được chứng minh trong runtime/test |
| Alias và allow/deny policy | Normalize chạy trước policy trong `create()` và `lookup()` | `ToolRegistry.cpp` | `PARTIALLY DONE` | Không có test alias/create/allow/deny/unknown/duplicate; comment nói duplicate creator bị overwrite nhưng code trả `false` |
| Tích hợp AgentLoop→Registry | AgentLoop vẫn chỉ gọi abstraction `ToolRegistry`, không hardcode concrete tool | `agent_loop.h::register_tool()` | `BLOCKED` | `test_harness` segfault khi bước test đầu tiên đăng ký tool; nghi ngờ truy cập `tool->get_name()` và move `tool` trong cùng lời gọi ở `ToolRegistry::register_tool()` |
| Build toàn bộ | Năm target build thành công sau lần build tiếp nối | `cmake --build build -j2`: `OopAgent`, `run_eval`, `test_multi_agent`, `test_harness`, `demo_multi_agent` đều built | `DONE` | Lần đầu hết timeout 120 giây; lần tiếp theo hoàn tất trong khoảng 23 giây |
| Offline integration tests | Multi-agent pass nhưng Harness crash | `test_multi_agent`: `ALL PASSED`; CTest: 1/2 pass, `harness` segfault | `BLOCKED` | Không thể dùng build pass để kết luận tích hợp đúng |
| `docs/report_tools.md` | Deliverable đã xuất hiện, có inventory/case study/pattern/SOLID | Commit `4a9f959`, file 978 dòng | `PARTIALLY DONE` | Nhiều đoạn mâu thuẫn source mới; phần Testing là danh sách cần test, không phải log test pass |
| Tính đúng của báo cáo Tools | Một số phần cuối mô tả Factory/alias hiện có | `report_tools.md` §21–§24 | `PARTIALLY DONE` | §5/§11/§19 vẫn nói Factory chưa có; §8/§11 nói không có FileRead/Write/Append dù source có các class này |
| Ba tool bổ sung | Time/JSON/Git có source và được đăng ký | `TimeTool`, `JsonTool`, `GitTool`; `register_all_tools()` | `PARTIALLY DONE` | Report chưa có phân loại ba nhóm rõ và không có nguồn OpenClaw/Hermes/link tham chiếu |
| Benchmark case study | Có bảng run 2/10→10/10 và mô tả cải thiện | `report_tools.md` §17 | `PARTIALLY DONE` | Chưa tách đóng góp B khỏi A/C và chưa ghi giới hạn fallback; không thay thế clean run hiện tại |
| Unit/focused tests của B | Không tìm thấy executable/test case gọi API Factory mới | Search trong `benchmark/`, `tests/`, CMake | `NOT STARTED` | Đây là acceptance criteria bắt buộc từ Tuần 8/9 và là nguyên nhân regression không bị chặn trước push |

### Kết luận Role B

Role B đã tạo được **khung Registry/Factory và báo cáo Tools**, nhưng chưa thể xem là hoàn thành tích hợp. Build pass, song gate quan trọng `test_harness` đang segfault; Factory chưa có call site/test; báo cáo Tools còn stale và thiếu nguồn ba nhóm tool.

---

## 2. Trạng thái hiện tại của các Role

| Role | Task trước đó | Trạng thái | Bằng chứng | Dependency còn lại |
|------|---------------|------------|-----------|---------------------|
| A | LLM clients, AgentLoop/StepHook, parser, loop detection từ Tuần 6–8 | `PARTIALLY DONE` | Source hiện có; guarded fallback/C++26 đã build | Harness regression từ B đang chặn test AgentLoop có tool; parser/fallback test rộng để Tuần 10 nếu không là blocker |
| A | Template Method, Environment, UML và báo cáo OOP Tuần 9 | `IN PROGRESS` | Template skeleton và Environment hierarchy có source; bốn tài liệu UML/report tồn tại | Thiếu focused subclass test; UML/report vẫn mô tả Registry/Factory/ownership theo trạng thái cũ |
| B | Tool core, Memory, Time/JSON/Git từ Tuần 6–8 | `PARTIALLY DONE` | Tool source tồn tại và build | Thiếu unit/error/security test; ba nhóm tool chưa có nguồn chứng minh |
| B | Registry/Factory, alias/policy Tuần 8–9 | `BLOCKED` | API mới có trong `f2cff55` | Segfault integration, không có focused test, `register_all_tools()` chưa có call site chứng minh |
| B | Báo cáo Tools Tuần 9 | `PARTIALLY DONE` | `docs/report_tools.md` đã xuất hiện trong `4a9f959` | Nội dung tự mâu thuẫn, stale File/Factory status, thiếu OpenClaw/Hermes và test evidence |
| C | Harness, evaluator, Environment integration, trajectory, multi-agent | `PARTIALLY DONE` | Source/docs C có; `test_multi_agent` pass | `test_harness` bị regression từ B nên trạng thái tích hợp sau pull chưa đạt |
| C | Report Eval, README, checklist, storyboard Tuần 9.5 | `DONE` | Tài liệu đã audit và trước pull các gate offline pass | Phải cập nhật bằng chứng tích hợp sau khi B đóng regression; benchmark thật vẫn chờ xác nhận |
| C | Clean current-provider benchmark | `BLOCKED` | Không chạy trong audit này | Chờ B fix + A/B/C offline gate pass + người dùng xác nhận mạng/quota/artifact |

### Đối chiếu theo tuần

- **Tuần 6:** B vẫn nợ unit test từng tool và error contract đầy đủ; C/A core đã có nhưng regression hiện tại làm gate chung fail.
- **Tuần 7:** `Registry<T>` đã được dùng thật trong ToolRegistry, nhưng smart-pointer/registration path đang có dấu hiệu lỗi runtime.
- **Tuần 8:** Factory API đã xuất hiện nhưng acceptance `alias/policy/create/unknown/duplicate` chưa có test; integration chưa đạt.
- **Tuần 9:** `report_tools.md` đã có nhưng chưa đúng source và thiếu nguồn ba nhóm tool; UML/OOP của A cần cập nhật theo API mới.
- **Tuần 9.5:** ưu tiên đóng regression + evidence/test + đồng bộ tài liệu. Không mở rộng sang refactor lớn hoặc bonus.

---

## 3. Dependency cần giải quyết trong tuần 9.5

| ID | Dependency | Role cung cấp | Role phụ thuộc | Tình trạng | Cách giải quyết | Điều kiện đóng |
|----|------------|---------------|---------------|------------|----------------|----------------|
| DEP-CODE-01 | **Code/API:** đăng ký `shared_ptr<Tool>` không được crash | B | A, C | `BLOCKED` | B khoanh và sửa `ToolRegistry::register_tool()`; giữ ownership rõ | `test_harness` qua StepHook/tool registration và CTest 2/2 pass |
| DEP-API-02 | **Code/API:** Factory phải được khởi tạo và có đường dùng thực | B | A, C | `IN PROGRESS` | Chốt nơi gọi `register_all_tools()` hoặc test fixture khởi tạo rõ; không hardcode vào AgentLoop | Test tạo hai instance mới, alias create, unknown/denied/duplicate pass |
| DEP-TEST-03 | **Test/evaluation:** focused test Tools/Registry | B | C | `NOT STARTED` | B tạo test target thuộc tool layer; C review CTest registration và isolation | Test target build/pass, không gọi mạng, artifact nằm trong temp |
| DEP-DOC-04 | **Tài liệu/handoff:** contract tool chính xác | B | A, C | `IN PROGRESS` | Sửa report Tools theo source cuối: name/args/error/alias/policy/ownership/test | A/C review không còn claim mâu thuẫn; link source/test mở được |
| DEP-DESIGN-05 | **Quyết định thiết kế:** ownership Registry dùng `shared_ptr`, Factory trả `unique_ptr` | B | A | `CẦN XÁC NHẬN` | B giải thích lifetime và duplicate semantics; A cập nhật UML/report | Source, comment, UML và report mô tả cùng một contract |
| DEP-DATA-06 | **Dữ liệu/tài liệu:** ba tool thuộc ba nhóm và nguồn OpenClaw/Hermes | B | A, C | `NOT STARTED` | B lập bảng tool→nhóm→nguồn→args/dependency→test; không tự nhận đạt nếu thiếu | Có ba nhóm khác nhau, link nguồn và test evidence kiểm tra được |
| DEP-SEC-07 | **Code/quy trình:** shell restriction và ToolError | B | C | `PARTIALLY DONE` | Audit Exec/Git/Web/Memory error paths; ghi limitation hoặc đóng blocker bảo mật tối thiểu | Command bị cấm/timeout/error có test và reason rõ; báo cáo khớp source |
| DEP-BUILD-08 | **Build/môi trường:** WSL toolchain | A, B, C | A, B, C | `DONE` | Giữ lệnh build hiện tại | Năm target build pass trên worktree hiện tại |
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
| 1 | Tái hiện và sửa segfault register Tool | B | C xác minh, A không workaround | DEP-CODE-01 | Patch + regression test |
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

1. B đóng segfault `register_tool()`.
2. B thêm focused test Registry/Factory và chốt runtime contract.
3. C chạy lại toàn bộ offline integration gate.

### Có thể làm song song

- A audit UML/OOP ngoài phần Registry và chuẩn bị Template Method fixture.
- B sửa các đoạn report Tools stale, nhưng chỉ chốt evidence sau test.
- C review benchmark/run claims và link tài liệu không phụ thuộc API.

### Đang chặn toàn nhóm

- `test_harness` segfault: chặn chứng minh AgentLoop→Tool→Harness và làm mất trạng thái pass trước pull.
- Thiếu test Factory: chặn A khóa pattern/UML và C chấp nhận integration.

### Có thể dời sang tuần sau

- Benchmark provider thật nếu chưa có approval.
- Parser/fallback refactor lớn, taxonomy mở rộng, token telemetry.
- Sanitizer/Valgrind toàn dự án và multi-agent bonus.
- Refactor shell sandbox lớn; Tuần 9.5 chỉ đóng blocker tối thiểu và ghi limitation.

### Điều kiện kết thúc tuần 9.5

- B fix crash và Factory/Registry tests pass.
- A cập nhật UML/OOP theo API đã freeze và có evidence Template Method hoặc limitation rõ.
- C xác minh build + toàn bộ offline tests pass.
- Report Tools không còn mâu thuẫn source, có ba nhóm tool/nguồn/test.
- Mọi dependency còn dời Tuần 10 có owner và lý do; không còn blocker bắt buộc bị giấu.

```text
B fix crash
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

- [ ] `ToolRegistry::register_tool()` không crash với `shared_ptr<Tool>` hợp lệ/null/duplicate.
- [ ] `Registry<Tool>` và Factory có contract ownership/duplicate rõ.
- [ ] `register_all_tools()` có đường dùng/test rõ; AgentLoop không hardcode concrete tool.
- [ ] Không sửa interface `Tool`, `LLMClient`, `Evaluator` để né regression.

### Build

- [x] Năm target build thành công trên WSL/Linux.
- [ ] Build sạch sau patch cuối của B, không chỉ incremental link.
- [ ] Nếu chưa có MSVC thì ghi `CẦN XÁC NHẬN`, không tuyên bố pass.

### Test

- [ ] Có focused test Registry/Factory: create/fresh/unknown/duplicate/alias/allow/deny.
- [ ] Có focused test tool args/error tối thiểu cho claim bắt buộc.
- [ ] `test_harness` kết thúc với `ALL HARNESS TESTS PASSED`.
- [x] `test_multi_agent` kết thúc với `ALL PASSED`.
- [ ] CTest đạt 100%; kết quả audit hiện tại mới 50% vì harness segfault.

### Evaluation

- [ ] Không dùng build pass thay cho integration pass.
- [ ] Không dùng run lịch sử hoặc fallback-assisted 10/10 để tuyên bố model reasoning hiện tại.
- [ ] Benchmark thật chỉ chạy sau offline gate và approval; nếu chưa chạy thì ghi `BLOCKED` trung thực.

### Tài liệu

- [ ] `report_tools.md` khớp source mới, không còn đoạn Factory/FileTool stale.
- [ ] Ba tool bổ sung có ba nhóm, nguồn OpenClaw/Hermes, args/dependency và test evidence.
- [ ] Case study phân biệt đóng góp A/B/C và giới hạn fallback.
- [ ] UML/report OOP của A khớp `Registry<Tool>`, creator map và ownership cuối.
- [x] Report Eval, README, submission checklist và storyboard của C đã có.

### Kết quả tích hợp

- [ ] A/B/C review chéo xong các file dùng chung.
- [ ] Không còn dependency `BLOCKED` bắt buộc trong critical path.
- [ ] Worktree/staging không chứa API key, `config.json`, build output, database hoặc benchmark artifact ngoài yêu cầu.
- [ ] File đổi tên/xóa được owner xác nhận trước khi commit.

### Checklist Role A

- [ ] **A-9.5-01 — `BLOCKED`:** chờ B freeze Registry/Factory API và giải thích ownership/duplicate semantics.
- [ ] Cập nhật bốn UML và `docs/report_oop_design.md` theo source cuối của `Registry<Tool>` và Factory.
- [ ] Không mô tả `ToolRegistry` giữ `unique_ptr` nếu source cuối dùng `shared_ptr` cho object đã đăng ký.
- [ ] **A-9.5-02 — `IN PROGRESS`:** bổ sung focused subclass test chứng minh Template Method override đúng hook mà không thay skeleton `run()`.
- [ ] Review phần AgentLoop/LLM trong tài liệu chung; phân biệt action của LLM với deterministic fallback.
- [ ] Bàn giao link source, diagram và test cho B/C review chéo.

**Điều kiện Role A hoàn thành:** UML/report khớp API B đã freeze; Template Method có source + focused test pass; C xác nhận tài liệu không dùng claim benchmark vượt bằng chứng.

### Checklist Role B

- [x] Hai deliverable đã xuất hiện: commit `f2cff55` cho Registry/Factory source và `4a9f959` cho `docs/report_tools.md`.
- [ ] **B-9.5-01 — `BLOCKED`:** sửa segfault trên đường `ToolRegistry::register_tool()` và thêm regression test.
- [ ] Xử lý rõ input `shared_ptr<Tool>` null và đăng ký trùng tên, không crash/undefined behavior.
- [ ] **B-9.5-02 — `NOT STARTED`:** thêm focused tests cho create/fresh instance/unknown/duplicate/alias/allow/deny.
- [ ] Chứng minh `register_all_tools()` có đường khởi tạo hoặc fixture sử dụng thực; không hardcode concrete Tool vào AgentLoop.
- [ ] Chốt contract ownership: Registry giữ gì, Factory trả gì, object sống bao lâu.
- [ ] **B-9.5-03 — `IN PROGRESS`:** sửa các đoạn tự mâu thuẫn trong `docs/report_tools.md` về Factory và FileRead/Write/Append.
- [ ] Bổ sung bảng ba tool thuộc ba nhóm, nguồn OpenClaw/Hermes, args/dependency và test evidence.
- [ ] Case study chỉ gọi 10/10 là pipeline evidence nếu action có thể đến từ fallback.
- [ ] **B-9.5-04 — `IN PROGRESS`:** audit error/security tối thiểu cho Exec/Git/Web/Memory và ghi limitation chưa xử lý.

**Điều kiện Role B hoàn thành:** patch + focused tests pass; Harness không còn crash; API/comment/report thống nhất; A/C nhận đủ contract và evidence.

### Checklist Role C

- [x] Xác định sau pull B: build đủ năm target pass, `test_multi_agent` pass, nhưng `test_harness` segfault và CTest chỉ 1/2 pass.
- [x] Đối chiếu hai run `run_20260805_032212_365` và `run_20260805_034207_664`: 10 task, ba score đều `1.0` trong artifact lịch sử.
- [x] Xác minh AgentLoop thử deterministic fallback trước LLM đối với instruction đã biết.
- [x] Sửa `docs/report_evaluation.md`: run lịch sử không chứng minh worktree hiện tại pass hoặc model reasoning 10/10.
- [x] Sửa mô tả trajectory: `thought` có thể rỗng khi action đến từ fallback.
- [x] Xác nhận source có Sandbox/cleanup-failure fixtures; chưa gọi toàn suite xanh vì executable crash trước các fixture sau.
- [ ] **C-9.5-01 — `BLOCKED`:** nhận commit hash patch B, build lại và chạy focused Tool test, `test_harness`, `test_multi_agent`, CTest.
- [ ] Ghi exact output mới; chỉ đóng DEP-CODE-01 khi không crash/hang và CTest đạt 100%.
- [ ] **C-9.5-02 — `PARTIALLY DONE`:** review Registry/Factory contract trong tài liệu chung sau khi B-9.5-03 hoàn thành.
- [ ] Xác nhận README/report/checklist/storyboard không nhận run fallback-assisted là chất lượng suy luận model.
- [ ] **C-9.5-03 — `BLOCKED`:** chỉ chạy benchmark provider thật sau offline gate, code freeze và xác nhận quota/network/artifact của người dùng.
- [ ] Nếu chưa chạy benchmark thật, giữ trạng thái `BLOCKED` và dùng đúng nhãn “historical pipeline evidence”.

**Điều kiện Role C hoàn thành:** offline integration gate pass sau patch B; review chéo tài liệu xong; benchmark thật hoặc được ghi hoãn minh bạch theo quyết định người dùng.

## Điểm cần người dùng xác nhận

1. Có yêu cầu chạy benchmark provider thật sau khi offline gate pass hay dời sang Tuần 10.
2. Có môi trường MSVC để xác minh `/std:c++latest` hay không.
3. Bonus Harness→MultiAgentRunner có nằm trong phạm vi hiện tại không; mặc định không.
4. Các file lịch sử đang hiện trạng thái deleted/renamed trong worktree có phải chủ ý hay không; không tự phục hồi trong audit này.
