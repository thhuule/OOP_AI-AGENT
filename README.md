# OOP AI Agent

A C++ AI Agent framework with interchangeable LLM clients, a tool-calling loop, Markdown skills, loop detection, a benchmark harness, and a multi-agent coordination demo.

## Architecture Overview

| Component | Responsibility | Location |
|---|---|---|
| LLM client | Sends conversation history to Gemini or Ollama | `src/client/` |
| Agent loop | Coordinates prompts, LLM responses, tool calls, and stopping conditions | `src/agent/` |
| Tool registry | Registers, resolves, and executes tools | `src/tools/` |
| Skill system | Loads Markdown instructions into the system prompt | `src/skills/` |
| Evaluation harness | Loads tasks, runs the agent, evaluates results, and exports trajectories | `src/harness/`, `benchmark/` |
| Multi-agent | Provides worker threads, a dispatcher, and message queues | `src/multiagent/` |

The main layers communicate through the `LLMClient`, `Tool`, and `Evaluator` abstractions. `AgentLoop` does not depend on `HarnessRunner`; the harness records trajectories through `StepHook`.

## Prerequisites

The supported build environment is WSL/Linux. For example, on Ubuntu:

```bash
sudo apt update
sudo apt install cmake g++ libcurl4-openssl-dev nlohmann-json3-dev libsqlite3-dev
```

The project requires CMake 3.28 or newer. GNU and Clang builds use `-std=c++26`, so the compiler must support the C++ features used by the source code.

## Build

Run the following commands from the repository root inside WSL:

```bash
cmake -S . -B build
cmake --build build -j2
```

The build produces five executables in `build/`:

- `OopAgent`: performs one Gemini smoke-test request. It is not an interactive chat mode and does not support `--chat`.
- `run_eval`: runs the 10-task batch from `benchmark/tasks.json` and exports results.
- `test_harness`: runs local focused tests for task validation, evaluator Strategy selection, StepHook recording, cleanup, failure taxonomy, and score aggregation; it does not require an LLM API.
- `test_multi_agent`: runs local tests for the dispatcher, message queue, and clean shutdown; it does not require an LLM API.
- `demo_multi_agent`: runs calculator and search workers, then creates `report.txt`. Its web-search path may use the network.

## Configuration

Create a local configuration file:

```bash
cp config.json.example config.json
```

Gemini example:

```json
{
  "provider": "gemini",
  "api_key": "YOUR_API_HERE",
  "model": "gemma-4-31b-it",
  "api_url": "https://generativelanguage.googleapis.com/v1beta",
  "use_mock": false
}
```

Ollama example:

```json
{
  "provider": "ollama",
  "api_key": "",
  "model": "gemma4:e4b",
  "api_url": "http://localhost:11434",
  "use_mock": false
}
```

Current implementation notes:

- `run_eval` reads `provider`, `api_key`, `model`, and `api_url`.
- `OopAgent` always creates a `GeminiClient` and reads only `api_key` and `model`.
- The example configuration contains `use_mock`, but the executables do not currently connect this field to a mock execution path. Do not use it as evidence that a run used a mock or a real provider.
- `LLMConfig` defines default temperature and timeout values in source code, but `run_eval` does not currently read them from `config.json`.

Never commit `config.json`. It may contain an API key and is listed in `.gitignore`.

## Tests and Demo

Run the local tests first:

```bash
./build/test_harness
./build/test_multi_agent
```

Alternatively, run both through CTest:

```bash
ctest --test-dir build --output-on-failure
```

A successful direct run ends with these messages:

```text
ALL HARNESS TESTS PASSED
ALL PASSED
```

Run the multi-agent demo only when network access to DuckDuckGo is acceptable:

```bash
./build/demo_multi_agent
```

The demo creates `report.txt` in the working directory. It is an independent extension and is not evidence that the benchmark harness coordinates sub-agents.

## Running the Benchmark

`run_eval` calls the real provider selected by the current configuration. It may consume quota, use the network, and create files. Before running it:

1. Inspect `config.json` without printing or sharing the API key.
2. Confirm that the selected model, quota, and cost are approved.
3. Build every target, then run `test_harness` and `test_multi_agent`.
4. Make sure the working tree does not contain old artifacts that must be preserved.

Then run:

```bash
./build/run_eval
```

The harness reads `benchmark/tasks.json`, removes known benchmark artifacts before the batch, runs 10 tasks sequentially, and creates:

```text
benchmark/results/run_YYYYMMDD_HHMMSS_mmm/
├── eval_results.json
├── benchmark_summary.txt
├── trajectory_task_001.json
└── ...
```

`eval_results.json` contains the evaluator score, action-level score, and final success rate. Each trajectory contains the thought, action, tool result, and latency for its recorded tool steps.

`tokens_used` is currently `0` because the clients do not pass provider token metadata to the harness. This value means **not measured**; it does not mean that the model used no tokens.

## Benchmark Criteria

`benchmark/tasks.json` is the source of truth. The current suite contains:

- 4 simple tasks;
- 4 medium tasks;
- 2 hard tasks.

A task that requires tools reaches final success only when its evaluator passes and the harness finds at least one relevant successful tool step. File-producing tasks are also checked by `FunctionalEvaluator` scripts for the required filename and content.

Files under `benchmark/results/` are historical evidence. A claim about the current revision must come from a new, clean run using the stated provider.

## Security

- Do not commit API keys, `config.json`, databases, build output, or generated benchmark artifacts.
- Do not weaken tasks or evaluators merely to improve the score.
- Keep the `execute_shell` policy restrictive; do not use the benchmark to run commands outside task scope.
- `run_eval` and `demo_multi_agent` may use the network. Use the local tests first.

## Troubleshooting

### CMake Cannot Find a Dependency

Install CURL, SQLite3, and nlohmann-json, then configure again:

```bash
sudo apt install libcurl4-openssl-dev libsqlite3-dev nlohmann-json3-dev
cmake -S . -B build
```

### `config.json` Is Missing

Run the executable from the repository root or `build/` directory, and make sure `config.json.example` has been copied to `config.json`.

### Gemini Returns 429 or Resource Exhausted

Stop the benchmark, inspect the quota or rate limit, and do not retry continuously. A rate-limited run must not be reported as evidence of agent quality.

### Ollama Cannot Connect

Verify that Ollama is running, `api_url` is correct, and the selected model is available locally.

### The Benchmark Passes Because of an Old File

Do not use files in the repository root as evidence. Check the cleanup log and the timestamped run directory. If cleanup fails, the harness stops the batch to prevent a false positive.

## Documentation

- [Evaluation report](docs/report_evaluation.md)
- [Batch evaluation sequence diagram](docs/sequence_harness.md)
- [Week 12 submission checklist](docs/submission_checklist.md)
- [Demo video storyboard](docs/video_storyboard.md)
