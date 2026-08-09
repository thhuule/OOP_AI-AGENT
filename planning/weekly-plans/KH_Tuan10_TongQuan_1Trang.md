# Tuần 10 — Tổng quan hoàn tất & Code Freeze

**Mục tiêu cuối tuần:** hoàn tất code bắt buộc → test/evidence → 3 bonus → full regression → **code freeze**.  
**Tuần 11:** merge/format docs, PowerPoint, video demo và oral; không thêm feature mới.

## Việc chung trước khi freeze

- [ ] Merge về một freeze candidate; không còn conflict marker.
- [ ] Clean build đủ 7 target; chạy `test_tools`, `test_harness`, `test_multi_agent`, `test_template_method` và CTest 4/4.
- [ ] Kiểm tra benchmark/action-level, artifact cũ không false-pass, API key/config/database/build không vào package.
- [ ] Khóa source path, test log và claim cho report/README; render đủ 4 UML.

## Role A — Systems/Core

- [ ] Test/fix LLM client: multimodal, timeout, connection refused, malformed JSON.
- [ ] Test/fix parser: JSON/fenced JSON/ACTION/function-call, malformed tool-intent, graceful stop.
- [ ] Chứng minh SkillLoader chọn và inject skill trước mỗi run; kiểm tra loop/failure reason.
- [ ] Chốt matrix C++17/20/23/26, fallback `inplace_vector`, sanitizer/MSVC limitation và review OOP/UML.

## Role B — Tools/Data

- [ ] Chốt tool names/aliases/descriptions/args/policy và report Tools.
- [ ] Test valid/invalid args của các tool; làm fixture Web/Exec timeout offline.
- [ ] Kiểm tra `memory.db`/artifact lifecycle và package hygiene.
- [ ] **Bonus Vector Search:** embedding, cosine similarity, ranking test, integration test và regression.
- [ ] Hỗ trợ GUI bonus: contract `capture_screenshot` và action-tool an toàn.

## Role C — Eval/Infra

- [ ] Kiểm tra Harness clean-state, failure taxonomy, action-level score, trajectory/post-condition.
- [ ] Chạy full offline integration/regression; chốt benchmark sạch nếu quota/network được duyệt.
- [ ] **Bonus Multi-agent:** Harness spawn 2 sub-agent song song, queue/mutex, timeout/join, focused test + demo log.
- [ ] **Bonus VLM/GUI:** action executor có validate/allow-list, controlled screenshot → VLM → action demo, test lỗi và regression.
- [ ] Dry-run package, clean extraction/build/test, freeze note/final revision.

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
