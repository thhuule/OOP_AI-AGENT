# Project Status — AI-AGENT OOP 2026

**Cập nhật lần cuối:** 2026-08-20
**Nguồn điều phối hiện hành:** [`KH_Tuan10_TongQuan_1Trang.md`](../weekly-plans/KH_Tuan10_TongQuan_1Trang.md) để theo dõi hằng ngày và [`KH_Tuan10_ChiTiet.md`](../weekly-plans/KH_Tuan10_ChiTiet.md) cho task/DoD chi tiết. Đề chính thức, source/test/CMake và artifact mới là nguồn xác minh; file này chỉ là trạng thái tóm tắt.

## Giai đoạn hiện tại

**Tuần 10 — Completion and code-freeze week.** Mục tiêu là hoàn tất requirement bắt buộc, test evidence và toàn bộ gate rồi freeze code **trong tuần này**. Tuần sau merge/format docs, làm slide, video/demo flow và oral preparation.

**Trạng thái freeze hiện tại: PENDING.** Code/test/package evidence đã PASS trên candidate hiện tại; còn independent A/B review, xử lý trung thực Git contribution gate và tuyên bố freeze của team.

**Latest production evidence:** `run_20260820_002933_100` used Gemini `gemma-4-31b-it` with `source: llm` actions and no fixture fallback, achieving **7/10 final success** (`0.7` evaluator; `0.9` action-level). §7.3 requires reporting this success rate; it does not require a fixed 10/10 score. The recovery evidence is in [`KH_Tuan10_75_ChiTiet.md`](../weekly-plans/KH_Tuan10_75_ChiTiet.md).

## Trạng thái theo Role

### Role A — Systems/Core: CODE/FOCUSED TEST DONE

- `test_role_a` và CTest `5/5` PASS trên candidate đã chạy 2026-08-20.
- AgentLoop production path keeps fallback disabled; the latest real-provider trajectories are all `source: llm`.
- Parser now preserves escaped arguments and classifies malformed apparent tool calls; focused cases pass.
- Skill selection uses task keywords with `task_planner` as default; the system prompt uses the live registry catalog.
- Conversation history now records LLM replies as `assistant` and observations as `tool`.
- Gemini/Ollama token metadata and final-answer trajectory steps are exported and tested.
- The active GCC 15/C++26 build accepts deleted-function reasons; `inplace_vector` remains a documented portability fallback.

### Role B — Tools/Data: CODE/FOCUSED TEST DONE

- Registry aliases, FileTool end-to-end contract, WebSearch error matrix, memory lifecycle, and tool-focused tests pass.
- `RUN_LIVE_OLLAMA=1 ./build/test_tools` passed the live `nomic-embed-text` vector acceptance test on 2026-08-20.
- Exact `memory_save`/`memory_search` production entries now use embedding + cosine search; keyword mode is explicitly legacy.

### Role C — Eval/Infra: TECHNICAL GATES PASS, REVIEW REMAINS

- Harness, evaluator, trajectory, multi-agent queue/thread baseline, CMake targets và 5 CTest tests đã có.
- Fresh real-provider run `run_20260820_002933_100` is recorded as 7/10; no fixture/fallback is claimed.
- `test_multi_agent`, all focused tests, and CTest 5/5 pass on the current candidate.
- The final 170-file candidate package passed repository hygiene, clean CMake build, and CTest 5/5.
- Remaining: A/B independent review and the team freeze note.

## Mandatory requirements not yet closed

The remaining closure items are independent A/B review and the official Git contribution gate. The exact audit records committed `src/` ownership as **91.62% / 6.20% / 2.18%**, with maximum personal commit gaps **8.76 / 20.85 / 10.91 days**. This cannot be repaired honestly by rewriting/recommitting history. See [`requirement_traceability_final_2026-08-20.md`](../../docs/evidence/requirement_traceability_final_2026-08-20.md). The benchmark score itself is documented evidence rather than a requirement for 10/10.

## Bonus status — committed for Tuần 10

- **Vector search (+4): PASS evidence.** Live Ollama acceptance using `nomic-embed-text` passed on 2026-08-20.
- **Multi-agent (+3): PASS focused evidence.** Worker lifecycle/error contract and demo-focused test pass; reviewer sign-off remains part of the final package gate.
- **VLM/GUI Agent (+8): not included in the declared Week 10 bonus closure.** Do not claim a full GUI/VLM end-to-end demo.

## Latest recorded evidence (not a freeze declaration)

- Current candidate: build completed; `test_role_a`, `test_tools`, `test_multi_agent`, and CTest `5/5` passed on 2026-08-20.
- Clean package: 170 intended candidate files; hygiene PASS, clean build PASS, CTest `5/5 PASS`.
- Real provider: `run_20260820_002933_100`, Gemini `gemma-4-31b-it`, all actions `source: llm`, 7/10 final success.
- Historical 10/10 artifacts are retained only as history and are not claimed as the current model score.

## Freeze blockers / confirmations

- Any failing mandatory test, missing independent review, or package secret/artifact leak blocks freeze.
- The package gate is closed. Remaining work is procedural: A/B review, an honest Git-contribution disposition, and the team freeze declaration. Do not alter features merely to chase a stochastic live-model score.

## Next action order

1. A/B independently inspect one stored production trajectory and the assigned post-audit contract each, then sign the W10.75 review gate.
2. Verify commit counts, spacing, author aliases, and source contribution; record the real result or instructor guidance without rewriting history.
3. Commit/push the reviewed candidate and declare code freeze by team convention.

After freeze: **No feature changes unless Critical Fix → targeted test → regression → re-freeze.** Tuần 11 docs chỉ được merge từ evidence đã khóa.
