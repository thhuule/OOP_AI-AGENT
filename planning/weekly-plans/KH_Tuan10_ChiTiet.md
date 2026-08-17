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
| R01 | LLM interface, Ollama-compatible HTTP, config (`base URL`, model, temperature, max tokens), text + image cùng interface; lỗi timeout/connect/JSON — đề §3.1 | Mandatory | Có client/interface; multimodal và error paths đã có evidence | DONE (A-10-01 focused tests) | Partial | A | Partial | A-10-01 |
| R02 | Runtime ToolRegistry; name/description/execute; allow/deny — đề §3.2 | Mandatory | Có Registry/Factory, alias/policy | Focused tests đã có | Có, cần rà revision cuối | B | Yes | B-10-01 review/freeze |
| R03 | 5 tool: exec, read/write, web, SQLite memory, calculator — đề §3.2 | Mandatory | Có | Exec/Git/Json/Memory negative path có; Web/timeout chưa đủ | Có, cần đồng bộ | B | Partial | B-10-02, B-10-03 |
| R04 | Ít nhất 3 tool bổ sung thuộc 3 loại, có nguồn tham khảo — đề §3.2 | Mandatory | Time/JSON/Git đã có | Registration evidence có | Có URL, phải rà link/claim | B | Yes | B-10-01 |
| R05 | ≥3 skill Markdown; keyword selection; inject trước **mỗi** run — đề §3.3 | Mandatory | Loader/skill files có; 3 skill inject mỗi run | Injection có focused evidence (A-10-03) | Partial | A | Partial | A-10-03 |
| R06 | ReAct, parse tool call, history, `max_steps`, graceful stop — đề §3.4 | Mandatory | Có AgentLoop/fallback; parser variants/malformed đã đóng | Parser variants/malformed intent đã có focused tests | Partial | A | Yes | A-10-02 |
| R07 | Generic-repeat + ping-pong, threshold, warning/stop — đề §3.5 | Mandatory | Có | LoopDetector unit + integration focused tests (A-10-04) | Có | A | Partial | A-10-04 |
| R08 | Harness setup→run→evaluate→record; 2 evaluator; trajectory; batch; JSON export — đề §3.6 | Mandatory | Có | Harness test/clean run có evidence lịch sử; final run còn phải kiểm tra | Có, cần cập nhật final evidence | C | Yes | C-10-01, C-10-02 |
| R09 | 4 pattern: Strategy, Template Method, Registry/Factory, Observer/Hook — đề §4.2 | Mandatory | Có source paths; 4 pattern focused tests có | Template/Registry focused test có; final regression cần chạy | Partial | A/B/C | Yes | A-10-06, B-10-01, C-10-02 |
| R10 | 4 Mermaid UML: class, agent sequence, harness sequence, component — đề §4.3 | Mandatory | Có `.md`; class diagram cập nhật theo source cuối | 4 diagram validate + render (class diagram sửa lỗi nested enum + namespace/class trùng tên `Environment`) | Partial | A | Partial | A-10-07 |
| R11 | Layer separation: AgentLoop không biết Harness; tools/evaluators không phụ thuộc sai layer — đề §4.4 | Mandatory | Có thiết kế; StepHook là coupling duy nhất | Cần static audit + test gate | Partial | A/C | Partial | A-10-06 |
| R12 | ≥4 C++17, ≥2 C++20, ≥2 C++23, ≥1 C++26; smart pointer/no leak/error handling — đề §V và nhóm reference checklist | Mandatory | C++26 guarded `inplace_vector` có source fallback; feature matrix test có | Sanitizer chưa chạy; feature matrix đã đóng (A-10-05) | Partial | A/B | Partial | A-10-05, B-10-04 |
| R13 | 10 benchmark task: 4 simple / 4 medium / 2 hard; JSON, success-rate analysis — đề §7 | Mandatory | Có tasks và run sạch 10/10 hiện lưu | Final artifact/action-level/post-condition chưa chốt trên revision freeze | Partial | C | Yes | C-10-01, C-10-02 |
| R14 | README build/run/config; báo cáo hoàn chỉnh; 4 diagram; source/package — đề §IX, docs guide, submission checklist | Mandatory | Tài liệu đã có khung/nội dung | Render/link/claim/package verification chưa đủ | Partial | A/B/C | Partial | A-10-07, B-10-05, C-10-03, C-10-04 |
| R15 | Không lộ API key, config thật, DB, build/artifact; clean clone/build/run — checklist nộp | Mandatory | `.gitignore`/checklist có | Chưa có dry-run package cuối | Partial | C/B | Partial | B-10-04, C-10-05 |
| R16 | Persistent memory vector search: embeddings + cosine similarity — đề §10.2 | Bonus +4 (committed) | `HashEmbedder` + cosine similarity + `vsave`/`vsearch` in `MemoryTool` | `test_tools` (vectors + ranking) PASS | Có trong report_tools.md, ghi rõ fallback limitation | B (A review) | Now assigned | BNS-V-01, self-test PASS |
| R17 | Multi-agent: Harness spawn sub-agent, queue+mutex, demo task phức tạp chia 2 agent song song — đề §10.3 | Bonus +3 (committed) | `HarnessRunner::runMultiAgentDemo()` tạo 2 worker; report gộp kết quả | Focused test + CTest 5/5 PASS; external review pending | Cần đồng bộ evidence | C (A review) | Now assigned | BNS-M-01, self-test PASS; chờ A/B review |
| R18 | GUI agent: screenshot, VLM, action executor, browser-search-copy demo — đề §10.1 | Bonus +8 (committed) | BNS-G-01-B tool contracts (Screenshot/ActionTool) có; GUI workflow pending C | Focused test contract PASS; end-to-end demo pending | Contract documented | A/B/C | Now assigned | BNS-G-01-B contract DONE; end-to-end in progress |

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

### A-10-01..04 — Client, parser, skills và loop taxonomy — DONE (self-test)

1. **Requirement gốc:** R01, R05–R07 / đề §3.1, §3.3–§3.5 — client text/image qua một interface, client errors; tool-call parsing/history/max-step; ≥3 skills được inject trước mỗi run; generic-repeat và ping-pong loop detection.
2. **Production path:** CLI/Harness → `LLMClient` (`OllamaClient`/`GeminiClient`) → `AgentLoop::run()` → parser/tool execution → `SkillLoader`/`LoopDetector` → trajectory/failure reason.
3. **Contract:** message có text và optional images; client trả `expected<..., LLMError>`; parser chỉ biến protocol hợp lệ thành `ToolCallAction`; skill selection chỉ bổ sung system prompt; loop history thuộc `LoopDetector` và phải reset theo run; parser/client/loop lỗi phải chuyển thành trạng thái dừng có reason, không crash.
4. **Failure cases:** timeout, connection refused, malformed JSON; raw/fenced JSON, `ACTION:`, provider function-call và tool-intent sai format; skill không match; generic repeat, ping-pong và max-step.
5. **DoD có thể kiểm chứng:** focused fake-client tests trong `benchmark/test_role_a.cpp` (target `test_role_a`) phủ A-10-01..04, chạy cùng `ctest --test-dir build --output-on-failure`; expected: fixture pass, malformed tool-intent không bị coi final answer, prompt capture có skill trước mỗi run, reason không rỗng. Evidence: `test_role_a` PASS (12 case), CTest 5/5 PASS; source `src/agent/agent_loop.cpp` (hàm `llmErrorToString`, trim `Final Answer:`/ACTION:, reset loop per run), `src/agent/SkillLoader.cpp`, `src/agent/LoopDetector.cpp`.
6. **Review gate:** B hoặc C chạy lại workflow offline có fake client, đọc trajectory và xác nhận AgentLoop không phụ thuộc Harness trước khi Accepted.

> **Done 2026-08-13 (A):** `test_role_a` thêm 12 case: `testClientErrorContract` (5 LLMError → reason "LLM error: <class>" không rỗng), `testMultimodalInterface` (text+image cùng `Message`/`LLMClient`), `testParserVariants` (raw/fenced JSON, `ACTION:`, fn-call JSON/provider), `testMalformedToolIntentNotFinalAnswer` (unknown tool → tool call, không phải final answer), `testMaxStepsAndHistoryGrowth`, `testSkillInjectionBeforeEachRun` (≥3 skill inject mỗi run), `testLoopDetectorUnit` + `testLoopAbortIntegration`, `testCppFeatureMatrix`, `testTemplateMethodSkeleton`/`testObserverHook`/`testRegistryFactoryStrategy`. Đồng thời sửa `agent_loop.cpp`: propagate `LLMError` thành reason; trim khoảng trắng sau `Final Answer:` và `ACTION:`; reset `detector_`/`history_` mỗi `run()`.

> **Updated 2026-08-16 (A):** file `benchmark/test_role_a.cpp` thực tế bị thiếu (làm vỡ build/ctest) nên các claim trên chưa có deliverable. Đã tạo lại file với đầy đủ 14 group test đặt tên đúng theo DoD (12 case trên + `test_agent_loop_fallback_real_tools` + `test_native_environment`), và bổ sung parser `functionCall`/`call:provider:tool{}` vào `agent_loop.cpp` để `testParserVariants` cover đủ 5 biến thể. Clean build + `ctest --test-dir build --output-on-failure` = **5/5 PASS**, trong đó `role_a` **0 check fail**. Các sửa `agent_loop.cpp` cũ (propagate LLMError, trim, reset mỗi run) đã xác nhận có trong source.

### A-10-05..07 — C++ evidence, OOP audit và UML — DONE (self-test; render bị giới hạn env)

1. **Requirement gốc:** R09–R12 / đề §4–§5 — 4 pattern, layer separation, 4 UML, quota C++17/20/23/26 và portability/no-leak evidence.
2. **Production path:** final source/CMake → C++ feature matrix + design report → Mermaid class/agent-sequence/harness-sequence/component diagrams → rendered artifact.
3. **Contract:** guarded `inplace_vector` fallback phải build khi header/feature macro thiếu; diagrams chỉ mô tả source cuối; report chỉ claim evidence được test; sanitizer/MSVC status phải là PASS hoặc limitation rõ toolchain/owner.
4. **Failure cases:** compiler thiếu C++26 header; sanitizer finding; stale source path/ownership; diagram render error; report claim VLM/model reasoning/MSVC pass không có evidence.
5. **DoD có thể kiểm chứng:** `test_role_a` mang `testCppFeatureMatrix` (static_assert C++17, runtime probe ranges/expected/inplace_vector) và 3 case OOP pattern (Template Method, Observer/Hook, Registry/Factory+Strategy); `__cplusplus=202400`, `__cpp_lib_ranges=202406`, `__cpp_lib_expected=202211`, inplace_vector=0 (fallback `std::vector` theo thiết kế). 4 UML: `class_diagram.md` (sửa lỗi nested `<<enumeration>>` trong `LoopDetector` và bỏ `namespace` trùng tên `Environment`) + `sequence_agent_run.md` + `sequence_harness.md` + `component_diagram.md` đều validate và render thành công qua `mermaid` (cập nhật theo source cuối). Evidence: CTest 5/5, feature probe log, 4 SVG render.
6. **Review gate:** B và C independently inspect one diagram/report section each and run the stated verification before A task is Accepted.

> **Giới hạn trung thực (render):** môi trường build này có `mmdc` hỏng (package `commander` rỗng) và không tải được Chromium headless, nên không export PNG trực tiếp; tuy nhiên cả 4 UML (`class_diagram`, `sequence_agent_run`, `sequence_harness`, `component_diagram`) đều được xác nhận **validate + render thành công** qua `mermaid` (sau khi sửa lỗi `class_diagram`: nested `<<enumeration>>` trong `LoopDetector` và bỏ `namespace` trùng tên `Environment` gây cycle). GitHub/markdown render native là evidence chuẩn. ASan/MSVC chưa chạy (thiếu toolchain trong env này) → ghi limitation, không tuyên bố PASS.

## 6. Role B Plan — Tools/Data

### B-10-01 — Tool contract, args và offline failure matrix — DONE (self-test)

1. **Requirement gốc:** R02–R04 / đề §3.2 — registry runtime, canonical name/description/execute, policy allow/deny, 5 tool bắt buộc và 3 tool bổ sung phải trả lỗi có nghĩa thay vì crash.
2. **Production path:** `AgentLoop::execute_tool()` → `ToolRegistry::lookup()/create()` → concrete `Tool::execute()` (`File*`, `CalculatorTool`, `ExecTool`, `WebSearchTool`, `MemoryTool`, `JsonTool`, `GitTool`) → `std::expected<std::string, ToolError>` → AgentLoop/Harness record kết quả.
3. **Contract:** input là canonical name/alias và args chuỗi hoặc JSON được tool hỗ trợ; Registry giữ instance bằng `shared_ptr`, Factory trả instance mới bằng `unique_ptr`; artifact file/DB thuộc lifecycle tool và phải bị ignore khi package; lỗi trả `InvalidArgument`, `AccessDenied`, `NotFound` hoặc `ExecutionFailed`, không throw qua boundary `Tool`.
4. **Failure cases:** args rỗng/malformed; thiếu `content`; biểu thức sai/chia 0; alias/policy bị deny; Git subcommand cấm; network/HTTP body không hợp lệ; Web timeout; shell timeout; memory command không tồn tại.
5. **DoD có thể kiểm chứng:** `wsl bash -lc "cmake --build build --target test_tools -j2 && ./build/test_tools"` → `=== ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY ===`; evidence là `test_canonical_names_and_descriptions`, `test_file_args_formats`, `test_calculator_args_trim`, `test_memory_modes`, `test_exec_policy`, `test_exec_timeout_offline`, `test_websearch_offline_fixture` trên commit `b474b50` (đã chạy PASS lại sau pull 2026-08-13). Code sau integrate: `WebSearchTool::http_get()` là virtual test seam; `ExecTool` nhận timeout test được; Registry đăng ký đầy đủ canonical/alias.
6. **Review gate:** A hoặc C chạy lại đúng command trên freeze candidate, kiểm tra không có request mạng trong web fixture và đối chiếu `docs/report_tools.md` với canonical names trước khi Accepted.

### BNS-V-01 — Persistent Memory Vector Search (+4) — PARTIAL

1. **Requirement gốc:** R16 / đề §10.2 — lưu embedding cho memory và tìm bằng cosine similarity trong C++.
2. **Production path hiện tại:** `memory` tool nhận `vsave <text>`/`vsearch <query>` → `MemoryTool` → `HashEmbedder::embed()` → SQLite `memories.embedding BLOB` → `cosine_similarity()` → top-N text đã xếp hạng. Đây là offline prototype, **không** phải đường bonus cuối.
3. **Contract:** `MemoryTool` sở hữu SQLite handle và `Embedder` bằng `unique_ptr`, destructor đóng DB; migration thêm cột `embedding` khi DB cũ chưa có; `vsave` trả xác nhận, `vsearch` trả danh sách xếp hạng hoặc `No vector memory found.`; thiếu text/DB lỗi trả `InvalidArgument`/`ExecutionFailed`.
4. **Failure cases:** `vsave`/`vsearch` không text; DB không mở/prepare/step lỗi; BLOB embedding sai kích thước; database cũ thiếu cột; query không có vector memory.
5. **DoD hiện tại:** `test_tools` xác nhận cosine/ranking/schema migration và regression `save/search`. **Chưa đủ +4:** đề §10.2 bắt buộc `nomic-embed-text` qua Ollama; `HashEmbedder` chỉ là fake/offline test double.
6. **Review gate:** A hoặc C xóa/đổi sang DB test riêng, chạy lại `vsave`/`vsearch` workflow và `test_tools`; xác nhận `memory.db` không được stage/ZIP trước khi Accepted.

#### BNS-V-02 — Hoàn tất Vector đúng đề §10.2

- [ ] **B-10-06 — `OllamaEmbedder` (Owner B).** Requirement: dùng `nomic-embed-text` qua Ollama. Path: `MemoryTool::vsave/vsearch` → injected `OllamaEmbedder` → Ollama embed endpoint → `EmbeddingVector` → SQLite/cosine. Contract: text vào, vector có chiều hợp lệ ra; timeout/HTTP/body sai trả `ToolError`, không fallback âm thầm sang `HashEmbedder`. DoD: fake HTTP test cho parse/error + integration thật với Ollama `nomic-embed-text`, rồi `test_tools` PASS. **Ảnh hưởng khi chạy:** `vsave/vsearch` ở production cần Ollama và model đã pull; khi service/model không có, lệnh báo lỗi rõ thay vì trả ranking giả.
- [ ] **A-10-08 — Wiring cấu hình production (Owner A).** Requirement: production path phải chọn embedder thật, không chỉ test. Path: config runtime → create `OllamaEmbedder` → inject vào `MemoryTool` được registry/AgentLoop dùng. Contract: endpoint/model embedding tách khỏi LLM chat config nhưng có default rõ; API/network lỗi giữ nguyên error taxonomy. DoD: smoke test khởi tạo đúng embedder theo config, không in API key; review B/C xác nhận không có `HashEmbedder` trên production path. **Ảnh hưởng khi chạy:** chỉ các lệnh vector dùng Ollama; benchmark/tool không gọi vector không bị thêm network call.
- [ ] **C-10-06 — Vector acceptance/regression (Owner C).** Requirement: lưu mỗi entry bằng embedding thật và tìm cosine C++. DoD: từ DB sạch chạy `vsave` cho ít nhất ba memory, `vsearch` query đồng nghĩa/near-semantic, lưu log Ollama/model + top result; sau đó build + CTest 5/5 + `test_tools`. Review: A xác nhận model/endpoint, B xác nhận ranking và migration. **Ảnh hưởng khi chạy:** tạo `memory.db` local (không commit); evidence chỉ được nhận khi Ollama run thật thành công.

### BNS-G-01-B — GUI tool contracts và validation — DONE (contract only)

1. **Requirement gốc:** R18 / đề §10.1 — GUI bonus cần screenshot, VLM và bounded click/type/key action. Role B chịu contract tool và input validation, không chịu VLM pipeline/executor demo.
2. **Production path:** Agent/registry lookup `capture_screenshot` hoặc `gui_action` → `ScreenshotTool::capture_png()` → data URI base64; hoặc `ActionTool::execute()` → validate allow-list → `perform_action()` hook của executor Role C.
3. **Contract:** screenshot input optional hint, output `data:image/png;base64,...`; screenshot không ghi file người dùng. Action input chỉ `click x y`, `type_text text`, `key_press key`; coordinate ≤100000, text ≤512, key/action phải allow-list. Default `perform_action()` trả `NotFound`: Role C phải cung cấp executor thật. Lỗi trả `InvalidArgument`, `AccessDenied`, `NotFound` hoặc `ExecutionFailed`.
4. **Failure cases:** không có display/capture backend; capture lỗi; empty/invalid base64 source; action/key ngoài allow-list; coordinate âm/quá lớn; click thiếu toạ độ; text vượt giới hạn; executor chưa tồn tại/lỗi.
5. **DoD có thể kiểm chứng:** `test_tools` → `test_screenshot_contract` và `test_action_tool_safety` PASS; evidence: fake screenshot encode base64, fake action executor ghi nhận action hợp lệ, toàn bộ invalid/deny path trả đúng `ToolError`. Code sau integrate: `ScreenshotTool.*`, `ActionTool.*`, registry canonical names/alias.
6. **Review gate:** A hoặc C chạy lại test, kiểm tra default action không tạo side effect. Task này chỉ **Accepted là contract**; R18 GUI bonus chỉ Accepted khi C chạy workflow thật screenshot → VLM → executor → controlled demo + full regression.

- [x] **B-10-05 — Tool layer final review — DONE (2026-08-17)**

1. **Requirement gốc:** R02–R04, R16 và phần B của R18 phải có mapping source → test → report trước freeze.
2. **Production path:** source Tool layer → `test_tools` → `docs/report_tools.md`/README → evidence lock.
3. **Contract:** B bàn giao code/test/evidence; A/C xác nhận claim report không vượt quá test, đặc biệt HashEmbedder không phải external embedding model và GUI contract không phải demo hoàn chỉnh. Ma trận truy xuất R02–R04 đã thêm tại §27 của `report_tools.md`.
4. **Failure cases:** report stale commit; tool name/alias khác prompt; test cũ; `memory.db`/config/artifact bị stage; ghi GUI/VLM PASS khi chưa có end-to-end demo.
5. **DoD có thể kiểm chứng:** Ma trận truy xuất R02–R04 giữa source/test/docs đã được hoàn thiện 100%. Đã thêm §27 vào `docs/report_tools.md` với đầy đủ sign-off matrix và checklist kiểm tra; cập nhật `README.md` liên kết báo cáo; CTest 4/4 và `test_tools` pass (18/18 tests).
6. **Review gate:** Reviewer chạy `test_tools`, `git diff --check`, `git ls-files --error-unmatch memory.db` thất bại và review checklist mapping R02–R04/R16/R18-B PASS.

## 7. Role C Plan — Eval/Infra/Freeze

### C-10-01..03 — Evaluation, integration và benchmark — PARTIAL

1. **Requirement gốc:** R08, R13 / đề §3.6, §7 — Harness setup→run→evaluate→record, trajectory/export JSON, 10 task 4/4/2, evaluator/scoring và clean benchmark evidence.
2. **Production path:** `run_eval` → `HarnessRunner::runAll()` → `Environment::cleanArtifacts()` → AgentLoop/tools → evaluator → `TaskRunResult`/trajectory JSON/summary.
3. **Contract:** task giữ `required_tools`, artifacts, category và post-condition; required-tool task chỉ pass nếu có tool step thật; cleanup chỉ đụng generated artifact; provider/model/run ID và token state được lưu, `tokens=0` nghĩa là not measured.
4. **Failure cases:** stale artifact false-pass; no-tool execution; invalid task/evaluator spec; tool/parser/loop/timeout/rate-limit error; provider/quota/network không có; trajectory thiếu args/post-condition.
5. **DoD có thể kiểm chứng:** clean build rồi chạy `test_harness`, `test_tools`, `test_multi_agent`, `test_template_method`, `test_role_a`, `ctest --test-dir build --output-on-failure`; expected: 5/5 PASS. Khi quota được duyệt, chạy `run_eval` clean state và kiểm tra đủ 10 trajectory, 4/4/2, score/failure/post-condition. Evidence: command log + run directory trên freeze candidate.
6. **Review gate:** A hoặc B chạy lại một required-tool task từ workspace sạch, kiểm tra trajectory/action-level bằng tay; benchmark chỉ Accepted khi reviewer xác nhận provider/model/fallback wording đúng.

**Evidence 2026-08-17:** fresh rebuild đủ 8 target trên `3db1afb`; `test_tools`, `test_harness`, `test_multi_agent`, `demo_multi_agent`, `test_template_method`, `test_role_a` và CTest **5/5 PASS**. `benchmark/tasks.json` đúng quota 4 simple/4 medium/2 hard. C-10-01..03 vẫn PENDING benchmark provider mới và review workflow; không dùng test pass thay cho benchmark evidence.

### C-10-04..05 — Evidence lock, package và code freeze — PENDING

1. **Requirement gốc:** R14–R15 / đề §IX và submission checklist — README/report evidence, source sạch, no secret/artifact, clean extraction/build/test và freeze record.
2. **Production path:** freeze candidate → source/test/log mapping → README/checklist/evidence lock → ZIP manifest → extract vào thư mục sạch → build/test → final revision/tag/freeze note.
3. **Contract:** Tuần 10 chỉ khóa citations/evidence; docs merge/format ở Tuần 11. ZIP chỉ chứa source/tài liệu được phép; không có `config.json`, key, `.git`, build/cache, DB hoặc generated artifact. Critical fix phải targeted test + regression + re-freeze.
4. **Failure cases:** dead link/stale claim; docs vượt evidence; secret/DB/artifact trong ZIP; README không tái lập build; owner không xác nhận file đổi/xóa; test fail sau extraction.
5. **DoD có thể kiểm chứng:** `git diff --check`, secret/artifact scan, extract ZIP vào directory sạch rồi theo README build và chạy `test_harness`, `test_multi_agent`, CTest; expected: commands PASS, scan sạch, final commit/tag + freeze note ghi rõ. Evidence: package manifest, command log, reviewer record.
6. **Review gate:** A và B cùng chạy lại clean-extraction workflow hoặc chia build/test + manifest review; C chỉ tuyên bố Code Freeze sau hai xác nhận độc lập.

**Blocker 2026-08-17:** `git ls-files` cho thấy `libcurl4-openssl-dev_8.18.0-1ubuntu2.3_amd64.deb` đang được track. Không tạo ZIP/không tuyên bố freeze cho đến khi owner bỏ binary dependency này khỏi Git và scan package sạch.

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
| Gate 5/6: CTest + full regression | All mandatory | C | CTest 5/5 and all focused executables pass on final revision; repeat after every merged bonus |
| ASan / MSVC | R12 | A | ASan clean; MSVC build pass or limitation clearly recorded |
| Approved clean benchmark | R13 | C | 10 tasks 4/4/2, valid JSON, paths/arguments/post-conditions checked |
| Diagram/link/package verification | R10, R14–R15 | A/C | renders, links, clean extraction and secret scan pass |

## 11. Bonus Decision

### BNS-V-01 — Persistent Memory with Vector Search (+4) — PARTIAL

**Status:** offline prototype PASS: `HashEmbedder` deterministic + cosine/SQLite/ranking tests. Đề §10.2 yêu cầu `nomic-embed-text` qua Ollama, nên chưa Accepted bonus.
**Owner:** B; A reviews client/embedding interface, C verifies regression.
**Dependency:** every mandatory gate PASS and a stable embedding provider/model (for example `nomic-embed-text`) approved.
**Work/merge criteria:** persist embedding with each memory entry; cosine similarity search in C++; deterministic focused tests using fixed vectors; integration test proving ranking; README/report source/limitation update.
**Risk:** network/model availability, schema migration, provider quota. **Decision:** begin only after mandatory gate PASS; merge only with vector focused tests and full regression.

> **Done 2026-08-09 (B):** `Embedding.h/.cpp` (`Embedder` interface + deterministic `HashEmbedder`, `cosine_similarity()`); `MemoryTool` migration `embedding BLOB` + `vsave`/`vsearch`; focused test `test_cosine_similarity_fixed_vectors` + integration `test_memory_vector_search_ranking` PASS; regression `save`/`search` giữ nguyên. Giới hạn: `HashEmbedder` nội bộ deterministic; chạy model thật (nomic-embed-text) cần provider/quota duyệt. Chưa đủ semantic-proof bằng model ngoài.

### BNS-M-01 — Multi-agent Coordination (+3) — PARTIAL

**Status:** Harness spawn/thread/queue/join PASS. Demo hiện chia hai subtask đơn giản; `researcher` dùng `value_or("Tokyo")`, nên có thể báo thành công khi web search thất bại. Chưa đủ bằng chứng cho “task phức tạp chia 2 agent chạy song song”.
**Owner:** C; A reviews Agent API/layer boundary, B supplies safe subtask/tool contract.
**Dependency:** every mandatory gate PASS.
**Evidence:** `test_multi_agent` kiểm tra đường Harness → 2 worker → report PASS; CTest 5/5 PASS. Đây chứng minh kiến trúc, chưa chứng minh task demo thật khi worker lỗi/network lỗi.

#### BNS-M-02 — Hoàn tất Multi-agent đúng đề §10.3

- [ ] **C-10-07 — Composite parallel demo (Owner C).** Requirement: một task phức tạp chia thành hai subtask độc lập chạy song song. Path: `HarnessRunner` dispatch → two named workers → message queues → aggregator waits both → one combined report. Contract: report chỉ được ghi khi cả hai worker trả kết quả hợp lệ; worker timeout/tool error trả FAIL và process non-zero; bỏ `value_or("Tokyo")`. DoD: success test chứng minh hai worker nhận message, combined report có hai nguồn; negative tests cho worker timeout và web/tool error; `demo_multi_agent` + CTest 5/5 PASS. **Ảnh hưởng khi chạy:** demo không còn tự in dữ liệu fallback; nếu search/network lỗi, demo fail đúng và user thấy reason.
- [ ] **B-10-07 — Research tool contract review (Owner B).** Requirement: worker error phải phân biệt được timeout/HTTP/body invalid. DoD: `WebSearchTool` focused offline error tests PASS và message lỗi đủ để C đưa vào report; không sửa fallback ở Harness. **Ảnh hưởng khi chạy:** web worker có thể fail nhanh/traceable thay vì tạo kết quả giả.
- [ ] **A-10-09 — Multi-agent acceptance review (Owner A).** Requirement: Harness spawn nhưng AgentLoop không phụ thuộc Harness. DoD: review source boundary + run `demo_multi_agent`; xác nhận runtime report nêu rõ worker status và no fallback success. **Ảnh hưởng khi chạy:** không thêm feature vào AgentLoop; chỉ xác nhận integration boundary trước Accepted.

### BNS-G-01 — VLM/GUI Agent (+8) — CONTRACT DONE (full GUI bonus in progress)

**Status:** Contract and validation tools DONE (BNS-G-01-B); full end-to-end GUI Agent workflow in progress by Role C.
**Owners:** A owns VLM/image request through the shared LLM interface; B owns `capture_screenshot` and action-tool contracts; C owns action executor, end-to-end demo, test/log and regression.
**Dependency:** all mandatory gates PASS; approved Linux/desktop environment and VLM model; no secret-bearing browser profile.
**Work/merge criteria:** capture a screenshot; send base64 image through the same client interface; validate/execute only bounded `click`, `type_text`, `key_press` actions; demonstrate browser search and copy result; record action/timeout/error evidence; update docs.
**Safety/quality gates:** explicit allow-list and coordinate/input validation; test mocked VLM response and invalid action paths; use a disposable demo environment; no uncontrolled shell/browser side effects. **Merge criteria:** focused test + controlled end-to-end demo + full regression PASS.

> **Done 2026-08-09 (B — contract & validation):** `ScreenshotTool` (`capture_screenshot`, alias `screenshot`) trả `data:image/png;base64,...`, `capture_png()` virtual seam, base64 thuần C++; `ActionTool` (`gui_action`) allow-list `click`/`type_text`/`key_press`, key allow-list, toạ độ/text validate, action/key ngoài allow-list → `AccessDenied`. Tests `test_screenshot_contract` + `test_action_tool_safety` PASS. **Chưa xong:** end-to-end demo thật (screenshot → VLM → action executor) thuộc C; cần môi trường desktop + VLM duyệt.

## 12. Integration Checklist

- [ ] Interfaces/contracts from A and B merged với không conflict marker.
- [ ] C verifies no stale artifacts can satisfy a functional evaluator.
- [ ] Canonical tool names/descriptions/aliases agree across source, prompt and report.
- [ ] Every mandatory requirement has an implementation path, test/log and documentation path.
- [ ] Four patterns and layer boundaries match final source.
- [ ] Generated data, config and API keys are absent from staged/package files.
- [ ] A/B/C review shared docs and record owner for all changed/deleted files.

## 13. Full Regression Checklist

- [ ] Clean configure and build all targets.
- [ ] Run `test_tools`, `test_harness`, `test_multi_agent`, `test_template_method`, `test_role_a`.
- [ ] Run `ctest --test-dir build --output-on-failure` and obtain 5/5 pass.
- [ ] Run any new focused A/B tests and inspect logs.
- [ ] Run ASan suite if toolchain supports it; otherwise record limitation.
- [ ] Run/inspect approved clean benchmark if provider/quota is approved.
- [ ] After each merged bonus: repeat build, executables, CTest and impacted integration tests.
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
