# Kế hoạch Tuần 10.75 — Production Benchmark Recovery

> **Ngày tạo:** 2026-08-19  
> **Source of truth:** `benchmark/results/run_20260819_002033_735/` trên revision sau Week 10.5.  
> **Mục tiêu:** sửa đúng các contract làm benchmark Gemini thật đạt `2/10`, sau đó chạy lại evidence. Không bật lại deterministic fallback và không thêm feature mới.

## 1. Trạng thái đầu vào

| Gate | Kết quả đã chứng minh | Kết luận |
|---|---|---|
| Build + CTest | Build all targets thành công; CTest `5/5 PASS` | PASS |
| Vector bonus | `RUN_LIVE_OLLAMA=1 ./build/test_tools` PASS | PASS evidence |
| Multi-agent bonus | `test_multi_agent` và `demo_multi_agent` PASS | PASS evidence |
| Production benchmark | Gemini `source: llm`, nhưng final success `2/10` | **FAIL — freeze blocked** |

`[requires tool]` trong output là yêu cầu hợp lệ của task: task chỉ pass action-level khi có ít nhất một tool phù hợp chạy thành công. Đây không phải lỗi.

## 2. Gaps phát hiện từ run thật

| ID | Evidence | Root cause đã xác định | Requirement | Owner |
|---|---|---|---|---|
| GAP-10.75-01 | task 003/007/010 gọi `file_read` → `TOOL_NOT_FOUND` | Skills hướng dẫn tên cũ `file_read`/`file_write`; registry chỉ canonical `read_file`/`write_file` | R02, R03, R05, R06 | A + B |
| GAP-10.75-02 | args JSON thành `"{\\"` ở task 003/007/009/010 | Parser tự tìm dấu quote, không decode escaped JSON string | R06 | A |
| GAP-10.75-03 | task 002/005/006/008 thiếu artifact | LLM không thể hoàn thành write path sau parser/alias mismatch; không được che bằng fallback | R03, R08, R13 | A + B, C verify |
| GAP-10.75-04 | timeout/503 và `LLM error: Malformed JSON` xuất hiện trong trajectories | Provider instability là failure evidence; cần giữ typed failure, không biến thành PASS | R01, R06, R08 | A + C |
| DOC-10.75-01 | report/status vẫn ghi benchmark lịch sử 10/10/fallback state cũ | Docs không phản ánh run thật mới | R14, R15 | C, B review |

## 3. Role A — Agent protocol and parser

### [ ] W10.75-A-01 — Parse the advertised JSON tool-call contract correctly

- **Requirement / gap:** R06; GAP-10.75-02.
- **Owner → reviewer:** A → C.
- **Production path:** Gemini response → `AgentLoop::think_and_act()` → `ToolCallAction` → `execute_tool()` → trajectory.
- **Current behavior:** the manual quote search truncates escaped JSON arguments, e.g. `"args":"{\\"path\\":\\"notes.txt\\"}"` becomes `"{\\"`.
- **Required change:** replace the manual JSON field extraction with one JSON parse path that preserves the complete string value of `args`. Fenced JSON must be supported only when it contains one valid tool-call object. Invalid JSON must become the existing classified failure path, never a partial tool call.
- **Contract freeze:** do not re-enable fallback; do not change `Tool::execute(std::string)` ownership or Harness JSON schema.
- **Failure cases:** escaped quote; nested JSON string; missing `tool`; missing `args`; non-string `args`; malformed/fenced JSON; multiple unrelated JSON blocks.
- **DoR:** [ ] exact accepted JSON examples selected [ ] tool error contract unchanged [ ] reviewer confirms trajectory assertion.
- **DoD:** [ ] focused test proves exact escaped argument survives parse [ ] malformed input does not invoke a tool [ ] existing parser variants remain PASS [ ] `test_role_a` + CTest PASS [ ] C independently reads emitted trajectory.
- **Evidence:** test name/output, revision, one trajectory showing the full args string.

### [ ] W10.75-A-02 — Align skill instructions with canonical tool protocol

- **Requirement / gap:** R05, R06; GAP-10.75-01.
- **Owner → reviewer:** A → B.
- **Files:** `skills/task_planner.md`, `skills/step_verifier.md`, `skills/error_recovery.md`.
- **Current behavior:** skills advertise `file_read` and `file_write`, which are not canonical registry names.
- **Required change:** use `read_file`, `write_file`, and `append_file`; include one exact JSON tool-call example using the string args format supported by the tools. Instruct the model to return one call per response and wait for the observation before the next call.
- **Failure cases:** legacy alias appears; a response contains plan text plus several calls; file content contains commas/newlines; tool error observation must lead to retry or explicit final failure.
- **DoD:** [ ] no `file_read`/`file_write` remains in active skill instructions [ ] SkillLoader test still injects all three skills [ ] B reviews examples against actual FileTool input contract.

## 4. Role B — Backward-compatible tool contract

### [ ] W10.75-B-01 — Support legacy file-tool aliases and prove file arguments end-to-end

- **Requirement / gap:** R02, R03; GAP-10.75-01, GAP-10.75-03.
- **Owner → reviewer:** B → A.
- **Production path:** parsed tool name/args → `ToolRegistry::normalize()` → FileRead/FileWrite/FileAppend tool → artifact.
- **Required change:** add the two backwards-compatible aliases `file_read → read_file` and `file_write → write_file`. Confirm whether the File tools accept the full preserved JSON string produced by A-01; if not, document one canonical string syntax in the skill and test that syntax. Do not add a second file implementation.
- **Failure cases:** unknown legacy name; invalid/missing path; missing content; malformed JSON-style argument; write then read exact content; append retains previous content.
- **DoD:** [ ] alias lookup test PASS [ ] file write/read/append integration test creates then verifies a temp artifact [ ] invalid input returns `ToolError`, no exception [ ] `test_tools` + CTest PASS [ ] A reviews canonical/alias behavior.
- **Evidence:** focused test output and temporary artifact cleanup proof.

## 5. Role C — Evidence, regression, and documentation

### [ ] W10.75-C-00 — Align the benchmark suite with §7.3

- **Requirement / gap:** R13; missing explicit simple TimeTool coverage.
- **Owner → reviewer:** C → A.
- **Current behavior:** the previous four simple tasks covered listing, file write/read, and shell execution; the suite did not explicitly include calculator and TimeTool as simple tasks.
- **Required change:** keep exactly ten tasks and the 4/4/2 distribution. Replace `task_001` with calculator `17 * 3 → 51`; retain `task_002`/`task_003` file write/read; replace `task_004` with a TimeTool request using empty args. The dynamic time result is evaluated by its required `time` tool action plus the stable `-` and `:` output separators.
- **Compatibility:** `run_20260819_002033_735` remains the pre-revision `2/10` baseline and must not be compared directly to the revised-suite score.
- **DoD:** [ ] task JSON is valid [ ] exactly 4 simple / 4 medium / 2 hard [ ] simple `required_tools` include `calculator`, `write_file`, `read_file`, and `time` [ ] focused regression PASS [ ] A approves the requirement mapping.

**Checkpoint 2026-08-19 (awaiting A review):**

- [x] `tasks.json` has 10 tasks with `simple=4`, `medium=4`, `hard=2`.
- [x] Simple task tools include `calculator`, `write_file`, `read_file`, and `time`.
- [x] CTest passes 5/5 after the suite revision.
- [ ] A approves the §7.3 requirement mapping; C-00 is not Accepted before that review.

### [ ] W10.75-C-01 — Re-run real-provider benchmark without fallback

- **Requirement / gap:** R08, R13; GAP-10.75-03, GAP-10.75-04.
- **Owner → reviewer:** C → A and B.
- **Dependency:** A-01, A-02, B-01 merged.
- **Command:** `cmake --build build -j2 && ctest --test-dir build --output-on-failure && ./build/run_eval`.
- **Pass criteria:** new run has 10 trajectories in 4/4/2 distribution; every recorded action has `source: llm`; no `fixture`; write/read tasks either meet evaluator post-condition or record a truthful typed failure. A provider timeout/503 is evidence, not a reason to restore fallback.
- **DoD:** [ ] summary and trajectories stored under a new run id [ ] inspect task 002, 005, 006, 008, 009, 010 artifact paths [ ] compare score to 2026-08-19 baseline [ ] A/B independently inspect one trajectory each.

### [ ] W10.75-C-02 — Update status and report claims from fresh evidence

- **Requirement / gap:** R14, R15; DOC-10.75-01.
- **Owner → reviewer:** C → B.
- **Required change:** record this baseline as `2/10`, update outdated fallback/CTest claims, and state the exact final status after C-01. Never present historical 10/10 as the current model score.
- **DoD:** [ ] Week 10.5/10.75/status point to the same run revision [ ] no claim says code freeze while mandatory benchmark is FAIL [ ] B approves report wording.

**Checkpoint 2026-08-19:**

- [x] Report records the real-provider `2/10` baseline and distinguishes it from historical fallback-assisted 10/10 evidence.
- [x] Submission checklist now requires CTest 5/5 and explicit calculator/file/time coverage in the four simple tasks.

### [ ] W10.75-B-02 — Close the remaining WebSearch error matrix

- **Transferred from:** W10.5-C-01 / B-03 review gate.
- **Owner → reviewer:** B → C.
- **Requirement:** R03; tool errors must be returned as typed, observable failures rather than a fabricated research result.
- **DoD:** [ ] offline fixtures cover timeout, HTTP failure, and malformed body [ ] `test_tools` passes [ ] C confirms the multi-agent worker reports `STATUS=FAIL` for the injected error.

### [ ] W10.75-C-03 — Lock Vector and Multi-agent final evidence

- **Transferred from:** W10.5-C-02 and C-01 acceptance.
- **Owner → reviewers:** C → A.
- **DoD:** [ ] record the existing live Ollama command/result and model in final evidence [ ] record multi-agent success plus injected failure/no fabricated capital [ ] run focused regression after A/B fixes [ ] reviewer signs both bonus rows PASS or explicitly SKIPPED.

### [ ] W10.75-C-04 — Complete clean extraction, documentation, and package gate

- **Transferred from:** W10.5-C-03.
- **Owner → reviewer:** C → B.
- **DoD:** [ ] clean extracted copy follows README configure/build/test steps [ ] Markdown links/diagrams are checked after docs reorganisation [ ] secret/database/build-artifact scan is recorded [ ] reports state historical 10/10 versus current 2/10 correctly.

### [ ] W10.75-C-05 — Cross-role integration walkthrough

- **Transferred from:** W10.5-INT-01.
- **Owner → reviewers:** C → A and B.
- **DoD:** [ ] provider config → client request [ ] parser → registry → FileTool artifact [ ] tool error → trajectory [ ] Vector and Multi-agent workflows [ ] docs match the final revision.

### [ ] W10.75-C-06 — Final regression and freeze decision

- **Transferred from:** W10.5-REG-01.
- **Owner → reviewers:** C → A and B.
- **DoD:** [ ] clean build [ ] all focused targets + CTest PASS [ ] new real-provider benchmark is reviewed [ ] each R01–R15 row has source/test/doc/evidence [ ] status template is completed. Freeze only if no mandatory row is FAIL/PARTIAL/NOT VERIFIED.

## 6. Dependency and review gate

```text
W10.75-A-01 parser ─┐
W10.75-A-02 skills ─┼→ W10.75-B-01 aliases/file contract → W10.75-C-01 real benchmark
                    ┘                                      → W10.75-C-02 evidence → Freeze decision
```

**Acceptance gate:** all focused tests PASS, a new production run is reviewed independently, and R01–R15 have no open mandatory FAIL/PARTIAL rows. Until then:

```text
Code freeze: NO
No deterministic benchmark fallback in production.
```

## 7. Final status template

```text
Build + CTest: PASS / FAIL
Parser contract: PASS / FAIL
Skill/registry names: PASS / FAIL
File artifact workflow: PASS / FAIL
Real provider benchmark: <score>/10, LLM source verified YES/NO
Vector bonus: PASS / FAIL / SKIPPED
Multi-agent bonus: PASS / FAIL / SKIPPED
Mandatory docs: PASS / FAIL
Code freeze: YES / NO
```
