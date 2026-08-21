# Project-Local Slide Authoring Instructions

This file guides any AI or team member editing the final deck. It is not an installable Codex skill.

## Role and Source Priority

Act as a technical presentation designer and fact checker. Use sources in this order:

1. `Presentation_Script_Detail.md` for slide order, speech, and ownership.
2. Requirement traceability and benchmark evidence for claims.
3. Current code/tests/README/report for implementation names.
4. `design.system.md` for visuals.

If sources disagree, the original requirement defines expected behavior; current code/test evidence defines implementation status.

## Required Output

Create exactly 18 slides:

- Role A: 1–7 — identity, overview, definitions, Agent Core.
- Role B: 8–12 — OOP/C++, tools, Vector.
- Role C: 13–18 — Harness, trajectory, Multi-agent, benchmark, conclusion, Thank You.

Each slide needs one takeaway, minimal text, one main visual, Vietnamese speaker notes, and a source note for technical claims. Q&A stays in the script only.

## Authoring Workflow

- Read the matching script section completely.
- Keep only keywords, labels, short metrics, or compact comparisons on-slide.
- Put explanation in notes and use exact production paths.
- Verify every class, model, score, C++ feature, and bonus claim.
- Place the Role A/B/C demo clips after Slides 7, 12, and 16 in the final hybrid video.

## Facts That Must Stay Exact

- C++17/20/23 are verified; C++26 evidence is deleted function with reason.
- Vector: `MemoryTool` → Ollama → `nomic-embed-text` → stored embedding → cosine search.
- `HashEmbedder` is test-only.
- Multi-agent: `HarnessRunner` → `MultiAgentRunner` → two worker threads → queue → report.
- Demo workers: Calculator computes `47 × 23`; Researcher finds Japan's capital.
- CTest 5/5 PASS.
- Benchmark: `run_20260820_002933_100`, `gemma-4-31b-it`, 7/10 final, 70% evaluator, 90% action-level.
- Bonus scope is Vector +4 and Multi-agent +3 only.

## Forbidden Claims

Do not claim benchmark 10/10, `std::inplace_vector`, completed GUI/VLM, production `HashEmbedder`, zero warnings/leaks, or automatic provider fallback without matching evidence. Do not expose secrets or private paths.

## Motion, Notes, and Video

- Fade within sections; Push at Slides 4, 8, and 13.
- Use short spoken handoffs at Slides 7, 12, and 16.
- Keep the final slide-plus-demo video within 8–10 minutes; do not create a second demo video.
- Explain diagrams naturally; do not read code line by line.
- Keep limitations honest because live-model behavior and service availability vary.

## Final Review Gate

- [ ] Exactly 18 slides; ownership A 1–7, B 8–12, C 13–18.
- [ ] Slide content, script, transition, and demo flow agree.
- [ ] Architecture arrows and all metrics match production evidence.
- [ ] Vector and Multi-agent have code, test, integration, and evidence.
- [ ] C++26 wording uses deleted function with reason.
- [ ] No Q&A or unverified claim appears in the deck.
- [ ] No secret, private path, or generated artifact is exposed.

## Reusable Generation Prompt

```text
Create the 18-slide AI Agent project deck from
planning/PowerPoint/Presentation_Script_Detail.md.

Follow planning/PowerPoint/design.system.md and this file. Preserve ownership
A 1–7, B 8–12, C 13–18. Keep slide text minimal, use diagrams for architecture
and runtime flows, and keep Q&A only in the script. Use Fade within sections,
Push at Slides 4/8/13, and prepare hybrid-video handoffs after Slides 7/12/16.
Verify all claims and benchmark metrics before generating the final PPTX.
```
