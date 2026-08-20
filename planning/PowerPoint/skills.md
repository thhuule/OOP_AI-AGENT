# Project-Local Slide Authoring Instructions

This file defines how an AI or team member should turn the approved project material into the final PowerPoint. It is a project instruction file, not an installable Codex skill package.

## 1. Role

Act as both:

- a technical presentation designer;
- a fact checker for the C++ AI Agent project.

Prioritize clarity, evidence, and spoken explanation over decorative content.

## 2. Source Priority

Use sources in this order:

1. `planning/PowerPoint/Presentation_Script_Detail.md` — slide order, ownership, on-slide content, and speech.
2. Approved requirement traceability and benchmark evidence — factual verification.
3. Source code, tests, README, and report files — supporting diagrams and implementation names.
4. `planning/PowerPoint/design.system.md` — visual rules.

If sources disagree, keep the original project requirement as the behavioral source of truth and use the current code/test evidence for implementation status. Do not invent a resolution.

## 3. Required Output

Create exactly 15 slides in the approved order:

- Role A: slides 1–5 — core and architecture.
- Role B: slides 6–10 — OOP, C++ features, tools, and Vector.
- Role C: slides 11–15 — harness, Multi-agent, verification, results, and conclusion.

For every slide, produce:

1. Slide title.
2. One-sentence takeaway.
3. Minimal on-slide text.
4. One primary visual or diagram.
5. Speaker notes based on the approved script.
6. Evidence/source note for technical claims.

## 4. Authoring Workflow

For each slide:

- Read the complete matching section in `Presentation_Script_Detail.md`.
- Extract the single message the audience must remember.
- Keep only keywords, labels, short metrics, or a compact comparison on the slide.
- Move explanations into speaker notes.
- Replace prose with a diagram when it explains architecture, execution order, ownership, or data flow more clearly.
- Verify every class name, model name, score, feature, and bonus claim against evidence.
- Apply `design.system.md` consistently.

## 5. Technical Facts That Must Stay Exact

- Project language: modern C++ with verified C++17/20/23 features.
- C++26 evidence: deleted function with reason, not `std::inplace_vector`.
- Production model in the approved benchmark: `gemma-4-31b-it`.
- Vector production path: `MemoryTool` → Ollama embedder → `nomic-embed-text` → stored embedding → cosine search.
- Offline Vector tests may inject `HashEmbedder`; do not present it as production behavior.
- Multi-agent path: `HarnessRunner` → `MultiAgentRunner` → two worker threads → message queue → combined report.
- Demo workers: Calculator computes `47 × 23`; Researcher finds Japan's capital.
- CTest evidence: 5/5 PASS.
- Benchmark run: `run_20260820_002933_100`.
- Benchmark: 7/10 final PASS, 70% evaluator score, 90% action-level score.
- Bonus scope: Vector +4 and Multi-agent +3 only.

## 6. Forbidden Claims

Do not state or imply:

- the benchmark always achieves 10/10;
- `std::inplace_vector` is implemented;
- GUI or VLM bonus is complete;
- `HashEmbedder` is used in production;
- all warnings or memory issues are proven absent;
- provider fallback is automatic unless current code and tests demonstrate it;
- a feature is complete merely because a class or function exists.

## 7. Content Limits

- Do not copy the full script onto slides.
- Do not exceed six short body lines on one slide.
- Do not show large code blocks or full terminal logs.
- Do not add new features, requirements, benchmark results, or bonus claims.
- Preserve the 15-minute plan: approximately 11–12 minutes speech, 2 minutes demo, and 1 minute transitions.

## 8. Diagram Contracts

Every technical flow must make these elements visible where relevant:

- entry point;
- modules or actors;
- input and output;
- state ownership;
- error path;
- cleanup or termination;
- observable evidence.

Use exact production paths. A unit-test-only path must be marked `Test only`.

## 9. Speaker Notes Contract

- Keep the meaning of the approved detailed script.
- Use natural spoken Vietnamese, short sentences, and clear transitions.
- Do not read code or diagrams line by line.
- End each role section with a handoff to the next presenter.
- Keep benchmark limitations honest: results depend partly on live model behavior and service availability.

## 10. Final Review Gate

- [ ] Exactly 15 slides and correct A/B/C ownership.
- [ ] Slide content matches the approved script.
- [ ] All architecture arrows match the production path.
- [ ] All metrics come from one identified run.
- [ ] Vector and Multi-agent bonuses have code, test, integration, and evidence references.
- [ ] C++26 wording uses deleted function with reason.
- [ ] No unverified or forbidden claim appears.
- [ ] No API key, secret, private path, or generated artifact is exposed.
- [ ] A different team member can present the slide using only the visual and speaker notes.

## 11. Reusable Generation Prompt

```text
Create the 15-slide AI Agent project deck from
planning/PowerPoint/Presentation_Script_Detail.md.

Follow planning/PowerPoint/design.system.md and this file exactly.
Preserve slide order and Role A/B/C ownership. Keep slide text minimal and put
the approved explanation in speaker notes. Use diagrams for architecture and
runtime flows. Verify all technical claims and benchmark metrics before using
them. Do not add features or claims outside the approved sources. Return a
slide-by-slide draft for review before generating the final PPTX.
```
