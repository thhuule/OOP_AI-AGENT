# Week 10.75 Verification Evidence — 2026-08-20

This file records the commands and observed results for the current working-tree candidate. Raw benchmark exports are intentionally ignored by Git under `benchmark/results/`; the local export referenced below is the reproducible detailed artifact.

## Required regression

```text
ctest --test-dir build --output-on-failure
Result: 5/5 tests passed, 0 failed.

./build/test_role_a
Result: PASS. Parser variants, escaped arguments, six malformed tool-call cases,
provider configuration, multimodal serialization, loop detection, and native
environment tests passed.

./build/test_tools
Result: PASS. Registry/file aliases, error paths, WebSearch malformed-response
handling, vector ranking, and tool-safety tests passed.

./build/test_multi_agent
Result: PASS. Worker lifecycle, queue dispatch, stop/join, and calculator/
researcher workflow tests passed.
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
- [ ] Commit the current working-tree changes before clean extraction: an archive of the pre-change `HEAD` still fails to compile because it lacks the current AgentLoop fix.
- [ ] After committing, run the clean-extraction build/CTest gate and have Role A and Role B independently inspect one production trajectory each before declaring code freeze.
