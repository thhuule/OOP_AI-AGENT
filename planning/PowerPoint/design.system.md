# AI Agent Project — PowerPoint Design System

## 1. Purpose

Create a clear 15-slide technical presentation for lecturers and classmates. The deck must communicate the architecture, OOP/C++ implementation, tools, Vector bonus, Multi-agent bonus, benchmark evidence, and conclusion within 15 minutes.

Primary content source: `planning/PowerPoint/Presentation_Script_Detail.md`.

## 2. Visual Direction

- Style: modern technical, clean, evidence-led.
- Canvas: 16:9 widescreen.
- Background: dark navy or charcoal; use a light background only for dense diagrams or code.
- Visual hierarchy: one key message per slide, one dominant visual, minimal supporting text.
- Tone: credible and academic, not promotional.

## 3. Design Tokens

| Token | Value | Use |
|---|---|---|
| Background | `#0B1220` | Main slide background |
| Surface | `#162033` | Cards and diagram nodes |
| Primary | `#35C2FF` | Architecture, flow, key terms |
| Secondary | `#33D6A6` | PASS, completed, production path |
| Bonus | `#F5B942` | Vector and Multi-agent bonus |
| Error | `#FF6B6B` | Failure paths and limitations |
| Main text | `#F4F7FB` | Titles and body text |
| Muted text | `#A9B5C7` | Captions and secondary labels |

Use the project colors consistently. Do not introduce extra accent colors without a functional reason.

## 4. Typography

- Font family: Aptos, Segoe UI, or Arial fallback.
- Slide title: 30–36 pt, semibold.
- Section label: 18–22 pt, semibold.
- Body: 18–24 pt.
- Caption/evidence label: 14–16 pt.
- Code: Consolas or Cascadia Mono, minimum 16 pt.
- Maximum: 6 short lines of body text per slide.

## 5. Layout and Spacing

- Safe margin: at least 5% of slide width on all sides.
- Use a 12-column grid or simple 60/40 split.
- Keep at least 24 px between related blocks and 40 px between separate sections.
- Prefer 2–4 cards per slide; avoid dense card walls.
- Place slide number and role owner in a small footer.
- Keep titles in the same position across all slides.

## 6. Core Slide Patterns

| Pattern | Use |
|---|---|
| Hero | Slides 1 and 15: one message, minimal text |
| Process flow | Slides 2, 3, 4, and 5 |
| Pattern cards | Slides 6 and 7 |
| Inventory/grid | Slide 9 |
| Flow diagram | Slides 8, 10, 11, 12, and 13 |
| Evidence dashboard | Slides 14 and 15 |

## 7. Diagram Rules

- Show direction with arrows and label inputs/outputs.
- Use one color per layer or responsibility, not per class.
- Convert detailed UML into presentation-friendly summaries; keep full UML in the report.
- Keep class/function names exact when they are evidence.
- Highlight production path with Primary/Secondary colors and optional or bonus paths with Bonus color.

## 8. Code, Screenshots, and Evidence

- Use code only when a language feature or contract cannot be explained more clearly as a diagram.
- Crop terminal screenshots to the relevant command and result.
- Every metric must show its evidence source in a small caption.
- Benchmark slide must show the selected model and the same run's metrics together.
- Never display API keys, `config.json` secrets, local user paths, or private data.

## 9. Motion

- Use only Fade or Appear.
- Reveal complex flows in 2–4 logical stages.
- No decorative spinning, bouncing, or continuous animation.
- Transitions must not consume demo time.

## 10. Slide-by-Slide Visual Map

| Slide | Owner | Recommended visual |
|---|---|---|
| 1 | A | Project title with Agent → Tool → Result motif |
| 2 | A | ReAct loop: Observe → Think → Act → Observe |
| 3 | A | Four-layer architecture diagram |
| 4 | A | AgentLoop, LLM client, parser, and typed error contract |
| 5 | A | Loop Detector and keyword-based skill selection |
| 6 | B | Four OOP pattern cards |
| 7 | B | C++17/20/23/26 feature timeline |
| 8 | B | Tool Registry, catalog, alias, and policy boundary |
| 9 | B | Required and supplemental tool inventory |
| 10 | B | Vector embedding → cosine search flow |
| 11 | C | Harness execution and evaluation pipeline |
| 12 | C | Evaluator and trajectory evidence |
| 13 | C | Multi-agent queue with two worker threads |
| 14 | C | Benchmark dashboard: 7/10, 70%, 90% |
| 15 | C | Final requirement coverage, test evidence, and conclusion |

## 11. Fact and Claim Guardrails

The deck must preserve these verified facts:

- CTest: 5/5 PASS.
- Model: `gemma-4-31b-it`.
- Benchmark evidence: `run_20260820_002933_100`.
- Benchmark result: 7/10 final PASS, 70% evaluator score, 90% action-level score.
- Vector bonus: +4 points; production embedding uses Ollama `nomic-embed-text`.
- Multi-agent bonus: +3 points; real flow uses two worker threads and a queue.
- C++26 evidence: deleted function with reason, such as `= delete("reason")`.

Do not claim:

- 10/10 benchmark success unless a newer evidenced run is approved.
- `std::inplace_vector` is used.
- GUI or VLM bonus is complete.
- Zero warnings, zero leaks, or automatic provider fallback without matching evidence.
- `HashEmbedder` is the production embedder; it is for offline tests.

## 12. Final Visual QA

- [ ] All text is readable from presentation distance.
- [ ] Each slide has one clear takeaway.
- [ ] Diagrams match the real production path.
- [ ] Metrics and technical claims have evidence.
- [ ] No secret or local-only data is visible.
- [ ] Role A/B/C ownership and 15-slide order are preserved.
- [ ] Speaker notes fit the assigned presentation time.
