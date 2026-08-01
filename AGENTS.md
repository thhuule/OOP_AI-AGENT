# AGENTS.md

## Project overview

This repository is a C++ AI-agent project with three main ownership areas:

- Role A — systems/core: LLM clients, `AgentLoop`, parsing, skills, and loop detection.
- Role B — tools/data: tool interfaces, registry, aliases, argument parsing, and tool implementations.
- Role C — evaluation/infra: harness, evaluators, benchmark tasks/results, trajectories, and multi-agent demos.

The benchmark source of truth is `benchmark/tasks.json`. Historical benchmark evidence and team plans live under `filephanchiacv/`; in particular, consult the newest `PHAN_TICH_RUN_*.md` before changing behavior for a failed run.

## Build and verification

Use WSL/Linux for the supported build because the project depends on CURL, SQLite3, Threads, and nlohmann_json:

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_multi_agent
./build/run_eval
```

`run_eval` can call a real LLM provider and writes benchmark artifacts. Inspect `config.json` without exposing its API key, and obtain user confirmation before a real network-backed benchmark when cost, quota, or generated artifacts matter.

For changes to one component, build all targets and run the smallest relevant executable first. A claimed benchmark success must come from a clean, current run rather than old files in the repository root.

## Architecture constraints

- Preserve the abstract `LLMClient`, `Tool`, and `Evaluator` interfaces.
- Keep tool registration in `ToolRegistry`; do not hardcode tool implementations into `AgentLoop`.
- Keep `AgentLoop` independent of `HarnessRunner`, tools independent of the agent loop, and evaluators independent of agent internals.
- Preserve the intended patterns: Strategy (`Evaluator`), Registry/Factory (`ToolRegistry`/`Registry<T>`), Observer/Hook (`StepHook`), and the agent-loop orchestration flow.
- Use RAII and smart pointers; do not introduce raw owning `new`/`delete`.
- Maintain the guarded C++26 feature path and its portability fallback. GNU/Clang builds are explicitly compiled with `-std=c++26`; MSVC uses `/std:c++latest` where configured.

## Benchmark correctness

- Do not weaken task post-conditions or evaluators merely to raise the score.
- Tasks that require tools must contain a real, successful tool step.
- File-producing tasks must verify the exact filename and content created during the current run.
- Clean or isolate known task artifacts before evaluation so stale `notes.txt`, `result.txt`, `capital.txt`, `calc.txt`, `data.txt`, `output.txt`, or similarly malformed files cannot cause false passes.
- Preserve actual tool arguments in trajectories; do not report an empty `args` field when arguments were supplied.
- Prefer specific failure reasons such as invalid arguments, missing artifacts, content mismatch, loop detection, or incomplete task.

## Editing conventions

- Read the relevant role plan and latest run analysis before editing role-owned code.
- Keep changes scoped to the requested role/task and preserve unrelated user modifications in a dirty worktree.
- Update or add focused tests for parsers, tool arguments, loop detection, and evaluator behavior when those areas change.
- Never commit `config.json`, API keys, generated benchmark runs, build output, databases, or task artifacts unless the user explicitly requests it.
- Do not edit vendored headers under `include/nlohmann/`.

## Security

- Treat `config.json` as secret-bearing. Never print, log, or commit its API key.
- Keep shell execution policy restrictive and return explicit `ToolError` values for rejected or failed operations.
- Do not make benchmark success depend on mocks when reporting a real provider success rate.
