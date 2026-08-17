# Tuần 10 — Tổng quan hoàn tất & Code Freeze

**Mục tiêu cuối tuần:** hoàn tất code bắt buộc → test/evidence → 3 bonus → full regression → **code freeze**.  
**Tuần 11:** merge/format docs, PowerPoint, video demo và oral; không thêm feature mới.

## Việc chung trước khi freeze

- [ ] Merge về một freeze candidate; không còn conflict marker.
- [x] Fresh rebuild đủ 8 target trên revision `3db1afb`; `test_tools`, `test_harness`, `test_multi_agent`, `test_template_method`, `test_role_a` và CTest 5/5 PASS (2026-08-17).
- [ ] Kiểm tra benchmark/action-level, artifact cũ không false-pass, API key/config/database/build không vào package.
- [ ] Khóa source path, test log và claim cho report/README; render đủ 4 UML.

## Role A — Systems/Core

- [x] **A-10-01..04 — Client/parser/skill/loop (self-test PASS).**
   1. **Requirement:** R01, R05–R07.
   2. **Path:** CLI/Harness → LLMClient → AgentLoop → parser/tool → SkillLoader/LoopDetector → trajectory.
   3. **Contract:** `expected<..., LLMError>`, only valid protocol becomes tool action, state resets each run, failure has reason.
   4. **Failures:** timeout/connect/JSON, malformed protocol, no skill match, repeat/ping-pong/max-step.
   5. **DoD:** `benchmark/test_role_a.cpp` (target `test_role_a`) 12 case + CTest 5/5 PASS; prompt captures skill injection, bad intent not final answer, reason non-empty.
   6. **Review:** B/C reruns offline workflow and checks trajectory/layer boundary.

- [x] **A-10-05..07 — C++/OOP/UML (self-test PASS; render env-limited).**
   1. **Requirement:** R09–R12.
   2. **Path:** source/CMake → feature matrix/report → four Mermaid diagrams → render artifacts.
   3. **Contract:** fallback builds without C++26 header; docs match source; limitations remain explicit.
   4. **Failures:** sanitizer finding, unsupported feature, stale diagram/claim, render error.
   5. **DoD:** `testCppFeatureMatrix` + 3 OOP-pattern cases PASS; 2 sequence diagrams validated via `mermaid.parse`; `class_diagram.md` updated to final source. mmdc render blocked (broken `commander` pkg + no Chromium) → documented limitation.
   6. **Review:** B/C independently verify diagram/report sections.

## Role B — Tools/Data

- [x] **B-10-01 — Tool core/negative paths (self-test PASS).**
  1. **Requirement:** R02–R04, đề §3.2.
  2. **Production path:** `AgentLoop` → `ToolRegistry` → concrete tool → `expected<string, ToolError>` → Harness.
  3. **Contract:** Registry `shared_ptr`, Factory `unique_ptr`; valid args trả output, invalid/policy/timeout trả `ToolError`; artifact/DB không vào package.
  4. **Failure cases:** malformed args, deny policy, calculator error, Git cấm, Web HTTP/network/timeout, Exec timeout.
  5. **DoD:** `wsl bash -lc "cmake --build build --target test_tools -j2 && ./build/test_tools"` → `ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY` (PASS lại 2026-08-13).
  6. **Review gate:** A/C chạy lại command trên freeze candidate và audit report Tools.

- [x] **BNS-V-01 — Vector Search (self-test PASS; external-model limitation).**
  1. **Requirement:** R16, đề §10.2.
  2. **Production path:** `memory vsave/vsearch` → `HashEmbedder` → SQLite embedding BLOB → cosine ranking.
  3. **Contract:** `MemoryTool` sở hữu DB/embedder, migration schema; missing text/DB error trả `ToolError`.
  4. **Failure cases:** empty command, DB/BLOB lỗi, DB cũ thiếu cột, không có vector memory.
  5. **DoD:** `test_cosine_similarity_fixed_vectors` + `test_memory_vector_search_ranking` trong `test_tools` PASS; legacy `save/search` regression PASS.
  6. **Review gate:** A/C chạy `vsave/vsearch`, xác nhận `memory.db` không stage/ZIP. HashEmbedder offline chưa chứng minh model embedding ngoài.

- [x] **BNS-G-01-B — Screenshot/action contracts (contract PASS, GUI bonus chưa xong).**
  1. **Requirement:** phần Role B của R18, đề §10.1.
  2. **Production path:** `capture_screenshot` → base64 data URI; `gui_action` → allow-list/validate → executor hook của C.
  3. **Contract:** chỉ click/type/key an toàn; default executor trả `NotFound`, không có side effect.
  4. **Failure cases:** không display, capture lỗi, action/key cấm, tọa độ/text không hợp lệ, executor chưa có.
  5. **DoD:** `test_screenshot_contract` + `test_action_tool_safety` PASS trong `test_tools`.
  6. **Review gate:** A/C xác nhận default không side effect; GUI bonus chỉ PASS sau workflow thật screenshot → VLM → executor do C chạy.

- [ ] **B-10-05 — External review.** A/C chạy lại workflow thật, ký mapping source → test → report; B không tự Accepted.

## Role C — Eval/Infra

- [ ] **C-10-01..03 — Eval/integration/benchmark (offline regression PASS; provider benchmark pending approval).**
  1. **Requirement:** R08, R13.
  2. **Path:** `run_eval` → Harness cleanup/run/evaluate → trajectories/summary.
  3. **Contract:** required-tool needs a real step; cleanup only generated files; token 0 = not measured.
  4. **Failures:** stale artifact, no-tool pass, invalid task, provider/timeout/failure reason.
  5. **DoD:** fresh rebuild 8 targets + CTest 5/5 PASS on `3db1afb`; an approved real-provider run must still have 10 trajectories 4/4/2.
  6. **Review:** A/B rerun required-tool workflow from clean state.

- [x] **BNS-M-01 — Multi-agent Harness integration (self-test PASS; review pending).**
  1. **Requirement:** R17.
  2. **Path:** `HarnessRunner::runMultiAgentDemo()` → `MultiAgentRunner` → 2 worker threads → report.
  3. **Contract:** timeout/join, two results required, report only written after both arrive.
  4. **Failures:** missing result/timeout, worker/tool error, report write failure.
  5. **DoD:** `test_multi_agent`, `demo_multi_agent`, CTest 5/5 PASS; report contains `CALC=1081` and `CAPITAL=`.
  6. **Review:** A/B rerun demo on freeze candidate before Accepted.

- [ ] **C bonus/freeze — VLM/GUI end-to-end, package (blocked).**
  1. **Requirement:** R17, R18, R14–R15.
  2. **Path:** Harness → 2 workers; screenshot → VLM → C executor; evidence → ZIP → clean extract → freeze note.
  3. **Contract:** bounded executor, timeout/join; ZIP excludes secrets/DB/artifacts; critical fix re-freezes.
  4. **Failures:** race/deadlock, invalid VLM action, unavailable display/model, secret/package leak, clean-build failure.
  5. **DoD:** focused demo + full regression; clean extraction follows README and tests PASS. Current blocker: tracked `libcurl4-openssl-dev_8.18.0-1ubuntu2.3_amd64.deb` must be removed by its owner before a package is created.
  6. **Review:** A/B run demo/package workflow before C declares Accepted/freeze.

## Thứ tự và điều kiện freeze

```text
Mandatory code + focused tests
        → clean build + CTest + evidence lock
        → Vector → regression
        → Multi-agent → regression
        → VLM/GUI → regression
        → package dry-run + clean extraction
        → CODE FREEZE
```

- [ ] Mọi requirement mandatory có source + test/log + doc citation.
- [ ] Cả 3 bonus PASS với focused test và full regression.
- [ ] Không còn critical bug, secret/artifact leak hoặc test fail.
- [ ] Ghi final commit/tag và freeze note: **No feature changes unless Critical Fix.**

Chi tiết task, dependency và DoD: [`KH_Tuan10_ChiTiet.md`](KH_Tuan10_ChiTiet.md).
