# Kế hoạch Tuần 10.5 — Tổng quan hardening trước Acceptance

> **Mục tiêu:** chuyển project từ “có code/test cục bộ” thành “mọi requirement bắt buộc có production path, failure behavior, test, evidence và independent review”. Đây là tuần hardening; không thêm scope GUI/VLM.
>
> **Source of truth:** đề gốc [`OOP Project 2026 AI Agent.docx (1).md`](../reference/OOP%20Project%202026%20AI%20Agent.docx%20%281%29.md) → source/CMake/test hiện tại → artifact chạy mới → docs. Kế hoạch Tuần 10 chỉ là lịch sử điều phối.

## 1. Current status và scope quyết định

| Nhóm | Trạng thái đầu vào | Quyết định Tuần 10.5 |
|---|---|---|
| Mandatory core R02–R07, R09–R12 | Có implementation và focused tests lịch sử; chưa có acceptance review toàn chuỗi | Re-verify tại Gate 1–6, không viết lại nếu không có gap |
| LLM/config R01 | PARTIAL: interface có, production wiring/multimodal evidence thiếu | Fix và prove |
| Harness/benchmark R08, R13 | PARTIAL: pipeline có nhưng deterministic fallback làm sai evidence | Fix blocker rồi E2E |
| Docs/package R14–R15 | PARTIAL | Đồng bộ claim/run/config và clean-clone evidence |
| Vector R16 | PARTIAL: hash vector + cosine/SQLite, chưa Ollama embedding | Chọn làm bonus, chỉ PASS sau live acceptance |
| Multi-agent R17 | PARTIAL: threads/queue có nhưng demo false-success | Chọn làm bonus, chỉ PASS sau composite live workflow |
| GUI/VLM R18 | Out of scope | SKIPPED; không kéo sang tuần này |

## 2. Requirement coverage summary

| Requirement IDs | Observable outcome | Current status | Exit evidence |
|---|---|---|---|
| R01 | Provider-configured text/image request; errors are classified | PARTIAL | captured local mock-request assertions + controlled provider run |
| R02–R04 | registry and mandatory tools return valid output or ToolError | NOT VERIFIED | clean `test_tools` + independent workflow |
| R05–R07 | skills, ReAct parsing/history/stop loops work per run | NOT VERIFIED | `test_role_a` + trajectory inspection |
| R08, R13 | Harness records an honest 4/4/2 benchmark | PARTIAL | 10 fresh trajectories, no fallback source, JSON summary |
| R09–R12 | OOP/layer/C++/UML requirements match source | NOT VERIFIED | tests + source/diagram review |
| R14–R15 | reproducible build/run and accurate docs/package | PARTIAL | clean clone/extraction run and link/secret scan |
| R16 | Ollama `nomic-embed-text` vector retrieval and cosine rank | PARTIAL | live local Ollama test + regression |
| R17 | Harness runs two meaningful independent subtasks in parallel, no invented success | PARTIAL | success + worker-failure demo/tests |

## 3. Scope, owners và contract freeze

| Owner | Owns | Must not change without cross-role approval |
|---|---|---|
| Role A | `AgentLoop`, client/config request path, benchmark action provenance | `Tool` error enum and Harness output schema |
| Role B | Embedder/MemoryTool/WebSearch contract and DB lifecycle | AgentLoop parser/history and Harness aggregation |
| Role C | Harness multi-agent/E2E, evidence and docs reconciliation | provider request contract and tool error semantics |

**Frozen cross-role contract:** `LLMClient` returns `expected<response, LLMError>`; tools return `expected<string, ToolError>`; each Harness trajectory identifies action source (`llm`, `fixture`, or `tool`) and failure reason; a multi-agent aggregate is successful only if every required worker is successful. No owner may review their own task.

## 4. Work allocation and dependency

```text
W10.5-A-01 honest benchmark path ─┐
W10.5-A-02 provider config ───────┼→ W10.5-INT-01 E2E benchmark → W10.5-REG-01 → Acceptance
W10.5-A-03 Gemini images ─────────┘
W10.5-B-01 live vector ─→ W10.5-C-02 vector acceptance ─┘
W10.5-B-02 memory lifecycle ─────────────────────────────┤
W10.5-B-03 web failures ─→ W10.5-C-01 multi-agent demo ─┤
W10.5-C-03 docs/package ─────────────────────────────────┘
```

## 5. Test and review strategy

1. **Gate 1 — requirement:** each RQ maps to source, test and doc/evidence.
2. **Gate 2 — code:** clean configure/build; no prohibited fallback/path/config hard-code in changed production scope.
3. **Gate 3 — focused failure:** owner runs stated unit/failure command and stores output/JSON revision.
4. **Gate 4 — integration/E2E:** a different role runs actual workflow with clean state; mock tests alone are insufficient for R01/R08/R16/R17.
5. **Gate 5 — regression:** after every merge, run all test executables and CTest; final revision repeats E2E.
6. **Gate 6 — acceptance:** all mandatory rows are PASS; selected bonus is PASS or explicitly SKIPPED. No PARTIAL/NOT VERIFIED on mandatory rows.

## 6. Evidence and exit criteria

Evidence location must contain command, revision, UTC/local timestamp, inputs/config with secrets redacted, stdout/stderr or JSON, and reviewer identity. Required final evidence:

- Clean build and CTest output.
- One fresh benchmark result with 10 tasks (4 simple / 4 medium / 2 hard), trajectories and action provenance.
- Provider request/body tests and failures for timeout/connect/malformed JSON.
- Vector live Ollama record (`nomic-embed-text`) plus offline deterministic regression.
- Multi-agent success and injected-worker-error result; no `Tokyo` or other fabricated fallback.
- README/report/package check from clean directory; no secret/database/build artifacts.

**Exit:** `R01–R15 = PASS`, `R16/R17 = PASS or SKIPPED by explicit team decision`, 0 blocker/critical bug, Gate 1–6 PASS, and independent reviewers approve. Then code freeze; only `critical fix → focused test → regression → re-freeze` is allowed.
