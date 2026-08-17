# Tuần 10 — Tổng quan hoàn tất & Code Freeze

**Mục tiêu cuối tuần:** hoàn tất code bắt buộc → test/evidence → 3 bonus → full regression → **code freeze**.  
**Tuần 11:** merge/format docs, PowerPoint, video demo và oral; không thêm feature mới.

## Việc chung trước khi freeze

- [ ] Merge về một freeze candidate; không còn conflict marker.
- [ ] Clean build đủ 8 target; chạy `test_tools`, `test_harness`, `test_multi_agent`, `test_template_method`, `test_role_a` và CTest 5/5.
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

- [ ] **A-10-10 — Production benchmark không dùng đáp án hardcode (BLOCKER).**
  1. **Requirement:** R06, R08, R13.
  2. **Path:** `run_eval` → AgentLoop/LLM → parser/tool → Harness trajectory; fallback chỉ cho fixture test.
  3. **DoD:** production disables fallback, test fixture enables it explicitly; `test_role_a`, `test_harness`, CTest 5/5 PASS; fresh provider trajectory records action source.
  4. **Runtime impact:** `run_eval` thực sự dùng provider, nên có quota/network latency và có thể fail thật; đây là evidence đúng thay vì 10 đáp án hardcode.

## Role B — Tools/Data

- [x] **B-10-01 — Tool core/negative paths (self-test PASS).**
  1. **Requirement:** R02–R04, đề §3.2.
  2. **Production path:** `AgentLoop` → `ToolRegistry` → concrete tool → `expected<string, ToolError>` → Harness.
  3. **Contract:** Registry `shared_ptr`, Factory `unique_ptr`; valid args trả output, invalid/policy/timeout trả `ToolError`; artifact/DB không vào package.
  4. **Failure cases:** malformed args, deny policy, calculator error, Git cấm, Web HTTP/network/timeout, Exec timeout.
  5. **DoD:** `wsl bash -lc "cmake --build build --target test_tools -j2 && ./build/test_tools"` → `ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY` (PASS lại 2026-08-13).
  6. **Review gate:** A/C chạy lại command trên freeze candidate và audit report Tools.

- [x] **BNS-V-01 — Vector Search (COMPLETED).**
  1. **Requirement:** R16, đề §10.2.
  2. **Production path hiện tại:** `memory vsave/vsearch` → `OllamaEmbedder(nomic-embed-text)` → SQLite embedding BLOB → C++ cosine ranking.
  3. **Contract:** `MemoryTool` sở hữu DB/embedder, migration schema; missing text/DB error trả `ToolError`.
  4. **Failure cases:** empty command, DB/BLOB lỗi, DB cũ thiếu cột, không có vector memory.
  5. **DoD:** B-10-06 Ollama embedder + A-10-08 runtime wiring + C-10-06 Ollama acceptance; `test_tools` (gồm `test_ollama_embedder`) & CTest 5/5 PASS.
  6. **Runtime impact:** `vsave/vsearch` ở production cần Ollama/model (`nomic-embed-text`); service lỗi trả lỗi rõ `ToolError::ExecutionFailed`, không fallback hash âm thầm. `save/search` cũ giữ offline regression.

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
  5. **DoD:** after A-10-10, fresh rebuild 8 targets + CTest 5/5 PASS; an approved real-provider run must still have 10 trajectories 4/4/2 and no task-specific fallback action.
  6. **Review:** A/B rerun required-tool workflow from clean state.

- [ ] **BNS-M-01 — Multi-agent (PARTIAL; strict demo evidence pending).**
  1. **Requirement:** R17.
  2. **Path:** `HarnessRunner::runMultiAgentDemo()` → `MultiAgentRunner` → 2 worker threads → report.
  3. **Contract:** timeout/join, two valid worker results required; no report/fake success if WebSearch fails.
  4. **Failures:** missing result/timeout, worker/tool error, report write failure must make demo non-zero.
  5. **DoD:** C-10-07 composite parallel demo + B-10-07 error contract + A-10-09 review; focused negative tests, `demo_multi_agent`, CTest 5/5 PASS.
  6. **Runtime impact:** demo now exposes worker/network failure instead of outputting fallback `Tokyo`; it needs network only if the selected research subtask uses web search.

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

## Hậu kiểm Tuần 10 → đầu vào Tuần 10.5

Checkbox Tuần 10 phía trên **giữ nguyên lịch sử**. Review source sau đó phát hiện bốn gate chưa thể Accepted: benchmark fallback hard-code (`HC-W10-001`), multi-agent demo có `Tokyo` fallback (`GAP-W10-002`), vector mới là `HashEmbedder` chứ chưa phải Ollama `nomic-embed-text` (`GAP-W10-003`), và config/multimodal provider chưa có production evidence (`REQMISS-W10-004/005`).

Tuần 10.5 chỉ harden/fix/test/docs: A xử lý production LLM/benchmark path; B xử lý embedding, memory và web-error contract; C xử lý composite multi-agent, E2E/docs/evidence. Không sửa checkbox Tuần 10; trạng thái chính thức được quyết định bởi Gate 1–6 trong [`KH_Tuan10_5_TongQuan.md`](KH_Tuan10_5_TongQuan.md).
