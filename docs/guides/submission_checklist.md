# Packaging and Submission Checklist

Current deadline: **before 21:00 on Sunday of Week 12**. The submission includes the design, source code, final report, and an Unlisted YouTube video link. There is no longer a Week 11 submission milestone or a Week 13 live demo.

## 1. Required Deliverables

- [ ] The Mermaid class diagram is rendered and matches the source.
- [ ] The agent-run sequence diagram is rendered.
- [ ] The batch-evaluation sequence diagram is rendered.
- [ ] The component diagram is rendered and has no invalid reverse dependency between layers.
- [ ] The source builds on WSL/Linux.
- [ ] The OOP report provides evidence for the design patterns and C++ techniques.
- [ ] The tools report covers canonical names, aliases, arguments, policies, dependencies, and tests.
- [ ] The evaluation report covers failed and successful runs, scoring, trajectories, and the failure taxonomy.
- [ ] The README enables a new user to build, configure, and run the correct executables.
- [ ] The Unlisted YouTube link opens in a signed-out browser window.
- [ ] Private-repository access or the required read-only access method has been checked without placing credentials in the report or ZIP.

## 2. Technical Verification

- [ ] `cmake -S . -B build` succeeds.
- [ ] `cmake --build build -j2` succeeds for every target.
- [ ] `./build/test_tools` prints `ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY`.
- [ ] `./build/test_harness` prints `ALL HARNESS TESTS PASSED`.
- [ ] `./build/test_multi_agent` prints `ALL PASSED`.
- [ ] `./build/test_template_method` prints `ALL ROLE A TESTS PASSED SUCCESSFULLY`.
- [ ] CTest passes 5/5 tests on the packaged revision.
- [ ] The final benchmark evidence comes from a new clean-state run that the team approved for quota use.
- [ ] The run contains 10 tasks: 4 simple, 4 medium, and 2 hard.
- [ ] The four simple tasks cover calculator, file write, file read, and current local time.
- [ ] A task that requires a tool contains a real and relevant tool step.
- [ ] The report/video states whether the shown actions came from the LLM or may have come from deterministic fallback.
- [ ] Each artifact has the exact required filename and content in the current run.
- [ ] The trajectories for tasks 005 and 010 preserve the actual arguments.
- [ ] Provider and model results are recorded correctly without exposing the API key.
- [ ] Gemini/Ollama token metadata is present when returned; a token count of `0` is identified as not measured.

## 3. Cross-Review

- [ ] Role A reviews the harness sequence diagram and the LLM/`AgentLoop` content in the README.
- [ ] Role B reviews tool names, aliases, arguments, and policies in the report and README.
- [x] Role C reviews benchmark figures, output paths, and run commands (2026-08-06, HEAD `08202f5`).
- [x] Every relative Markdown link in README and the three Role C documents opens the correct file.
- [ ] All four Mermaid diagrams render without errors.
- [x] `VLMEvaluator` is not described as a completed feature.
- [x] Multi-agent is described through `HarnessRunner::runMultiAgentDemo()` and is not confused with the ten-task single-agent benchmark.
- [x] A fallback-assisted 10/10 run is described as pipeline evidence, not proof of model planning quality.

## 4. Secret and Artifact Checks

- [ ] The staged files do not contain `config.json` or an API key.
- [ ] The staged files do not contain `build/`, databases, or generated task artifacts.
- [ ] The commit does not contain a new benchmark run unless the team agreed to preserve it.
- [ ] The ZIP does not contain `.git`, caches, compiler output, or secrets.
- [ ] The README uses placeholders such as `YOUR_API_HERE` only.

Run these checks before packaging:

```bash
git status --short
git diff --check
git ls-files config.json build memory.db notes.txt result.txt capital.txt calc.txt data.txt output.txt report.txt
git ls-files benchmark/results
./scripts/check_repo_hygiene.sh --package
```

Only `benchmark/results/latest/` may contain committed benchmark evidence. The hygiene gate also checks for local build output, secrets, root artifacts, and IDE/cache folders before packaging.

## 5. ZIP Name and Contents

Suggested name:

```text
MSSV1_MSSV2_MSSV3_OopAgent.zip
```

According to the plan, the `OopAgent` suffix may be omitted if the name still contains all three student IDs. At minimum, the ZIP must contain the source code, `CMakeLists.txt`, `benchmark/tasks.json`, skills, documentation, `config.json.example`, and README.

## 6. Final Confirmation

- [ ] One team member extracts the ZIP into a new directory.
- [ ] Rebuild the project by following the README exactly.
- [ ] Run `test_harness` from the extracted copy.
- [ ] Run `test_multi_agent` from the extracted copy.
- [ ] Check the video, submission link, and private-repository access.
- [ ] Submit before the deadline instead of waiting until nearly 21:00.
