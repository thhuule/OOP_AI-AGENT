# Kế hoạch Tuần 10.75 — Tổng quan

> **Mục tiêu:** khôi phục production benchmark sau run Gemini thật `2/10`, rồi quyết định freeze dựa trên evidence mới. Không thêm feature và không bật lại deterministic fallback.

> **Chi tiết task/DoD:** [KH_Tuan10_75_ChiTiet.md](KH_Tuan10_75_ChiTiet.md).

## 1. Trạng thái hiện tại

| Area | Status | Evidence |
|---|---|---|
| Build + CTest | PASS | CTest `5/5 PASS` on 2026-08-19 |
| Vector | PASS evidence | `RUN_LIVE_OLLAMA=1 ./build/test_tools` PASS |
| Multi-agent | PASS evidence | `test_multi_agent` and demo PASS |
| Real-provider benchmark | FAIL | Gemini run `run_20260819_002033_735`: `2/10`, all actions `source: llm` |
| Code freeze | NO | mandatory benchmark workflow is failing |

`[requires tool]` is expected: the task must call a relevant tool successfully, not only return prose.

## 2. Confirmed gaps

1. Skills tell the model to call `file_read`/`file_write`, but the registry canonical names are `read_file`/`write_file`.
2. The AgentLoop parser truncates escaped JSON arguments to `"{\\"`.
3. File artifacts are therefore missing in task 002, 003, 005–010.
4. Gemini timeout/503 errors are real provider failures and must remain visible; they are not a reason to restore fallback.
5. The previous simple-task set did not explicitly cover calculator and TimeTool as required by §7.3; C-00 revises the suite while preserving 10 tasks and the 4/4/2 distribution.

## 3. Role allocation

| Role | Deliverable | Dependency |
|---|---|---|
| A | Correct escaped JSON parsing and canonical tool instructions in skills | None |
| B | Add backwards-compatible file aliases and verify FileTool contract | A parser examples agreed |
| C | Re-run real benchmark, lock evidence, update status/report claims | A + B merged |

## 4. Dependency map

```text
A: parser + skills ─┐
                    ├→ B: aliases/file integration → C: real run_eval → review → freeze decision
                    ┘
```

## 5. Exit gate

Before code freeze:

- [ ] `test_role_a`, `test_tools`, and CTest pass after the parser/alias fix.
- [ ] A fresh `run_eval` has 10 trajectories with `source: llm`, never `fixture`.
- [ ] File workflow tasks are no longer blocked by truncated args or unknown file aliases.
- [ ] A and B independently review the production trajectories.
- [ ] README/report/status all report the same fresh evidence.

Until every item passes: **Code freeze = NO**.
