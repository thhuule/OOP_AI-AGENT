# Project Status — AI-AGENT OOP 2026

**Cập nhật lần cuối:** 2026-08-17
**Nguồn điều phối hiện hành:** [`KH_Tuan10_TongQuan_1Trang.md`](../weekly-plans/KH_Tuan10_TongQuan_1Trang.md) để theo dõi hằng ngày và [`KH_Tuan10_ChiTiet.md`](../weekly-plans/KH_Tuan10_ChiTiet.md) cho task/DoD chi tiết. Đề chính thức, source/test/CMake và artifact mới là nguồn xác minh; file này chỉ là trạng thái tóm tắt.

## Giai đoạn hiện tại

**Tuần 10 — Completion and code-freeze week.** Mục tiêu là hoàn tất requirement bắt buộc, test evidence và toàn bộ gate rồi freeze code **trong tuần này**. Tuần sau merge/format docs, làm slide, video/demo flow và oral preparation.

**Trạng thái freeze hiện tại: NO.** Không được coi code/docs/test đã freeze chỉ dựa vào evidence ở revision cũ; mọi gate phải được chạy lại trên final freeze candidate.

## Trạng thái theo Role

### Role A — Systems/Core: CODE/FOCUSED TEST DONE, package hygiene remaining

- `test_role_a` đã được bổ sung; focused tests và CTest 5/5 PASS trên `3db1afb`.
- Tracked binary dependency `libcurl4-openssl-dev_8.18.0-1ubuntu2.3_amd64.deb` phải được owner bỏ khỏi Git trước package/freeze.

### Role B — Tools/Data: PARTIALLY DONE, error matrix/package remaining

- Registry/Factory, 5 tools, 3 tool bổ sung, alias/policy và focused registry test đã có.
- URL tham chiếu bổ sung và clean provider run được ghi trong artifact/kế hoạch mới hơn status cũ; vẫn phải đồng bộ lại revision/claim ở final docs.
- Còn phải đóng args/error matrix, Web/Exec timeout offline fixture, memory database lifecycle và tool report review.
- Owner tasks: B-10-01 đến B-10-05.

### Role C — Eval/Infra: OFFLINE REGRESSION DONE, provider/package gates remaining

- Fresh rebuild đủ 8 targets, `test_tools`, `test_harness`, `test_multi_agent`, `demo_multi_agent`, `test_template_method`, `test_role_a` và CTest 5/5 PASS trên `3db1afb`.
- `benchmark/tasks.json` có đúng 10 task theo quota 4 simple/4 medium/2 hard. Real-provider `run_eval` chưa chạy lại vì cần quota/cost approval; historical run vẫn chỉ là pipeline evidence có fallback limitation.
- Package dry-run, clean extraction và freeze note bị chặn đến khi binary `.deb` bị track được gỡ.

## Mandatory requirements not yet closed

- LLM multimodal + timeout/connection/malformed-JSON verification.
- Skill injection-before-every-run proof; parser/invalid tool-intent and loop/failure coverage.
- Tool args and Web/timeout offline error coverage.
- Feature/test matrix C++17/20/23/26, sanitizer and MSVC evidence/limitation.
- All four diagram renders; report/README claim and link audit.
- Clean final build, full tests/CTest, action-level/post-condition inspection, clean package/extraction.

## Bonus status — committed for Tuần 10

- **Vector search (+4): IN PROGRESS (BNS-V-01).** `Embedding.h/.cpp` (`HashEmbedder` deterministic + `cosine_similarity`), `MemoryTool` migration `embedding BLOB` + `vsave`/`vsearch`, focused test fixed vectors + integration ranking test PASS. Remaining: A review interface, C regression, optional real-model semantic proof (provider/quota).
- **Multi-agent (+3): SELF-TEST PASS, review pending.** `HarnessRunner::runMultiAgentDemo()` now spawns calculator/researcher workers through `MultiAgentRunner`, joins them and writes a combined report; `test_multi_agent` and CTest 4/4 passed on 2026-08-13. A/B must still rerun the demo on the freeze candidate before Accepted.
- **VLM/GUI Agent (+8): IN PROGRESS (BNS-G-01).** B đã đóng contract `capture_screenshot` (ScreenshotTool → base64) + action-tool an toàn (ActionTool `gui_action`, allow-list click/type_text/key_press + validate) + focused tests PASS. Remaining: C triển khai action executor + controlled end-to-end demo + regression; cần môi trường desktop + VLM duyệt. Không tự nhận demo thật đã chạy.
- The team has committed all three for Tuần 10. Each starts/merges only after mandatory gates PASS and requires focused tests plus full regression.

## Latest recorded evidence (not a freeze declaration)

- Plan/artifact records a clean provider run `run_20260807_085427_143` with 10 tasks PASS, labelled pipeline evidence with fallback limitation.
- Fresh offline gate on `3db1afb`: all 8 targets rebuilt; CTest 5/5 passed.
- These results must be repeated on the final candidate because the current repository has commits after the cited historical revisions.

## Freeze blockers / confirmations

- Any failing mandatory test, requirement without source/test/doc mapping, or package secret/artifact leak blocks freeze.
- Provider quota/network approval is needed only for a new real-provider benchmark. Without it, record the limitation; do not fabricate a fresh run.
- MSVC availability determines whether there is a real MSVC build log or an explicit limitation.
- Every deletion/rename and final ZIP name requires owner/team confirmation before packaging.

## Next action order

1. A/B close focused code/test gaps in parallel; C prepares clean-state checks.
2. Merge to one freeze candidate and run clean build, all focused tests, CTest and sanitizer.
3. Lock source/test evidence and render diagrams; merge/format/final-review docs in Tuần 11.
4. Implement and merge Vector Search, Multi-agent and VLM/GUI Agent one at a time; repeat full regression after each.
5. Dry-run package + clean extraction, record final revision, declare code freeze.

After freeze: **No feature changes unless Critical Fix → targeted test → regression → re-freeze.** Tuần 11 docs chỉ được merge từ evidence đã khóa.
