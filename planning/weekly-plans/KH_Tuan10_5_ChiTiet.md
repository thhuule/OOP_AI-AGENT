# Kế hoạch Tuần 10.5 — Chi tiết thực thi và Acceptance

> Các checkbox trong `KH_Tuan10_ChiTiet.md` là lịch sử và không bị sửa. Checkbox dưới đây là công việc sau review. **Owner ≠ Reviewer** là điều kiện bắt buộc.

## 1. Atomic requirements và traceability đầu vào

| ID                              | Hành vi observable / failure behavior                                                                                        | Implementation hiện tại                                             | Status / gap                 |
| ------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- | ---------------------------- |
| RQ-LLM-01 (đề §3.1)          | Config base URL/model/temperature/max_tokens tạo request Ollama-compatible; timeout/connect/malformed JSON trả`LLMError`. | client interface có; config production chưa chứng minh end-to-end. | PARTIAL — REQMISS-W10-004   |
| RQ-LLM-02 (đề §3.1)          | Một`Message` mang text + image, provider serializes image request; lỗi không crash.                                      | shared type có; Gemini image path cần verify/implement.             | PARTIAL — REQMISS-W10-005   |
| RQ-TOOLS-01 (đề §3.2)        | Registry runtime + 5 mandatory tools + error contract.                                                                        | Có source/tests.                                                     | NOT VERIFIED — TEST-W10-007 |
| RQ-SKILL-01 (đề §3.3)        | ≥3 Markdown skills được select/inject trước mỗi run.                                                                   | Có focused test lịch sử.                                           | NOT VERIFIED                 |
| RQ-AGENT-01 (đề §3.4–3.5)   | ReAct parser/history/max steps/loop stop output reason rõ.                                                                   | Có focused test lịch sử.                                           | NOT VERIFIED                 |
| RQ-HARNESS-01 (đề §3.6, §7) | Setup→run→evaluate→record; 10 task 4/4/2; trajectory honest.                                                               | Fallback can pre-answer benchmark.                                    | PARTIAL — HC-W10-001        |
| RQ-OOP-01 (đề §4–§5)       | Patterns, boundaries, UML and C++ feature quota match code.                                                                   | Existing evidence needs independent rerun.                            | NOT VERIFIED                 |
| RQ-DOC-01 (đề §IX)           | README/run/config/report/package claims reproducible and accurate.                                                            | Docs may disagree with CMake/provider behavior.                       | PARTIAL — DOC-W10-008       |
| RQ-VECTOR-01 (bonus §10.2)     | Persist Ollama`nomic-embed-text` embedding and C++ cosine retrieval.                                                        | Hash embedding, BLOB, cosine only.                                    | PARTIAL — GAP-W10-003       |
| RQ-MULTI-01 (bonus §10.3)      | Harness splits meaningful task into 2 parallel agents; failures are visible.                                                  | threads/queue exist; hardcoded task and fake`Tokyo` success.        | PARTIAL — GAP-W10-002       |

## 2. Task cards

### [x] W10.5-A-01 — Remove deterministic benchmark answers from production path

- **Requirement / gap:** RQ-HARNESS-01; HC-W10-001.
- **Owner → reviewer:** Role A → Role C.
- **Production path:** `benchmark/run_eval.cpp` → `AgentLoop::run()` / `think_and_act()` → LLM response → tool registry → `HarnessRunner` trajectory/JSON.
- **Current / expected:** current fallback recognizes benchmark wording and supplies answers before LLM. Production evaluation must never select task-specific answer/path fallback; fixtures may opt in explicitly and must label it `fixture`.
- **Specific change:** move deterministic plan behind test-only injection or an explicit fixture dependency; make `run_eval` create production AgentLoop with it absent. Add action provenance to trajectory if schema does not already capture source. Do not alter tool input/output contracts.
- **Input → output:** benchmark instruction + valid provider response → recorded LLM/tool action and post-condition; missing/invalid response → failed trajectory with `LLMError`, never synthetic success.
- **Failure cases:** provider timeout; connection refused; malformed tool call; benchmark phrase that formerly matched; fixture accidentally used by production.
- **DoR:** [x] requirement/path confirmed [x] trajectory schema owner agreed with C [x] fixture test identified [x] C review slot agreed.
- **Implementation/test checklist:** [x] test production path does not invoke fallback [x] fixture fallback test remains deterministic [x] `wsl cmake --build build --target test_role_a test_harness run_eval -j2` passes [x] `wsl ctest --test-dir build --output-on-failure` passes.
- **Evidence / DoD:** fresh trajectory for a formerly hardcoded task contains source `llm`/`tool`, input and raw/sanitized response reference; no preset answer exists in run_eval path; C independently reruns from clean generated-artifact state and approves.
- **Review gate:** C reads the one trajectory and source diff; reject if a provider failure still yields PASS.

### [x] W10.5-A-02 — Wire provider configuration into the actual request

- **Requirement / gap:** RQ-LLM-01; REQMISS-W10-004.
- **Owner → reviewer:** Role A → Role B.
- **Files/modules:** `benchmark/run_eval.cpp`, `src/client/llm_client.h`, `OllamaClient`/`GeminiClient` implementation, config example/README only after code contract is fixed.
- **Current / expected:** config is parsed but evidence does not prove endpoint/model/temperature/max_tokens reaches request. Each supported provider must consume the defined fields or reject unsupported fields explicitly.
- **Specific change:** pass one `LLMConfig` instance from config loading into the chosen client/request generation; validate required URL/model/key before request; keep secrets out of output.
- **Contract:** valid config → configured request; absent/invalid config → typed error before network; HTTP non-2xx, timeout, malformed JSON → classified `LLMError`; client owns no global config.
- **Failure cases:** missing config file/key/model/url, malformed numeric limits, connection refusal, timeout, bad JSON, provider error payload.
- **DoR:** [x] exact config field names/source agreed [x] no API key in evidence [x] reviewer assigned.
- **Test / evidence / DoD:** [x] local fake HTTP capture asserts URL/body contains model and permitted generation fields [x] negative fixture asserts each error class [x] focused client tests and CTest PASS [x] B independently repeats capture. Evidence includes redacted config, request assertions, command/revision.
- **Review gate:** B rejects if a field is merely read but not used, or silently ignored without documentation.

### [x] W10.5-A-03 — Prove multimodal serialization through supported provider

- **Requirement / gap:** RQ-LLM-02; REQMISS-W10-005.
- **Owner → reviewer:** Role A → Role C.
- **Production path:** `Message{text, images}` → selected `LLMClient` → provider JSON request → response/error.
- **Specific change:** make Gemini/Ollama request construction serialize a valid image part from `Message.images`; retain text-only behavior. No GUI/action feature is added.
- **Contract/failures:** empty/non-image/invalid data URI is rejected or returns `InvalidResponse`; text+image appears in one request; network/JSON failures return `LLMError`.
- **DoR:** [x] provider JSON format sourced from official provider spec [x] test uses small synthetic image [x] C reviewer booked.
- **DoD:** [x] no-network body test verifies text and image part [x] invalid image fixture fails safely [x] text-only regression passes [x] `test_role_a`/CTest PASS [x] C independently inspects emitted body and records evidence.

### [ ] W10.5-B-01 — Replace hash-only vector production path with Ollama embeddings

- **Requirement / gap:** RQ-VECTOR-01; GAP-W10-003.
- **Owner → reviewer:** Role B → Role A.
- **Production path:** config → `OllamaEmbedder(nomic-embed-text)` → `MemoryTool::vsave` BLOB → `vsearch` C++ cosine ranking → tool result.
- **Current / expected:** `HashEmbedder` gives deterministic test vectors but cannot satisfy the stated Ollama semantic embedding bonus. Production uses `/api/embed` with configured base URL/model; fixed-vector fake remains only test seam.
- **Specific change:** add injectable `Embedder` implementation with HTTP timeout/status/JSON/dimension validation; wire it only in production setup; keep `HashEmbedder` for offline tests.
- **Contract:** save text → persisted nonempty same-dimension vector; search query → top-k ranked IDs/text; unavailable model/network/malformed response/dimension mismatch → `ToolError::ExecutionFailed`, never hash fallback.
- **Failure cases:** empty input, Ollama down/timeout/non-2xx, malformed embeddings, zero/unequal dimensions, SQLite/BLOB failure, old DB migration.
- **DoR:** [ ] endpoint/model confirmed [ ] C owns live environment [ ] DB migration plan and reviewer set.
- **DoD:** [ ] deterministic unit vectors/ranking remain PASS [ ] mock HTTP tests cover all failure cases [ ] controlled `ollama pull nomic-embed-text` and live `vsave`/`vsearch` proves semantic nearest result [ ] `test_tools` + CTest PASS [ ] A reviews ownership/RAII and no silent fallback.

### [ ] W10.5-B-02 — Make MemoryTool lifecycle and storage path explicit

- **Requirement / gap:** RQ-TOOLS-01/RQ-VECTOR-01; HC-W10-006.
- **Owner → reviewer:** Role B → Role C.
- **Files:** `MemoryTool.*`, runner construction, ignore/package docs.
- **Current / expected:** fixed `memory.db` in working directory and constructor ignores initialization failure. Path is injected/configured with a safe default; unavailable DB is returned as a tool failure, not a later null/undefined operation.
- **Contract/failures:** tool owns `sqlite3*`, destructor closes it; all public commands check ready state; invalid/non-writable path/migration failure returns `ExecutionFailed`; no database is staged/package artifact.
- **DoR:** [ ] chosen path precedence documented [ ] backward compatibility/migration test named [ ] C reviewer assigned.
- **DoD:** [ ] temp writable path pass [ ] invalid/permission-denied fixture returns typed error [ ] old schema migration verified [ ] shutdown/cleanup regression passes [ ] C reviews package scan and `test_tools`/CTest result.

### [ ] W10.5-B-03 — Make WebSearch failure contract consumable by Harness

- **Requirement / gap:** RQ-TOOLS-01/RQ-MULTI-01; TEST-W10-007.
- **Owner → reviewer:** Role B → Role C.
- **Current /expected:** worker needs distinguishable web errors; no caller may convert error to answer.
- **Contract:** empty query invalid; timeout/connect/HTTP non-2xx/malformed body return stable `ToolError` plus safe diagnostic; valid response returns normalized text only.
- **DoD:** [ ] offline seam tests each outcome [ ] error is propagated to caller unchanged enough for report [ ] `test_tools` passes [ ] C injects the error through W10.5-C-01 and approves.

### [ ] W10.5-C-01 — Replace toy multi-agent demo with honest composite parallel workflow

- **Requirement / gap:** RQ-MULTI-01; GAP-W10-002, HC-W10-002.
- **Owner → reviewer:** Role C → Role A.
- **Production path:** `demo_multi_agent` / `HarnessRunner::runMultiAgentDemo()` → two named worker threads + queue/mutex → tool/agent subtask result → join/aggregate → exit code/report.
- **Current / expected:** fixed math/research task and `value_or("Tokyo")` allow false PASS. A supplied composite input is split into two independent subtasks, both start before aggregation, and final success requires both valid results.
- **Contract:** producer owns each message until queue transfer; workers return `{status, value, error, latency}`; runner joins all threads before cleanup; failed/missing/timeout worker produces FAILED aggregate and nonzero exit; no invented data.
- **Failure cases:** web/tool error, timeout, missing worker result, exception, duplicate result, report write failure; all must cleanly join and explain failure.
- **DoR:** [ ] A/B frozen error/action contract [ ] test inputs deterministic or live condition documented [ ] reviewer assigned.
- **DoD:** [ ] success test asserts two worker IDs, two independent outputs, measured overlap/parallel start and combined report [ ] injected web failure asserts no `Tokyo`/success output and nonzero exit [ ] cleanup/no hang test [ ] `test_multi_agent`, `demo_multi_agent`, CTest PASS [ ] A independently runs actual workflow.

**Checkpoint 2026-08-17 — implementation/test evidence (chưa Accepted):**

- [x] `runMultiAgentDemo(MultiAgentDemoInput, report_path)` nhận hai subtask độc lập thay vì worker tự bỏ qua input; `demo_multi_agent` nhận calculator expression và research query từ CLI.
- [x] Worker trả lỗi có định danh (`ERROR=<worker>:<ToolError>`); aggregate ghi `STATUS=FAIL`, không ghi capital giả, và `demo_multi_agent` trả non-zero khi một worker fail.
- [x] `test_multi_agent` PASS: success flow dùng calculation input `2 * 3`; failure flow inject query rỗng, xác nhận `STATUS=FAIL`, `ERROR=researcher:InvalidArgument`, và không có `CAPITAL=`.
- [x] `demo_multi_agent '2 * 3' 'Japan capital'` PASS, report có hai output thật và `STATUS=PASS`; worker threads đều được `stopAndJoinAll()` trước khi return.
- [ ] B-03 cung cấp/verify WebSearch timeout/HTTP/malformed-body contract để thay error fixture bằng full web-failure matrix.
- [ ] A chạy lại workflow độc lập và xác nhận two-worker parallel evidence trước khi task được Accepted.

### [ ] W10.5-C-02 — Vector bonus independent acceptance

- **Requirement / gap:** RQ-VECTOR-01; depends on B-01/B-02.
- **Owner → reviewer:** Role C → Role A.
- **Workflow:** fresh configured local Ollama → isolated temp DB → `vsave` two or more distinct facts → `vsearch` paraphrase → inspect rank/output → delete temp DB.
- **Pass/fail:** PASS only if configured `nomic-embed-text` request succeeds and expected semantic item is ranked first; Ollama unavailable is FAIL/BLOCKED, not hash fallback PASS. Capture model/version, redacted config, commands, result IDs and cleanup.
- **DoD:** [ ] run carried out by C [ ] A repeats from clean directory [ ] regression artifacts attached to final status.

**Checkpoint 2026-08-18 — live acceptance PASS (chưa Accepted):**

- [x] Ollama WSL service trả `/api/embed` với model `nomic-embed-text` và vector 768 chiều.
- [x] `RUN_LIVE_OLLAMA=1 ./build/test_tools` PASS: `MemoryTool` lưu `weather forecast for Tokyo` và một fact C++; `vsearch Tokyo weather` trả fact thời tiết đúng thứ hạng đầu.
- [x] Test dùng DB tạm `artifacts/live_vector_acceptance.db` và xóa sau khi đóng `MemoryTool`; CTest mặc định vẫn offline, không phụ thuộc Ollama.
- [ ] A rerun cùng command từ clean directory và review rằng production configuration truyền `ollama_host`/`embedding_model` vào `MemoryTool` trước khi claim Vector bonus PASS.

### [ ] W10.5-C-03 — Reconcile README/reports/package with verified behavior

- **Requirement / gap:** RQ-DOC-01; DOC-W10-008.
- **Owner → reviewer:** Role C → Role B.
- **Scope:** `README.md`, relevant `docs/` report sections, submission checklist; do not alter source.
- **Specific corrections:** documented build targets equal CMake; config distinguishes supported providers and redacts keys; run command states actual entry point; benchmark says whether actions were LLM/tool/fixture; bonus claims are PASS only with evidence; GUI/VLM is SKIPPED.
- **DoD:** [ ] links/render checked [ ] clean directory follows README to configure/build/test [ ] package/secret/artifact scan recorded [ ] B checks every claim against source/evidence and approves.

**Checkpoint 2026-08-18 — documentation reconciliation (chưa Accepted):**

- [x] README đã đồng bộ fallback production, action provenance, executable targets và strict multi-agent failure behavior.
- [x] CTest 5/5 PASS; live Vector evidence được ghi ở C-02, không claim Accepted trước independent review.
- [ ] Clean extraction/package scan và B claim review vẫn là freeze gate.

### [ ] W10.5-INT-01 — Cross-role integration and requirement walk-through

- **Owner → reviewers:** Role C → Roles A and B.
- **Dependency:** A-01..03, B-01..03, C-01..03 merged into one candidate.
- **Checklist:** [ ] provider config → client request [ ] agent → tool errors → trajectory [ ] vector live retrieval [ ] multi-agent success/failure [ ] docs match runtime [ ] no conflict markers [ ] source ownership/cleanup is observed.
- **DoD:** each mandatory RQ has `implementation path + test command/result + doc/evidence`; reviewers independently run their allocated workflow; any PARTIAL/NOT VERIFIED opens a new gap, not an acceptance waiver.

### [ ] W10.5-REG-01 — Final regression, evidence lock and Acceptance

- **Owner → reviewers:** Role C → Roles A and B.
- **Commands:** clean `cmake -S . -B build`; build all targets; run `test_role_a`, `test_tools`, `test_harness`, `test_multi_agent`, `test_template_method`; `ctest --test-dir build --output-on-failure`; then fresh RQ-HARNESS/RQ-VECTOR/RQ-MULTI workflows.
- **Pass:** every command exits 0; no stale artifact/fallback success; evidence points to one revision; clean extraction follows README; reviewer approvals recorded.
- **Failure:** stop freeze, assign a targeted task; after fix run targeted test plus this complete regression again.

## 3. Integration matrix

| Producer → consumer                  | Input / output                                  | Ownership, cleanup, error contract                                       | Verification   |
| ------------------------------------- | ----------------------------------------------- | ------------------------------------------------------------------------ | -------------- |
| A client → AgentLoop                 | `Message` → `expected<string, LLMError>`   | client request temporary; AgentLoop records failure and stops            | A-02/A-03      |
| AgentLoop → B tools                  | parsed action →`expected<string, ToolError>` | registry owns shared tools; tool owns its resource                       | B-03 + INT-01  |
| B embedder → MemoryTool → C harness | text → vector/ranked result                    | embedder request temporary; DB tool-owned/closed; no empty/hash fallback | B-01/B-02/C-02 |
| B WebSearch → C multi-agent          | query → value or typed failure                 | worker owns result after queue receipt; all threads join                 | B-03/C-01      |
| A provenance → C evidence            | action source/result → JSON/summary            | Harness owns artifact lifecycle; reviewer reads clean run                | A-01/REG-01    |

## 4. Final requirement-to-evidence table

| Requirement ID | Bug/Gap         | Task           | Owner | Reviewer | Test                                  | Evidence                 | Final Status |
| -------------- | --------------- | -------------- | ----- | -------- | ------------------------------------- | ------------------------ | ------------ |
| RQ-LLM-01      | REQMISS-W10-004 | A-02           | A     | B        | mock request + error fixtures         | redacted config/body/log | VERIFIED     |
| RQ-LLM-02      | REQMISS-W10-005 | A-03           | A     | C        | text/image body + invalid image       | captured body/log        | VERIFIED     |
| RQ-TOOLS-01    | TEST-W10-007    | B-02/B-03      | B     | C        | `test_tools` + failures             | test log                 | NOT VERIFIED |
| RQ-SKILL-01    | —              | INT-01         | C     | A        | `test_role_a`                       | prompt capture           | VERIFIED     |
| RQ-AGENT-01    | HC-W10-001      | A-01/INT-01    | A     | C        | role_a/harness                        | trajectory               | VERIFIED     |
| RQ-HARNESS-01  | HC-W10-001      | A-01/REG-01    | A/C   | C/B      | clean 10-task run                     | 10 JSON trajectories     | PARTIAL      |
| RQ-OOP-01      | —              | REG-01         | C     | A/B      | pattern/feature tests + source review | test logs/diagrams       | NOT VERIFIED |
| RQ-DOC-01      | DOC-W10-008     | C-03/REG-01    | C     | B        | clean README run                      | link/package scan        | PARTIAL      |
| RQ-VECTOR-01   | GAP-W10-003     | B-01/B-02/C-02 | B/C   | A        | mock + live Ollama + regression       | model/version/results    | PARTIAL      |
| RQ-MULTI-01    | GAP-W10-002     | C-01           | C     | A        | success + injected failure            | report/exit/log          | PARTIAL      |

## 5. Project-level Definition of Done and final status template

**DoD:** code is complete; prohibited hard-code is removed or marked legitimate with evidence; happy/failure/unit/integration tests PASS; contract unchanged or migration verified; docs match runtime; independent reviewer approves; final regression is on the same revision.

```text
Mandatory implementation: PASS / FAIL
Mandatory docs: PASS / FAIL
Required tests: PASS / FAIL
Regression: PASS / FAIL
Vector bonus: PASS / FAIL / SKIPPED
Multi-agent bonus: PASS / FAIL / SKIPPED
GUI/VLM bonus: SKIPPED
Critical bugs remaining: N
Code freeze: YES / NO
Evidence revision: <commit>
```
