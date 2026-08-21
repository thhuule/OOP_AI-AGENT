# AI Agent Project — PowerPoint Design System

## 1. Purpose

Create a clear 18-slide technical presentation for an 8–10 minute hybrid video. Slides explain the system; three short demo clips prove Role A, B, and C behavior; Slide 18 closes with a minimal Thank You. Primary source: `planning/PowerPoint/Presentation_Script_Detail.md`.

## 2. Visual Direction

- 16:9 widescreen; modern, academic, evidence-led.
- Dark navy background; light text; one key message per slide.
- Use diagrams for architecture, sequence, ownership, and evidence flow.
- Keep Q&A outside the deck.

## 3. Design Tokens

| Token | Value | Use |
|---|---|---|
| Background | `#0B1220` | Main background |
| Surface | `#162033` | Cards and nodes |
| Primary | `#35C2FF` | Core architecture and flow |
| Secondary | `#33D6A6` | PASS and production path |
| Bonus | `#F5B942` | Vector and Multi-agent |
| Error | `#FF6B6B` | Failure and limitations |
| Main text | `#F4F7FB` | Titles/body |
| Muted text | `#A9B5C7` | Captions/footer |

## 4. Typography and Layout

- Aptos/Segoe UI/Arial; Consolas/Cascadia Mono for code.
- Title 30–36 pt; body 18–24 pt; caption 14–16 pt.
- Safe margin at least 5%; maximum six short body lines.
- Use 2–4 cards or one dominant diagram; keep title/footer positions consistent.

## 5. Core Patterns

| Pattern | Slides |
|---|---|
| Hero | 1, 17, 18 |
| Roadmap/glossary | 2, 3 |
| Process/architecture | 4–7, 10, 12–15 |
| Cards/timeline/inventory | 8, 9, 11 |
| Evidence dashboard | 16, 17 |

## 6. Diagram Rules

- Draw connectors before nodes; label direction, input/output, error, and cleanup where relevant.
- Use one color per responsibility, not per class.
- Keep exact class/function names when they are evidence.
- Show production paths in Primary/Secondary; bonus paths in Bonus; test-only paths must say `Test only`.

## 7. Motion and Transitions

- Fade within a section.
- Push only at Slide 4, 8, and 13 to mark section changes.
- Reveal complex flows in 2–4 stages; no decorative spinning or bouncing.
- In the final video, cross-dissolve from Slide 7/12/16 to the matching demo clip, then return to the next section.

## 8. Slide Map

| Slides | Owner | Visual purpose |
|---|---|---|
| 1–3 | A | Identity, roadmap, definitions |
| 4–7 | A | ReAct, architecture, AgentLoop, reliability |
| 8–12 | B | OOP/C++, registry, tools, Vector |
| 13–18 | C | Harness, trajectory, Multi-agent, benchmark, conclusion, Thank You |

## 9. Evidence Guardrails

- CTest: 5/5 PASS.
- Model/run: `gemma-4-31b-it`, `run_20260820_002933_100`.
- Benchmark: 7/10 final PASS, 70% evaluator, 90% action-level.
- Vector: +4; production uses Ollama `nomic-embed-text`; `HashEmbedder` is test-only.
- Multi-agent: +3; two worker threads communicate through a queue.
- C++26: deleted function with reason, not `std::inplace_vector`.
- Do not claim GUI/VLM, benchmark 10/10, zero warnings/leaks, or automatic provider fallback without evidence.
- Never expose API keys, `config.json`, private paths, or personal data.

## 10. Final Visual QA

- [ ] All 18 slides are readable and have one takeaway.
- [ ] Logo, lecturer, names, and MSSVs are filled before recording.
- [ ] Diagrams match the production path and metrics cite one run.
- [ ] Role ownership is A 1–7, B 8–12, C 13–18.
- [ ] Section transitions and three demo handoffs are present.
- [ ] Total hybrid video remains within 8–10 minutes.
