# Kế hoạch hoàn tất và Code Freeze — Tuần 10 — AI-AGENT OOP 2026

> **Cập nhật:** 2026-08-09
> **Mục tiêu cuối tuần:** `Implementation Complete → Test Evidence Locked → Bonus Checked → Full Regression Passed → Code Freeze`. Docs được merge/format/chốt phát hành vào Tuần 11 dựa trên evidence đã khóa.
> **Quy tắc:** chỉ tick `[x]` khi có source + test/log + tài liệu tương ứng. Benchmark có deterministic fallback chỉ là **pipeline evidence**, không phải bằng chứng model reasoning. Ba bonus đã được nhóm chọn cho Tuần 10: Vector Search, Multi-agent và VLM/GUI Agent.
> **Nguồn ưu tiên khi có mâu thuẫn:** đề chính thức `planning/reference/OOP Project 2026 AI Agent.docx (1).md` → source/test/CMake → artifact run mới → tài liệu báo cáo → kế hoạch lịch sử.

> **Bản điều phối một trang:** [`KH_Tuan10_TongQuan_1Trang.md`](KH_Tuan10_TongQuan_1Trang.md). File này giữ task, dependency và DoD chi tiết.

## 1. Project Completion Goal

Tuần 10 là tuần hoàn tất implementation cuối cùng. Cả ba role phải đóng toàn bộ requirement bắt buộc, bug và evidence test, sau đó hoàn thành ba bonus đã chọn: Vector Search, Multi-agent Coordination và VLM/GUI Agent. Các claim/source/test link cho docs phải được khóa để Tuần 11 merge an toàn; việc tổng hợp/format/final review docs diễn ra Tuần 11. Mỗi bonus chỉ được merge khi mandatory gate PASS và không được làm chậm freeze.

Tuần 11 là presentation/documentation phase: merge và format docs theo evidence đã khóa, làm slide, chốt demo flow, quay/chỉnh video Unlisted và chuẩn bị oral. Không thêm feature mới sau freeze; critical fix phải đi theo luồng `fix → targeted test → full regression → re-freeze`.

## 2. Requirement Audit

| ID | Requirement / nguồn | Phân loại | Implementation status | Test status | Documentation status | Owner | Có trong plan cũ? | Kết luận / việc phải đóng |
|---|---|---|---|---|---|---|---|---|
| R01 | LLM interface, Ollama-compatible HTTP, config (`base URL`, model, temperature, max tokens), text + image cùng interface; lỗi timeout/connect/JSON — đề §3.1 | Mandatory | Có client/interface; multimodal và error paths chưa có evidence cuối | Partial | Partial | A | Partial | A-10-01 |
| R02 | Runtime ToolRegistry; name/description/execute; allow/deny — đề §3.2 | Mandatory | Có Registry/Factory, alias/policy | Focused tests đã có | Có, cần rà revision cuối | B | Yes | B-10-01 review/freeze |
| R03 | 5 tool: exec, read/write, web, SQLite memory, calculator — đề §3.2 | Mandatory | Có | Exec/Git/Json/Memory negative path có; Web/timeout chưa đủ | Có, cần đồng bộ | B | Partial | B-10-02, B-10-03 |
| R04 | Ít nhất 3 tool bổ sung thuộc 3 loại, có nguồn tham khảo — đề §3.2 | Mandatory | Time/JSON/Git đã có | Registration evidence có | Có URL, phải rà link/claim | B | Yes | B-10-01 |
| R05 | ≥3 skill Markdown; keyword selection; inject trước **mỗi** run — đề §3.3 | Mandatory | Loader/skill files có | Injection chưa có focused evidence | Partial | A | Partial | A-10-03 |
| R06 | ReAct, parse tool call, history, `max_steps`, graceful stop — đề §3.4 | Mandatory | Có AgentLoop/fallback | Parser variants/malformed intent chưa đóng | Partial | A | Yes | A-10-02 |
| R07 | Generic-repeat + ping-pong, threshold, warning/stop — đề §3.5 | Mandatory | Có | Regression phải chạy lại | Có | A | Partial | A-10-04 |
| R08 | Harness setup→run→evaluate→record; 2 evaluator; trajectory; batch; JSON export — đề §3.6 | Mandatory | Có | Harness test/clean run có evidence lịch sử; final run còn phải kiểm tra | Có, cần cập nhật final evidence | C | Yes | C-10-01, C-10-02 |
| R09 | 4 pattern: Strategy, Template Method, Registry/Factory, Observer/Hook — đề §4.2 | Mandatory | Có source paths | Template/Registry focused test có; final regression cần chạy | Partial | A/B/C | Yes | A-10-06, B-10-01, C-10-02 |
| R10 | 4 Mermaid UML: class, agent sequence, harness sequence, component — đề §4.3 | Mandatory | Có `.md` | Chưa có full render evidence | Partial | A | Partial | A-10-07 |
| R11 | Layer separation: AgentLoop không biết Harness; tools/evaluators không phụ thuộc sai layer — đề §4.4 | Mandatory | Có thiết kế | Cần static audit + test gate | Partial | A/C | Partial | A-10-06 |
| R12 | ≥4 C++17, ≥2 C++20, ≥2 C++23, ≥1 C++26; smart pointer/no leak/error handling — đề §V và nhóm reference checklist | Mandatory | C++26 guarded `inplace_vector` có source fallback | Sanitizer và feature matrix chưa đóng | Partial | A/B | Partial | A-10-05, B-10-04 |
| R13 | 10 benchmark task: 4 simple / 4 medium / 2 hard; JSON, success-rate analysis — đề §7 | Mandatory | Có tasks và run sạch 10/10 hiện lưu | Final artifact/action-level/post-condition chưa chốt trên revision freeze | Partial | C | Yes | C-10-01, C-10-02 |
| R14 | README build/run/config; báo cáo hoàn chỉnh; 4 diagram; source/package — đề §IX, docs guide, submission checklist | Mandatory | Tài liệu đã có khung/nội dung | Render/link/claim/package verification chưa đủ | Partial | A/B/C | Partial | A-10-07, B-10-05, C-10-03, C-10-04 |
| R15 | Không lộ API key, config thật, DB, build/artifact; clean clone/build/run — checklist nộp | Mandatory | `.gitignore`/checklist có | Chưa có dry-run package cuối | Partial | C/B | Partial | B-10-04, C-10-05 |
| R16 | Persistent memory vector search: embeddings + cosine similarity — đề §10.2 | Bonus +4 (committed) | **Chưa có implementation**; `std::inplace_vector` không phải vector-search bonus | Chưa có | Chưa có | B (A review) | Now assigned | BNS-V-01, sau mandatory PASS |
| R17 | Multi-agent: Harness spawn sub-agent, queue+mutex, demo task phức tạp chia 2 agent song song — đề §10.3 | Bonus +3 (committed) | Queue/threads/demo riêng có; chưa chứng minh Harness integration + 2-agent parallel decomposition | Basic queue test có | Có mô tả limitation | C (A review) | Now assigned | BNS-M-01, sau mandatory PASS |
| R18 | GUI agent: screenshot, VLM, action executor, browser-search-copy demo — đề §10.1 | Bonus +8 (committed) | Chưa có GUI workflow; `VLMEvaluator` skeleton không phải GUI agent | Chưa có | Chưa có | A/B/C | Now assigned | BNS-G-01, sau mandatory PASS |

### Inconsistency resolved

- Kế hoạch cũ gọi Tuần 10 là “video/slide”; yêu cầu mới của nhóm chuyển video/slide sang tuần sau. Kế hoạch này **thay thế thứ tự cũ**: freeze code trước, presentation sau.
- `std::inplace_vector` là requirement C++26 (R12), **không** đáp ứng bonus vector search (R16).
- Multi-agent hiện có message queue/demo riêng, nhưng chưa đủ để claim bonus R17 vì đề yêu cầu Harness spawn và task phức tạp chia hai agent song song.
- `PROJECT_STATUS.md` cũ còn ghi benchmark thật blocked và report Tools thiếu URL; artifact/kế hoạch Tuần 9.5 và Tuần 10 đã ghi run sạch `run_20260807_085427_143` cùng URL bổ sung. Status sẽ được cập nhật thành “có evidence lịch sử, phải re-verify ở freeze”.

## 3. Missing Items Found

Các mục mandatory bị thiếu hoặc chưa được phân rã đủ trong kế hoạch cũ:

- Focused verification cho multimodal, timeout/connection-refused/malformed JSON của LLM client.
- Focused evidence rằng SkillLoader chọn và inject skill trước mỗi agent run.
- Render evidence cho đủ 4 Mermaid diagram.
- Audit/test matrix chứng minh toàn bộ quota C++17/20/23/26, portability fallback và sanitizer.
- Clean-clone/package verification từ README, kèm secret/artifact scan.
- Requirement bonus multi-agent đầy đủ (Harness integration + hai worker song song), không chỉ queue ping/pong.
- Requirement vector-search bonus được tách riêng khỏi C++26 `inplace_vector`.
- VLM/GUI Agent bonus được tách riêng khỏi `VLMEvaluator` skeleton; bonus yêu cầu screenshot → VLM → action executor → demo thật.

Không có mandatory requirement nào được phép chuyển sang tuần presentation. Những mục chưa thể chạy do môi trường/quota phải ghi limitation có owner; riêng lỗi làm feature mandatory sai thì là **BLOCKER** và phải sửa trong tuần này.

## 4. Remaining Work

### Core implementation và bug fixing

- A: đóng parser formats/malformed tool-intent, skill injection, client error contract; sửa lỗi mandatory tìm được.
- B: hoàn tất error contract/args cho 5 tool, đặc biệt Web/timeout; kiểm tra database lifecycle và policy.
- C: kiểm tra clean state, failure reason/action-level/post-condition; sửa Harness/evaluator nếu final run phát hiện false pass.

### Required testing

- Focused tests cho A/B/C task dưới đây; build + CTest + 4 executable test bắt buộc.
- Benchmark clean-state với provider đã được nhóm phê duyệt quota, sau khi source freeze-candidate pass offline gates.
- ASan trên WSL nếu toolchain hỗ trợ; MSVC build nếu có môi trường. Không có môi trường phải ghi limitation, không tuyên bố pass.

### Documentation evidence this week; merge next week

- Mỗi claim requirement phải có mapping `requirement → source → test/log → doc` trước freeze.
- Tuần 10 khóa nội dung/evidence/source paths và render 4 diagram; Tuần 11 A/B/C merge, format, review cuối các report/README/checklist, không thêm claim hoặc feature mới.

### Integration/finalization

- Clean worktree review, owner confirmation file đổi/xóa, dry-run package, clean extraction/build/test, freeze note/commit/tag theo quy ước nhóm.

## 5. Role A Plan — Systems/Core

- [ ] **A-10-01 — LLM client requirement audit và negative fixtures**
  **P0 · Files:** `src/client/*`, `benchmark/test_harness.cpp` (hoặc test mới), `docs/report_oop_design.md`, README.
  **Dependency:** mock/fake transport không cần mạng. **Output:** test text-only, image payload đi cùng interface; timeout, connection-refused và malformed JSON trả lỗi phân loại. **DoD:** source pass build, fixture deterministic pass, doc chỉ claim được phần test. **Verify:** targeted executable + CTest.

- [ ] **A-10-02 — Đóng parser protocol và graceful tool-intent handling**
  **P0 · Files:** `src/agent/agent_loop.*`, client normalizer, focused test.
  **Input:** raw JSON, fenced JSON, `ACTION:`, provider-call/function-call, malformed intent. **Output:** valid call thực thi; malformed/cần-tool không bị coi final answer; parser failure có signal rõ. **DoD:** mỗi protocol có assert/result; không thêm Harness dependency vào AgentLoop. **Verify:** focused test + harness regression.

- [ ] **A-10-03 — Skill selection/injection evidence**
  **P0 · Files:** `src/agent/SkillLoader.*`, `src/agent/agent_loop.*`, `skills/`, focused test, report OOP.
  **Output:** test chọn skill theo keyword và xác nhận nội dung skill đi vào system prompt **trước mỗi run**. **DoD:** có ≥3 skill thực, unmatched case rõ, limitation ghi nếu selection không deterministic. **Verify:** focused fake client captures prompt.

- [ ] **A-10-04 — Loop detection and core failure taxonomy handoff**
  **P1 · Files:** `LoopDetector.*`, AgentLoop, harness schema/docs.
  **Dependency:** A-10-02. **Output:** generic repeat, ping-pong, max-step và parser-fail có reason/stop behavior đúng. **DoD:** reason không trống và C có thể consume; test không phụ thuộc provider. **Verify:** targeted + `test_harness`.

- [ ] **A-10-05 — C++ feature matrix, C++26 fallback và sanitizer**
  **P1 · Files:** `agent_loop.cpp`, CMake, `docs/report_oop_design.md`.
  **Output:** file→feature→purpose→test table chứng minh ≥4 C++17, ≥2 C++20, ≥2 C++23, ≥1 C++26; guarded `inplace_vector` fallback build được. **DoD:** ASan log không có lỗi mới, hoặc limitation có compiler/command/owner. **Verify:** WSL ASan build + 4 test executables; C review log.

- [ ] **A-10-06 — OOP/layer/pattern final audit**
  **P1 · Files:** `docs/report_oop_design.md`, `docs/bao_cao_du_an.md`, source interfaces.
  **Dependency:** A-10-01..05, B-10-01, C-10-02. **Output:** source/doc mapping cho 4 patterns, ownership và layer separation. **DoD:** không claim VLM complete, MSVC pass hay model reasoning nếu thiếu evidence. **Verify:** static review signed by B/C.

- [ ] **A-10-07 — Render và review 4 UML**
  **P0 · Files:** `class_diagram.md`, `sequence_agent_run.md`, `sequence_harness.md`, `component_diagram.md`.
  **Output:** 4 diagram render không lỗi, khớp source freeze candidate. **DoD:** render evidence/link stored and A/B/C review. **Verify:** Mermaid render command/service và visual check.

## 6. Role B Plan — Tools/Data

- [ ] **B-10-01 — Tool contract/document synchronization**
  **P0 · Files:** `src/tools/*`, `docs/report_tools.md`, README.
  **Output:** canonical name, aliases, description, args, allow/deny, ownership and three additional-tool sources match exact source. **DoD:** no stale HEAD reference/dead source URL; AgentLoop has no hardcoded concrete tool. **Verify:** `test_tools` + A/C doc review.

- [ ] **B-10-02 — Tool args and negative-path matrix**
  **P0 · Files:** `benchmark/test_tools.cpp`, `src/tools/*`.
  **Input:** read/write filename/path/content variants; calculator trim/invalid; Exec policy/exit code; memory save/search/invalid; JSON/Git errors. **Output:** valid args create expected artifact; invalid input returns `ToolError`/`std::unexpected`, no throw/crash. **DoD:** one deterministic assertion per listed path and report table update. **Verify:** targeted `test_tools` + CTest.

- [ ] **B-10-03 — WebSearch and Exec timeout offline fixture**
  **P0 · Files:** `WebSearchTool.*`, `ExecTool.*`, `benchmark/test_tools.cpp` or injected adapter.
  **Output:** offline tests for network failure/HTTP error/timeout and shell timeout/denied command. **DoD:** no live-network dependency, stable expected error classification. **Verify:** repeat `test_tools` twice offline. **BLOCKER:** if injection seam cannot be added safely, record exact limitation/owner and obtain team decision before freeze.

- [ ] **B-10-04 — Memory database lifecycle and repository hygiene**
  **P1 · Files:** `MemoryTool.*`, `.gitignore`, `docs/report_tools.md`, checklist.
  **Output:** proof `memory.db` and generated artifacts are ignored/untracked and tests do not package user data. **DoD:** do not delete user data without owner confirmation; `git ls-files`/dry-run evidence clear. **Verify:** C package scan.

- [ ] **B-10-05 — Tool layer final review**
  **P1 · Files:** report Tools, README, shared docs.
  **Dependency:** B-10-01..04. **Output:** review sign-off for tool source/test/docs mapping. **DoD:** every R02–R04 claim answers where implemented, tested and documented. **Verify:** A/C review checklist.

## 7. Role C Plan — Eval/Infra/Freeze

- [ ] **C-10-01 — Clean-state and failure/action audit**
  **P0 · Files:** `HarnessRunner.*`, `Task.*`, evaluators, `report_evaluation.md`.
  **Output:** final run records `required_tools`, category, tool step count, failure reason, evaluator/action-level score and post-condition. **DoD:** required-tool task with no actual step is `NO_TOOL_EXECUTION`; old artifact cannot false-pass. **Verify:** focused harness tests and inspect trajectories.

- [ ] **C-10-02 — Full offline integration gate**
  **P0 · Files:** CMake/test logs/README.
  **Dependency:** A-10-01..05 and B-10-01..03 merge to freeze candidate. **Output:** clean configure/build, all four tests and CTest 4/4 pass. **DoD:** no ignored failure; defects return owner and regression run repeats after fix. **Verify:** Gate 1–6 matrix below.

- [ ] **C-10-03 — Approved clean benchmark and evidence lock**
  **P0 · Files:** `benchmark/tasks.json`, fresh `benchmark/results/...`, README/eval report.
  **Dependency:** C-10-02 + explicit provider/quota approval. **Output:** one approved clean-state 10-task run, 4/4/2 distribution, JSON/trajectory/summary inspection. **DoD:** provider/model/run ID exact, no API key, fallback source and token `0 = not measured` explicit. **Verify:** inspect all 10 trajectory files and summary. **BLOCKER:** no quota/network means retain existing run as historical pipeline evidence and record limitation; it does not silently become a fresh final run.

- [ ] **C-10-04 — Documentation evidence lock and cross-review**
  **P0 · Files:** `README.md`, `docs/report_evaluation.md`, `docs/bao_cao_du_an.md`, `docs/submission_checklist.md`.
  **Dependency:** A-10-06/07, B-10-05, C-10-01/03. **Output:** requirement→implementation→test→doc table, final source/test citations and reviewer log for Tuần 11 merge. **DoD:** all links work, diagram/render references correct, limitations consistent; no unverified claim remains. **Verify:** A/B/C sign-off.

- [ ] **C-10-05 — Dry-run package, clean extraction and freeze declaration**
  **P0 · Files:** submission checklist, freeze note, Git history.
  **Dependency:** all mandatory tasks/gates PASS. **Output:** approved ZIP manifest; extraction into clean directory follows README, build and core tests pass; final commit/tag per team convention; freeze note. **DoD:** ZIP excludes `config.json`, API keys, `.git`, build/cache, DB and generated artifacts; every changed/deleted file has owner. **Verify:** clean extraction build + `test_harness`, `test_multi_agent`, CTest.

## 8. Dependency Map

```text
A-10-01/02/03/04/05 ─┐
B-10-01/02/03/04    ├→ C-10-01 → C-10-02 → C-10-03 (approved quota)
A-10-06/07 + B-10-05┘                         ↓
                                      C-10-04 docs review
                                             ↓
                                [all mandatory PASS]
                                             ↓
                   BNS-V-01 → BNS-M-01 → BNS-G-01 (committed; isolated)
                                             ↓
                                  full regression again
                                             ↓
                               C-10-05 package → Code Freeze
```

Parallel work: A-10-01..05, B-10-01..04, and C-10-01 can start independently. Shared docs only merge through their named owner after reviewer sign-off, reducing conflicts.

## 9. Documentation Plan

| Deliverable | Owner → Reviewer → merge target | Completion rule |
|---|---|---|
| Architecture/OOP, C++ matrix, 4 UML | A → B/C → `docs/report_oop_design.md`, diagram files | source/test paths correct; 4 render checks pass |
| Tools inventory/error matrix/source URLs | B → A/C → `docs/report_tools.md` | canonical names/args/policy/test evidence match code |
| Harness/benchmark/failure taxonomy | C → A/B → `docs/report_evaluation.md` | run ID, scores, limitation and trajectory claims match artifact |
| Combined report | A drafts sections; B/C review domains in T10 → merge/format in T11: `docs/bao_cao_du_an.md` | no stale skeleton/overclaim; links to detailed reports |
| Build/run/security/submission | C locks commands/evidence in T10; A/B review → merge in T11: `README.md`, `docs/submission_checklist.md` | clean extraction follows README; no secret/artifact claim |
| Freeze record | C → A/B → planning status + freeze note | gates and final revision recorded |

## 10. Testing Matrix

| Gate / test | Requirement covered | Owner | Expected result / PASS criteria |
|---|---|---|---|
| Gate 1: clean configure/build | All | C | `cmake -S . -B build` and build all 7 targets exit 0; no correctness warning ignored |
| A focused client/parser/skill/loop tests | R01, R05–R07 | A | All protocol and negative fixtures pass without provider network |
| `test_tools` repeated offline | R02–R04 | B | Canonical/alias/policy/args/error/timeout fixture pass twice |
| `test_template_method` | R09, R11 | A | Skeleton/hook behavior passes |
| `test_harness` | R08, R13 | C | Cleanup, trajectory, evaluator, failure/post-condition checks pass |
| `test_multi_agent` | baseline R17 support | C | Queue/thread lifecycle passes; not yet bonus claim |
| `demo_multi_agent` | R17 committed bonus | C | Parallel task evidence after BNS-M-01 implementation |
| GUI Agent focused + controlled end-to-end demo | R18 committed bonus | A/B/C | Mocked invalid/timeout actions pass; controlled screenshot→VLM→action demo passes |
| Gate 3: negative/error review | R01, R03, R06–R08 | A/B/C | timeout, invalid args, malformed protocol, loops and no-tool cases classified |
| Gate 4: integration | R01–R13 | C | Agent → registry/tool → harness/evaluator works without stale artifact pass |
| Gate 5/6: CTest + full regression | All mandatory | C | CTest 4/4 and all focused executables pass on final revision; repeat after every merged bonus |
| ASan / MSVC | R12 | A | ASan clean; MSVC build pass or limitation clearly recorded |
| Approved clean benchmark | R13 | C | 10 tasks 4/4/2, valid JSON, paths/arguments/post-conditions checked |
| Diagram/link/package verification | R10, R14–R15 | A/C | renders, links, clean extraction and secret scan pass |

## 11. Bonus Decision

### BNS-V-01 — Persistent Memory with Vector Search (+4)

**Status:** Committed; currently NOT STARTED. `std::inplace_vector` does not count.
**Owner:** B; A reviews client/embedding interface, C verifies regression.
**Dependency:** every mandatory gate PASS and a stable embedding provider/model (for example `nomic-embed-text`) approved.
**Work/merge criteria:** persist embedding with each memory entry; cosine similarity search in C++; deterministic focused tests using fixed vectors; integration test proving ranking; README/report source/limitation update.
**Risk:** network/model availability, schema migration, provider quota. **Decision:** begin only after mandatory gate PASS; merge only with vector focused tests and full regression.

### BNS-M-01 — Multi-agent Coordination (+3)

**Status:** Committed; partial baseline exists only.
**Owner:** C; A reviews Agent API/layer boundary, B supplies safe subtask/tool contract.
**Dependency:** every mandatory gate PASS.
**Work/merge criteria:** HarnessRunner uses public API to spawn two sub-agents on separate threads; queue+mutex lifecycle is safe; a complex task is partitioned and two agents execute concurrently; join/error/timeout handled; focused concurrency test and demo log show parallel work; docs updated.
**Risk:** race/deadlock and scope creep. **Decision:** begin only after mandatory gate PASS; existing ping/pong test may be demonstrated only as baseline, not claimed as bonus completion.

### BNS-G-01 — VLM/GUI Agent (+8)

**Status:** Committed; NOT STARTED. This is the GUI Agent requirement in đề §10.1, not the existing `VLMEvaluator` skeleton.
**Owners:** A owns VLM/image request through the shared LLM interface; B owns `capture_screenshot` and action-tool contracts; C owns action executor, end-to-end demo, test/log and regression.
**Dependency:** all mandatory gates PASS; approved Linux/desktop environment and VLM model; no secret-bearing browser profile.
**Work/merge criteria:** capture a screenshot; send base64 image through the same client interface; validate/execute only bounded `click`, `type_text`, `key_press` actions; demonstrate browser search and copy result; record action/timeout/error evidence; update docs.
**Safety/quality gates:** explicit allow-list and coordinate/input validation; test mocked VLM response and invalid action paths; use a disposable demo environment; no uncontrolled shell/browser side effects. **Merge criteria:** focused test + controlled end-to-end demo + full regression PASS.

## 12. Integration Checklist

- [ ] Interfaces/contracts from A and B merged with no conflict marker.
- [ ] C verifies no stale artifacts can satisfy a functional evaluator.
- [ ] Canonical tool names/descriptions/aliases agree across source, prompt and report.
- [ ] Every mandatory requirement has an implementation path, test/log and documentation path.
- [ ] Four patterns and layer boundaries match final source.
- [ ] Generated data, config and API keys are absent from staged/package files.
- [ ] A/B/C review shared docs and record owner for all changed/deleted files.

## 13. Full Regression Checklist

- [ ] Clean configure and build all 7 targets.
- [ ] Run `test_tools`, `test_harness`, `test_multi_agent`, `test_template_method`.
- [ ] Run `ctest --test-dir build --output-on-failure` and obtain 4/4 pass.
- [ ] Run any new focused A/B tests and inspect logs.
- [ ] Run ASan suite if toolchain supports it; otherwise record limitation.
- [ ] Run/inspect approved clean benchmark if provider/quota is approved.
- [ ] After each merged bonus: repeat build, 4 executables, CTest and impacted integration tests.
- [ ] Perform clean extraction/build/test using README.

## 14. Code Freeze Checklist

- [ ] Mandatory implementation complete; no mandatory blocker is marked future work.
- [ ] Required bugs fixed or a blocking decision is recorded.
- [ ] Source/test evidence and four rendered UML are locked; docs merge/format is scheduled for Tuần 11.
- [ ] Gates 1–6 all PASS on one final revision.
- [ ] Vector Search, Multi-agent and VLM/GUI Agent each meet their merge criteria; focused tests and full regression pass.
- [ ] Benchmark evidence/limitations and token/fallback wording are accurate.
- [ ] Package dry-run and clean extraction pass; no secret/artifact leak.
- [ ] Final branch merged, commit/tag and project status recorded by team convention.
- [ ] A/B/C cross-review complete and owner confirmation recorded.
- [ ] Freeze note declares: **No feature changes unless Critical Fix.**

## 15. Definition of Done

A task is DONE only when implementation is complete, clean build passes, relevant focused/integration tests pass, no regression is found, related docs are updated, output matches requirement/spec, and reviewer or the recorded verification checklist passes. Writing code alone is never DONE.

At project level, every mandatory row R01–R15 must answer: **implemented where? tested by which log? documented where?** An unanswered question means FAIL and blocks code freeze; final prose merge follows in Tuần 11.

## 16. Final Status Template

```text
Mandatory implementation: PASS / FAIL
Mandatory docs: PASS / FAIL
Required tests: PASS / FAIL
Regression: PASS / FAIL
Vector bonus: PASS / FAIL
Multi-agent bonus: PASS / FAIL
VLM/GUI Agent bonus: PASS / FAIL
Critical bugs remaining: N
Code freeze: YES / NO
Final revision: <commit/tag>
Verification date: <YYYY-MM-DD>
```

## 17. Next Week — Documentation and Presentation Phase

- Merge, format and cross-review the reports, README and submission checklist using only frozen source/test evidence; reconcile a discovered factual error through the Critical Fix/re-freeze rule.
- Build slide deck from the frozen revision and the approved evidence, including the three bonuses only if their final status is PASS.
- Rehearse a safe demo flow: build/test output, one CLI task, benchmark summary/trajectory, one OOP pattern, and bonus only if PASS.
- Record/edit the YouTube Unlisted video; never show API key and never call fallback-assisted output proof of model reasoning.
- Prepare oral explanations of each member’s owned code, tests, limitations and design decisions.
- No implementation feature work. A critical demo/mandatory-requirement fix follows targeted test, full regression and a new re-freeze note.
