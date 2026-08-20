# Week 10.75 Verification Evidence — 2026-08-20

This file records the commands and observed results for the current working-tree candidate. Raw benchmark exports are intentionally ignored by Git under `benchmark/results/`; the local export referenced below is the reproducible detailed artifact.

## Required regression

```text
ctest --test-dir build --output-on-failure
Result: 5/5 tests passed, 0 failed.

./build/test_role_a
Result: PASS. Parser variants, escaped arguments, six malformed tool-call cases,
provider configuration, multimodal serialization, loop detection, and native
environment tests passed. The same target also verifies task-keyword skill
selection, dynamic registry catalog, `system/user/assistant/tool` history order,
Gemini/Ollama usage parsing, final-answer trajectory recording, and the active
C++26 deleted-function-reason build.

./build/test_tools
Result: PASS. Registry/file aliases, error paths, WebSearch malformed-response
handling, exact `memory_save`/`memory_search` entries, primary vector ranking,
explicit legacy keyword mode, and tool-safety tests passed.

./build/test_multi_agent
Result: PASS. Worker lifecycle, queue dispatch, stop/join, and calculator/
researcher workflow tests passed.

./build/test_harness
Result: PASS. A final-answer trajectory step does not increase tool_steps_count;
per-step/total token metadata and the exported JSON schema are verified.
```

## Vector bonus

```text
RUN_LIVE_OLLAMA=1 ./build/test_tools
Result: PASS, including test_live_ollama_vector_acceptance.
Embedder: Ollama nomic-embed-text.
```

## Real-provider benchmark

```text
Command: ./build/run_eval
Provider/model: Gemini gemma-4-31b-it
Export: benchmark/results/run_20260820_002933_100/
Suite: 10 tasks (4 simple, 4 medium, 2 hard)
Evaluator score: 0.7
Action-level score: 0.9
Final success rate: 0.7 (7/10)
Trajectory action source: llm only; no fixture action recorded.
```

The 7/10 score is the selected model's measured result, as required by §7.3. It is not a deterministic code-test score. The recorded misses were model choices: task 004 selected `execute_shell(date)` instead of `time`; tasks 005 and 009 repeated a successful calculator call until loop detection.

## Freeze hand-off

- [x] Current working tree builds and passes the regression commands above.
- [x] Current real-provider benchmark result is recorded without fallback.
- [x] The final temporary package contained 170 intended candidate files, including the two new evidence/summary files, and passed `scripts/check_repo_hygiene.sh --package`.
- [x] The same temporary copy completed a clean CMake configure/build and CTest `5/5 PASS`; it was removed after verification.
- [ ] Commit/push the current working-tree candidate after review. The evidence above describes working-tree contents, not the older `HEAD` commit.
- [ ] Role A and Role B independently inspect one production trajectory and their assigned post-audit contract before the team declares code freeze.
- [ ] Team verifies the specification's Git contribution, per-member commit count, and commit-spacing rules. The previous audit reported a high-risk imbalance; do not rewrite history or fake ownership to change the result.

The exact requirement-to-code/test/docs matrix and normalized Git measurements are recorded in [`requirement_traceability_final_2026-08-20.md`](requirement_traceability_final_2026-08-20.md).
