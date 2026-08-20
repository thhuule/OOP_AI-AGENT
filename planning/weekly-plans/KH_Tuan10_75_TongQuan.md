# Kế hoạch Tuần 10.75 — Tổng quan

> **Mục tiêu:** khôi phục production benchmark sau baseline Gemini thật `2/10`, ghi nhận evidence re-run trung thực, rồi quyết định freeze dựa trên các gate bắt buộc. Không thêm feature và không bật lại deterministic fallback.

> **Chi tiết task/DoD:** [KH_Tuan10_75_ChiTiet.md](KH_Tuan10_75_ChiTiet.md).

## 1. Trạng thái hiện tại

| Area | Status | Evidence |
|---|---|---|
| Build + CTest | PASS | CTest `5/5 PASS` on 2026-08-20 |
| Vector | PASS evidence | `RUN_LIVE_OLLAMA=1 ./build/test_tools` PASS on 2026-08-20 |
| Multi-agent | PASS evidence | `./build/test_multi_agent` PASS on 2026-08-20 |
| Real-provider benchmark | PASS as evidence; model score recorded | Gemini run `run_20260820_002933_100`: `7/10`, all recorded actions `source: llm`, no fixture |
| Clean package gate | PASS | candidate-only copy: hygiene PASS, clean CMake build PASS, CTest `5/5 PASS` on 2026-08-20 |
| Code freeze | PENDING | technical gates pass; independent A/B review and the team freeze declaration remain |

`[requires tool]` is expected: the task must call a relevant tool successfully, not only return prose.

## 2. Confirmed gaps

1. The former skill/registry name mismatch, escaped-argument parser defect, and raw malformed-call fall-through are fixed and covered by focused tests.
2. The suite now explicitly covers calculator, file write/read, and TimeTool in its four simple tasks, while preserving the required 4/4/2 distribution.
3. Gemini timeout/503 errors and model loop behaviour remain visible; they are not a reason to restore fallback.
4. The 2026-08-20 run still shows model-planning failures: task 004 chose `execute_shell(date)` rather than `time`; tasks 005 and 009 repeated a successful calculator call until `LoopDetector` stopped them. These are recorded benchmark outcomes, not proof of a missing tool implementation.

## 3. Post-audit requirement closure

The full-project audit found additional gaps that were not visible in CTest. They are now implemented and covered by focused tests:

| Area | Closure in the current candidate |
|---|---|
| Skills | Keyword metadata selects the relevant skill; `task_planner` is the default |
| Tool prompt | `ToolRegistry::catalog()` supplies real tool names and descriptions dynamically |
| ReAct history | LLM output is stored as `assistant`; tool observation is stored as `tool` |
| Trajectory/tokens | Gemini/Ollama usage metadata is recorded; final answer is exported without increasing tool-step count |
| Memory contract | `memory_save` and `memory_search` are production entries; primary save/search uses embedding + cosine similarity; legacy keyword mode is explicit |
| C++26 | `= delete("reason")` is compiled on the active GCC 15/C++26 toolchain |
| Multi-agent/docs | Harness integration and two-worker queue/thread path are documented; GUI/VLM is not claimed |
| Package | candidate-only extraction passes hygiene, clean build, and CTest |
| Git contribution | FAIL by current evidence: committed `src/` share is 91.62% / 6.20% / 2.18%; maximum personal gaps are 8.76 / 20.85 / 10.91 days. Team/instructor disposition required |

## 4. Role allocation

| Role | Deliverable | Dependency |
|---|---|---|
| A | Agent protocol, skill selection, dynamic prompt, history roles, LLM usage metadata | None |
| B | File compatibility plus vector-memory production contract | Agent/tool contract frozen |
| C | Harness export, benchmark/evidence, docs, clean package gate | A + B merged |

## 5. Dependency map

```text
A: parser + skills + classified malformed-call failure ─┐
                                                        ├→ B: aliases/file integration → C: real run_eval → review → freeze decision
                    ┘
```

## 6. Exit gate

Before code freeze:

- [x] `test_role_a`, `test_tools`, and CTest pass after the parser/alias fix.
- [x] Malformed apparent tool calls produce a stable classified failure, not a raw final answer.
- [x] A fresh `run_eval` has 10 trajectories with `source: llm`, never `fixture`.
- [x] File workflow tasks are no longer blocked by truncated args or unknown file aliases (tasks 002, 003, 006, 007 and 010 passed in the re-run).
- [x] Post-audit skill/catalog/history/token/vector/C++26 gaps have focused test evidence.
- [x] Tracked-only clean package passes hygiene, clean build, and CTest.
- [ ] A and B independently review the production trajectories.
- [ ] Team verifies the official Git contribution/commit-history rules and records an honest explanation; no history rewrite or fake recommit.
- [x] Exact normalized Git measurements are recorded in `docs/evidence/requirement_traceability_final_2026-08-20.md`.
- [x] Report/status/plan evidence has been synchronized to `run_20260820_002933_100`.

Remaining before declaring freeze: independent A/B trajectory review, Git-contribution disposition, and the team's freeze declaration. A non-10/10 live score alone does not block freeze because §7.3 requires reporting the selected model's success rate, not a perfect rate.
