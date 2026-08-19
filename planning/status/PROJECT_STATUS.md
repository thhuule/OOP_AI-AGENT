# Project Status — AI-AGENT OOP 2026

**Cập nhật lần cuối:** 2026-08-20
**Nguồn điều phối hiện hành:** [`KH_Tuan10_TongQuan_1Trang.md`](../weekly-plans/KH_Tuan10_TongQuan_1Trang.md) để theo dõi hằng ngày và [`KH_Tuan10_ChiTiet.md`](../weekly-plans/KH_Tuan10_ChiTiet.md) cho task/DoD chi tiết. Đề chính thức, source/test/CMake và artifact mới là nguồn xác minh; file này chỉ là trạng thái tóm tắt.

## Giai đoạn hiện tại

**Tuần 10 — Completion and code-freeze week.** Mục tiêu là hoàn tất requirement bắt buộc, test evidence và toàn bộ gate rồi freeze code **trong tuần này**. Tuần sau merge/format docs, làm slide, video/demo flow và oral preparation.

**Trạng thái freeze hiện tại: PENDING.** Code/test evidence đã được chạy lại trên candidate hiện tại; chỉ còn independent trajectory review và clean-extraction/package gate trước khi team tuyên bố freeze.

**Latest production evidence:** `run_20260820_002933_100` used Gemini `gemma-4-31b-it` with `source: llm` actions and no fixture fallback, achieving **7/10 final success** (`0.7` evaluator; `0.9` action-level). §7.3 requires reporting this success rate; it does not require a fixed 10/10 score. The recovery evidence is in [`KH_Tuan10_75_ChiTiet.md`](../weekly-plans/KH_Tuan10_75_ChiTiet.md).

## Trạng thái theo Role

### Role A — Systems/Core: CODE/FOCUSED TEST DONE

- `test_role_a` và CTest `5/5` PASS trên candidate đã chạy 2026-08-20.
- AgentLoop production path keeps fallback disabled; the latest real-provider trajectories are all `source: llm`.
- Parser now preserves escaped arguments and classifies malformed apparent tool calls; focused cases pass.

### Role B — Tools/Data: CODE/FOCUSED TEST DONE

- Registry aliases, FileTool end-to-end contract, WebSearch error matrix, memory lifecycle, and tool-focused tests pass.
- `RUN_LIVE_OLLAMA=1 ./build/test_tools` passed the live `nomic-embed-text` vector acceptance test on 2026-08-20.

### Role C — Eval/Infra: EVIDENCE RECORDED, REVIEW/PACKAGE GATES REMAIN

- Harness, evaluator, trajectory, multi-agent queue/thread baseline, CMake targets và 5 CTest tests đã có.
- Fresh real-provider run `run_20260820_002933_100` is recorded as 7/10; no fixture/fallback is claimed.
- `test_multi_agent`, all focused tests, and CTest 5/5 pass on the current candidate.
- Remaining: A/B independent trajectory review, clean extraction/package scan, then freeze note.

## Mandatory requirements not yet closed

The last mandatory closure items are independent review of the recorded production trajectories and the clean extraction/package scan. The benchmark score itself is documented evidence rather than a requirement for 10/10.

## Bonus status — committed for Tuần 10

- **Vector search (+4): PASS evidence.** Live Ollama acceptance using `nomic-embed-text` passed on 2026-08-20.
- **Multi-agent (+3): PASS focused evidence.** Worker lifecycle/error contract and demo-focused test pass; reviewer sign-off remains part of the final package gate.
- **VLM/GUI Agent (+8): not included in the declared Week 10 bonus closure.** Do not claim a full GUI/VLM end-to-end demo.

## Latest recorded evidence (not a freeze declaration)

- Current candidate: build completed; `test_role_a`, `test_tools`, `test_multi_agent`, and CTest `5/5` passed on 2026-08-20.
- Real provider: `run_20260820_002933_100`, Gemini `gemma-4-31b-it`, all actions `source: llm`, 7/10 final success.
- Historical 10/10 artifacts are retained only as history and are not claimed as the current model score.

## Freeze blockers / confirmations

- Any failing mandatory test, missing independent review, or package secret/artifact leak blocks freeze.
- Remaining evidence is procedural: A/B trajectory review and clean extraction/package scan. Do not alter features merely to chase a stochastic live-model score.

## Next action order

1. A/B independently inspect one stored production trajectory each and sign the W10.75 review gate.
2. Run the clean extraction/package scan, record its result, then declare code freeze if it passes.

After freeze: **No feature changes unless Critical Fix → targeted test → regression → re-freeze.** Tuần 11 docs chỉ được merge từ evidence đã khóa.
