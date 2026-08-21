# OOP AI Agent

A Modern C++ AI Agent framework with Gemini and Ollama clients, a ReAct tool loop, Markdown skills, loop detection, an evaluation harness, vector memory, and a two-worker coordination demo.

This README is the main requirement trace and presentation demo guide. Detailed reports remain under [`docs/`](docs/).

## What Is Implemented

| Layer | Implementation | Evidence |
| --- | --- | --- |
| LLM clients | Common `LLMClient` interface with Gemini and Ollama implementations and provider token metadata | [`src/client/`](src/client/), `test_role_a` |
| Agent core | ReAct-style `AgentLoop`, tool-call parsing, conversation history, `StepHook`, and safe stopping | [`src/agent/`](src/agent/), `test_role_a`, `test_template_method` |
| Skills and loop safety | Markdown skill selection with `task_planner` fallback; repeated-action and ping-pong detection | [`skills/`](skills/), `test_role_a` |
| Tools | `Tool` contract plus Registry/Factory, aliases, runtime catalog, and allow/deny policy | [`src/tools/`](src/tools/), `test_tools` |
| Harness | Task loading, setup/cleanup, evaluator strategies, scoring, failure reasons, and JSON trajectory export | [`src/harness/`](src/harness/), [`benchmark/`](benchmark/), `test_harness` |
| Environment | Abstract artifact operations with native-filesystem and injectable in-memory implementations | [`src/environment/`](src/environment/), `test_harness` |
| Vector memory | SQLite persistence, Ollama embeddings, and C++ cosine-similarity ranking | [`MemoryTool.cpp`](src/tools/MemoryTool.cpp), [`Embedding.cpp`](src/tools/Embedding.cpp), `test_tools` |
| Multi-agent | Two worker threads coordinated through message queues, with timeout and failure propagation | [`src/multiagent/`](src/multiagent/), `test_multi_agent`, `demo_multi_agent` |

`AgentLoop` does not depend on `HarnessRunner`. The harness observes steps through `StepHook`, and uses `NativeEnvironment` by default while tests can inject `SandboxEnvironment`.

## Requirement Traceability

| Requirement | Implementation and check | Status |
| --- | --- | --- |
| LLM client, Skill System, ReAct loop, loop detection, Harness/Evaluator, trajectory | Production components above; focused Role A and harness tests | Implemented |
| Required tools: shell, file, web search, persistent memory, calculator | `execute_shell`; `file` plus read/write/append tools; `web_search`; SQLite `memory`; `calculator`; `test_tools` | Implemented |
| At least three extra tools | `time`, `json`, and `git` | Implemented |
| Four design patterns | Strategy (`Tool`, `Evaluator`), Template Method (`AgentLoop::run`), Registry/Factory (`ToolRegistry`), Observer (`StepHook`) | Implemented and tested |
| Modern C++ matrix | C++17 smart pointers/function/variant/filesystem/optional/templates; C++20 ranges/views; C++23 expected/print; C++26 deleted functions with a reason | Checked by `test_role_a`; GNU/Clang targets compile with `-std=c++26` |
| Benchmark | 10 tasks: 4 simple, 4 medium, 2 hard; evaluator, action, final-success, token, and trajectory output | Implemented; verified Gemini result is 7/10 |
| UML and documentation | Four Mermaid component/class/sequence diagrams and detailed design/evaluation reports | Available under [`docs/diagrams/`](docs/diagrams/) and [`docs/reports/`](docs/reports/) |
| Vector-search bonus | Ollama `nomic-embed-text`, SQLite vectors, cosine ranking; deterministic embedder only in offline tests | Technically implemented; independent review remains |
| Multi-agent bonus | Harness-to-runner path, two threads, queues, report, and explicit failure handling | Technically implemented; independent review remains |
| GUI bonus | Screenshot and bounded-action contracts exist, but no complete cross-platform GUI agent is implemented | Not claimed |
| Git contribution gate | Commit count passes; source-line balance and maximum commit spacing do not | Unresolved non-code requirement; instructor decision needed |

The full audit, including the Git evidence, is in [`requirement_traceability_final_2026-08-20.md`](docs/evidence/requirement_traceability_final_2026-08-20.md).

## Tool Catalog

`ToolRegistry::register_all_tools()` currently exposes these canonical names:

| Group | Canonical tools | Notes |
| --- | --- | --- |
| Required | `calculator`, `execute_shell`, `file`, `read_file`, `write_file`, `append_file`, `web_search`, `memory`, `memory_save`, `memory_search` | `memory_save` and `memory_search` are explicit adapters over persistent memory |
| Additional | `time`, `json`, `git` | Satisfy the three-extra-tool requirement |
| GUI contracts | `capture_screenshot`, `gui_action` | Screenshot uses macOS `screencapture`; action validation exists, but real desktop action execution is intentionally absent |

Aliases are normalized before policy checks and lookup:

| Alias | Canonical name |
| --- | --- |
| `calculate` | `calculator` |
| `exec` | `execute_shell` |
| `google_search` | `web_search` |
| `create_file` | `write_file` |
| `file_read` | `read_file` |
| `file_write` | `write_file` |
| `screenshot` | `capture_screenshot` |

The deny list always blocks a canonical tool. When the allow list is non-empty, only explicitly allowed canonical tools are visible through lookup, creation, and the dynamic catalog supplied to the agent.

## Prerequisites and Build

The verified build environment is WSL/Linux. On Ubuntu:

```bash
sudo apt update
sudo apt install cmake g++ libcurl4-openssl-dev libsqlite3-dev
```

CMake 3.28 or newer is required. The repository vendors nlohmann/json, so the system package is optional. The compiler must support the used C++23/C++26 features.

From the repository root:

```bash
cmake -S . -B build
cmake --build build -j2
```

The build defines nine executables: `OopAgent`, `run_eval`, five focused test executables, `demo_multi_agent`, and the manual `test_websearch_cli` diagnostic.

## Configuration

Create the ignored local configuration:

```bash
cp config.json.example config.json
```

The complete example schema is:

```json
{
  "provider": "gemini",
  "api_key": "YOUR_API_HERE",
  "model": "gemma-4-31b-it",
  "api_url": "https://generativelanguage.googleapis.com/v1beta",
  "temperature": 0.7,
  "max_tokens": 2048,
  "timeout_seconds": 60,
  "use_mock": false,
  "ollama_host": "http://localhost:11434",
  "embedding_model": "nomic-embed-text"
}
```

- `OopAgent` is only a one-request Gemini smoke test. It always creates `GeminiClient`, reads only `api_key` and `model`, and is not an interactive `--chat` program.
- `run_eval` selects Gemini when `provider` is `gemini`; every other value currently selects Ollama. It reads the common `model`, `api_url`, `temperature`, `max_tokens`, and `timeout_seconds` fields, plus `api_key` for Gemini.
- Vector memory in `run_eval` is configured separately by `ollama_host` and `embedding_model`, even when Gemini is the chat provider. Ollama must therefore be available for benchmark tasks that use memory.
- `use_mock` exists in the example but is not read by either executable. It is not proof of a mock or live run.

Never commit `config.json`; it may contain a secret.

Run the Gemini smoke test only after configuring a valid Gemini key/model:

```bash
./build/OopAgent
```

## Offline Verification

Run all five CTest targets:

```bash
ctest --test-dir build --output-on-failure
```

Run each focused executable directly when presenting its output:

```bash
./build/test_harness
./build/test_multi_agent
./build/test_tools
./build/test_template_method
./build/test_role_a
```

These focused tests use fixtures or injected fakes for provider-sensitive behavior. The default vector checks use `HashEmbedder` only to stay deterministic; production does not silently fall back to it.

`test_websearch_cli` is a manual network diagnostic, not a CTest target:

```bash
./build/test_websearch_cli 'capital of France'
```

## Optional Live Ollama Vector Acceptance

Install/start Ollama and pull the embedding model, then confirm the service before opting into the live check:

```bash
ollama pull nomic-embed-text
curl http://localhost:11434/api/tags
RUN_LIVE_OLLAMA=1 ./build/test_tools
```

Without `RUN_LIVE_OLLAMA=1`, `test_tools` skips the live acceptance. A saved successful check is documented in [`week10_75_verification_2026-08-20.md`](docs/evidence/week10_75_verification_2026-08-20.md).

## Multi-Agent Demo

The default demo computes `47 * 23` and searches for Japan's capital. The research worker uses DuckDuckGo, so this demo needs network access.

```bash
./build/demo_multi_agent
cat artifacts/demo/report.txt
```

Optional custom inputs:

```bash
./build/demo_multi_agent '2 * 3' 'France capital'
cat artifacts/demo/report.txt
```

The report says `STATUS=PASS` only when both workers succeed. A worker error or timeout remains a failure; no answer is fabricated.

## Benchmark Run and Evidence

`run_eval` calls the configured real provider, may consume quota, and writes generated files. Check the provider, model, network, Ollama embedding service, quota, and cost before running:

```bash
./build/run_eval
```

Each run creates `benchmark/results/run_YYYYMMDD_HHMMSS_mmm/` containing:

```text
benchmark_summary.txt
eval_results.json
trajectory_task_001.json
...
trajectory_task_010.json
```

Inspect the newest run using its printed run ID:

```bash
cat benchmark/results/run_YYYYMMDD_HHMMSS_mmm/benchmark_summary.txt
cat benchmark/results/run_YYYYMMDD_HHMMSS_mmm/eval_results.json
cat benchmark/results/run_YYYYMMDD_HHMMSS_mmm/trajectory_task_005.json
cat benchmark/results/run_YYYYMMDD_HHMMSS_mmm/trajectory_task_010.json
```

The harness removes known task artifacts before the batch, runs the 10 tasks sequentially, evaluates their output and real file postconditions, and records tool steps plus the final answer. `tool_steps_count` counts tool calls only. Provider token metadata is recorded when available; zero means “not reported,” not “no tokens used.” Production `run_eval` disables deterministic task fallback.

### Verified Benchmark Result

The local Gemini evidence run is `run_20260820_002933_100`, using `gemma-4-31b-it`:

| Metric | Result |
| --- | ---: |
| Final success | **7/10 (0.7)** |
| Evaluator score | **0.7** |
| Action-level score | **0.9** |

This is not a 10/10 claim. Tasks 004, 005, and 009 failed. Task 005's calculator action succeeded but repeated until loop detection stopped it; Task 010 completed its file workflow. Inspect the saved evidence locally:

```bash
cat benchmark/results/run_20260820_002933_100/benchmark_summary.txt
cat benchmark/results/run_20260820_002933_100/eval_results.json
cat benchmark/results/run_20260820_002933_100/trajectory_task_005.json
cat benchmark/results/run_20260820_002933_100/trajectory_task_010.json
```

Timestamped run directories are ignored local evidence, not tracked submission files. Only a reviewed baseline may be copied to [`benchmark/results/latest/`](benchmark/results/latest/).

## Presentation Demo Map

The current 18-slide script is [`planning/PowerPoint/script + demoflow.md`](planning/PowerPoint/script%20%2B%20demoflow.md).

| Presenter | Slides | Demo/evidence |
| --- | ---: | --- |
| Role A | 5–7 | Configure/build; `AgentLoop`, `LoopDetector`, skills; `test_role_a` |
| Role B | 8–12 | Design patterns, Modern C++, Registry/Factory, tool tests, optional vector acceptance |
| Role C | 13–17 | Harness/trajectories, CTest, multi-agent report, verified 7/10 benchmark evidence |

Do not run `run_eval` live during the presentation unless provider access and quota are stable. Open the saved summary and Task 005/010 trajectories instead.

## Generated Files, Security, and Failure Modes

- `build/`, `config.json`, timestamped benchmark results, `artifacts/*` except its README/placeholder, root task outputs, and `memory.db` are ignored generated files.
- `benchmark/results/latest/` is the only location intended for an approved tracked baseline; it is currently a policy location, not automatic evidence.
- Never expose or commit API keys. Do not weaken tasks/evaluators or enable fixture fallback to improve a score.
- Gemini can fail because of invalid credentials, model access, quota, rate limits, or network errors. Ollama chat/vector calls can fail when the service or requested model is unavailable.
- `demo_multi_agent` and `test_websearch_cli` depend on DuckDuckGo/network availability. Run offline CTest first.
- Old task files can create false positives; the harness treats cleanup failure as a batch failure.
- The Git contribution gate is still unresolved and cannot be fixed by runtime code or documentation claims.

## Documentation

- [OOP design report](docs/reports/report_oop_design.md)
- [Tools and vector-memory report](docs/reports/report_tools.md)
- [Evaluation report](docs/reports/report_evaluation.md)
- [Final requirement traceability](docs/evidence/requirement_traceability_final_2026-08-20.md)
- [Week 10.75 verification evidence](docs/evidence/week10_75_verification_2026-08-20.md)
- [Diagrams](docs/diagrams/)
- [Submission checklist](docs/guides/submission_checklist.md)
