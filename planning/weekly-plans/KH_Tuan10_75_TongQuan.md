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
| Code freeze | PENDING | benchmark is a reported model-quality measurement, not a requirement for a fixed 10/10; independent review and package gate remain |

`[requires tool]` is expected: the task must call a relevant tool successfully, not only return prose.

## 2. Confirmed gaps

1. The former skill/registry name mismatch, escaped-argument parser defect, and raw malformed-call fall-through are fixed and covered by focused tests.
2. The suite now explicitly covers calculator, file write/read, and TimeTool in its four simple tasks, while preserving the required 4/4/2 distribution.
3. Gemini timeout/503 errors and model loop behaviour remain visible; they are not a reason to restore fallback.
4. The 2026-08-20 run still shows model-planning failures: task 004 chose `execute_shell(date)` rather than `time`; tasks 005 and 009 repeated a successful calculator call until `LoopDetector` stopped them. These are recorded benchmark outcomes, not proof of a missing tool implementation.

## 3. Role allocation

| Role | Deliverable | Dependency |
|---|---|---|
| A | Correct escaped JSON parsing, canonical skill instructions, and classified malformed tool-call failure | None |
| B | Add backwards-compatible file aliases and verify FileTool contract | A parser examples agreed |
| C | Re-run real benchmark, lock evidence, update status/report claims | A + B merged |

## 4. Dependency map

```text
A: parser + skills + classified malformed-call failure ─┐
                                                        ├→ B: aliases/file integration → C: real run_eval → review → freeze decision
                    ┘
```

## 5. Exit gate

Before code freeze:

- [x] `test_role_a`, `test_tools`, and CTest pass after the parser/alias fix.
- [x] Malformed apparent tool calls produce a stable classified failure, not a raw final answer.
- [x] A fresh `run_eval` has 10 trajectories with `source: llm`, never `fixture`.
- [x] File workflow tasks are no longer blocked by truncated args or unknown file aliases (tasks 002, 003, 006, 007 and 010 passed in the re-run).
- [ ] A and B independently review the production trajectories.
- [x] Report/status/plan evidence has been synchronized to `run_20260820_002933_100`.

Remaining before declaring freeze: independent A/B trajectory review plus the clean-extraction/package gate. A non-10/10 live score alone does not block freeze because §7.3 requires reporting the selected model's success rate, not a perfect rate.
