# Demo Video Storyboard

Goal: demonstrate the project requirements with a short, reproducible video that does not expose secrets. Upload the video to YouTube as **Unlisted**.

## Suggested Duration: 8–10 Minutes

| Time | Visuals | Main Talking Points | Evidence |
|---|---|---|---|
| 0:00–0:30 | Title, team members, and goal | C++ AI Agent, ReAct, tools, skills, and evaluation | Opening slide |
| 0:30–1:30 | Component and class diagrams | The three A/B/C layers and their main abstractions | Mermaid diagrams |
| 1:30–2:15 | README and WSL terminal | Dependencies and build process | CMake commands |
| 2:15–3:15 | `AgentLoop` source and one tool call | The LLM returns an action, the registry finds the tool, and the loop receives the observation | Agent-run sequence diagram |
| 3:15–4:00 | Tool inventory | File, exec, web, memory, calculator, and additional tools | Tools report |
| 4:00–5:00 | `benchmark/tasks.json` and harness sequence | 10 tasks, evaluator Strategy, `StepHook`, and artifact cleanup | Batch-evaluation sequence diagram |
| 5:00–6:15 | Run `test_harness` and `test_multi_agent`, then open the test source | Harness validation and trajectories pass; the message bus delivers the correct result and shuts down cleanly | Both tests pass |
| 6:15–7:30 | Prepared confirmation benchmark run | Total score, categories, and trajectories for tasks 005 and 010 | New run directory |
| 7:30–8:20 | Compare the 2/10 and 10/10 runs | Explain the artifact-parsing failure and how the evidence changed | Evaluation report |
| 8:20–9:00 | Limitations and backlog | Tokens are not measured, VLM is a skeleton, and the harness does not integrate sub-agents | Limitations section |
| 9:00–9:30 | Conclusion | Summarize OOP design, reproducibility, and verified results | Final checklist |

## Terminal Script

Prepare before recording; do not show lengthy package installation in the video:

```bash
cmake -S . -B build
cmake --build build -j2
./build/test_harness
./build/test_multi_agent
```

Record a real `run_eval` execution only if:

- the team has approved the quota or cost;
- `config.json` has been checked but will not be displayed;
- old artifacts do not need to be preserved;
- there is enough time to wait for all 10 tasks.

If the real benchmark takes too long, present the directory from a clean confirmation run completed immediately before recording. State its run ID and timestamp clearly; do not fabricate results or present a historical run as a new one.

## Two Trajectories to Show

### Task 005

Show these three actions:

1. `calculator` with `47 * 23` → `1081`;
2. `write_file` with `result.txt,1081`;
3. `read_file` to verify the artifact.

### Task 010

Show the recovery flow:

1. `read_file(data.txt)` returns `ToolError: NotFound`;
2. create the file with `initial data`;
3. append the line `appended`;
4. read the final content again.

## Do Not Show or Claim

- Do not display `config.json`, API keys, tokens, or credentials.
- Do not say `tokens_used = 0` means the model used no tokens; say that token usage is not measured.
- Do not describe `VLMEvaluator` as a complete image evaluator.
- Do not describe the `MultiAgentRunner` demo as sub-agent integration in the harness.
- Do not mention `OopAgent --chat`; the executable is currently only a Gemini smoke test.
- Do not use the action-level score alone as proof that a task is complete.
- Do not reintroduce a Week 13 live demo as a mandatory requirement.

## Pre-Upload Checklist

- [ ] Terminal text is large enough and contains no sensitive personal information.
- [ ] Audio is clear, and class and file names are pronounced correctly.
- [ ] The run ID and model shown in the benchmark section match the artifacts.
- [ ] The video contains no long waits or unexplained errors.
- [ ] The link is Unlisted and opens while signed out.
- [ ] The final link is added to the report or submission location required by the instructor.
