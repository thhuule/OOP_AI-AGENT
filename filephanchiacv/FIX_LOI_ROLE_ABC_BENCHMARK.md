
## 0. Rang buoc bat buoc tu de bai

### Kien truc va chuc nang toi thieu

He thong phai giu dung cac tang OOP:

- `LLMClient` abstract, co `OllamaClient`; co the them `GeminiClient` nhung khong lam mat interface generic.
- `Tool` abstract va `ToolRegistry` dang ky tool dong runtime, khong hardcode truc tiep trong `AgentLoop`.
- `SkillLoader` load cac file `.md` trong `src/skills` hoac `skills/`, co selection theo keyword.
- `AgentLoop` theo ReAct: observe -> think -> act -> observe, parse tool call, giu conversation history, max_steps.
- `LoopDetector` phat hien lap generic repeat va ping-pong, co warning/critical.
- `HarnessRunner` setup -> run agent -> evaluate -> record trajectory.
- `Evaluator` abstract, co toi thieu `KeywordEvaluator` va `FunctionalEvaluator`; `VLMEvaluator` skeleton la diem mo rong hop le.
- `Trajectory` phai ghi du step: thought/action/tool_result/latency/tokens neu co.

### Tool bat buoc

Toi thieu phai co va dang ky du:

- `execute_shell` hoac `exec`
- `read_file`
- `write_file`
- `web_search`
- `memory_save` / `memory_search` hoac tool `memory` co 2 mode ro rang
- `calculator`

Co the them `time`, `json`, `git`, nhung cac tool them khong duoc thay the 5 nhom bat buoc.

### C++ features bat buoc

De bai yeu cau:

- C++17: dung toi thieu 4 feature trong bang de bai.
- C++20: dung toi thieu 2 feature.
- C++23: dung toi thieu 2 feature.
- C++26: dung toi thieu 1 feature.

Vi vay:

- Khong duoc bo sach C++26.
- Neu dung `std::inplace_vector`, phai co fallback build duoc tren compiler chua co header `<inplace_vector>`.
- CMake van nen bien dich theo `-std=c++26` voi GCC/Clang hoac `/std:c++latest` voi MSVC.
- Neu fallback sang `std::vector`, trong code/comment/report phai ghi ro: C++26 feature duoc dung khi compiler ho tro, fallback chi de portability.

### Design pattern bat buoc

Phai giu va chi ra duoc trong code:

- Strategy: `Evaluator` hierarchy.
- Template Method: skeleton `AgentLoop::run()` hoac cac hook observe/act neu co.
- Registry/Factory: `ToolRegistry` / `Registry<T>`.
- Observer/Hook: `HarnessRunner` inject `StepHook` vao `AgentLoop`.

### Nguyen tac khong duoc vi pham

- `AgentLoop` khong include hoac phu thuoc `HarnessRunner`.
- Tool implementation khong phu thuoc `AgentLoop`.
- Evaluator khong phu thuoc cach agent thuc thi noi bo; neu can xet trajectory thi nhan thong tin qua harness/result, khong include nguoc agent loop.
- Benchmark khong duoc pass bang mock khi bao cao "success rate that".
- Khong de file cu nhu `result.txt`, `calc.txt`, `data.txt` lam pass gia.

---

## 1. Tinh trang hien tai

Benchmark chay du 10 task voi Gemini/Gemma va xuat ket qua, nhung diem chi dat:

| Category | Ket qua |
|---|---:|
| Simple | 2/4 |
| Medium | 1/4 |
| Hard | 0/2 |
| Total | 3/10 |

Danh gia thu cong trajectory cho thay van de lon hon con so 3/10:
nhieu file `trajectory_task_*.json` co `steps: []`, tuc la agent khong
execute tool that. Model chi tra loi dang lap ke hoach, vi du:

- "I will use write_file"
- "Call read_file"
- "Tool: ls"
- `call:python_interpreter{...}` khong dung tool registry cua project

Vi vay:

- `evaluator_score`: 3/10.
- `action_level_score`: gan 0/10.
- Cac task dang PASS co kha nang la false positive neu khong co tool step lien quan.

---

## 2. Loi tong quan can sua

1. Prompt/AgentLoop chua ep model sinh tool call theo protocol on dinh.
2. Parser tool call chua bat duoc cac format Gemini/Gemma hay tra ve.
3. Tool name trong prompt, parser, registry va `tasks.json` chua dong bo.
4. Evaluator cho pass khi output co keyword, du agent chua thuc thi tool.
5. Functional evaluator co the tu chay script doc lap va pass do artifact cu.
6. Build co rui ro neu include truc tiep `<inplace_vector>` tren compiler chua ho tro.
7. Conflict marker trong source lam build/pipeline khong sach:
   - `src/agent/agent_loop.cpp`
   - `benchmark/demo_multi_agent.cpp`
8. Bao cao benchmark chua tach ro score evaluator va score thuc thi hanh dong.

---

## 3. Role A - Systems/Core

### Pham vi Role A

Role A phu trach:

- `LLMClient`, `OllamaClient`, `GeminiClient`.
- `AgentLoop`.
- Parser tool call.
- Prompt integration voi skills.
- `LoopDetector`.
- Integration pipeline de model that goi tool that.

### Loi cua Role A

- `AgentLoop` coi planning text la final answer qua som.
- System prompt chua liet ke tool name/chuc nang/input format ro.
- Prompt chua cam planning-only response.
- Parser chua robust voi:
  - JSON raw: `{"tool":"write_file","args":"result.txt,1081"}`
  - JSON trong markdown fence.
  - `ACTION: tool(args)`.
  - `call:provider:tool{...}` neu model sinh ra.
  - Gemini `functionCall` structured part neu API tra ve.
- `GeminiClient` moi lay text part, chua normalize function call ve action ma `AgentLoop` hieu.
- Lich su conversation co the phinh to; truncate phai giu system prompt va cac message gan nhat.
- C++26 `std::inplace_vector` khong duoc include truc tiep neu compiler chua co.

### Checklist fix Role A

- [ ] Resolve conflict marker trong `src/agent/agent_loop.cpp`.
- [ ] Khong include `<inplace_vector>` trong header public neu khong can.
- [ ] Dung C++26 feature theo cach portable:
  - Uu tien wrapper `FixedCapacityVector<T,N>`.
  - Neu `__has_include(<inplace_vector>)` va `__cpp_lib_inplace_vector` co thi alias sang `std::inplace_vector`.
  - Neu khong co thi fallback `std::vector` co `reserve(N)`.
  - Ghi comment ro day la C++26 feature co fallback, khong phai bo yeu cau C++26.
- [ ] Giu CMake compile voi `-std=c++26`/`/std:c++latest`.
- [ ] Chuan hoa system prompt trong `AgentLoop::run()`:
  - Liet ke dung tool canonical dang register.
  - Mo ta input format ngan gon cho tung tool.
  - Khi can tool, model chi duoc tra ve dung 1 JSON object.
  - Cam "I will call...", "Plan:", "Tool:" neu chua dung protocol.
  - Final answer chi xuat sau khi tool result da co neu task yeu cau tool/file.
- [ ] Inject skill selected tu `SkillLoader` vao system prompt, khong bo qua skill system.
- [ ] Hoan thien `parse_llm_response()`:
  - Parse JSON raw.
  - Parse JSON trong ```json fence.
  - Parse `ACTION: tool(args)`.
  - Parse whitespace/newline variants.
  - Parse/normalize `call:provider:tool{...}` thanh `ToolCallAction`.
  - Neu text co dau hieu muon goi tool nhung sai format, tra ve retry instruction thay vi final answer.
- [ ] Sua `GeminiClient`:
  - Neu response co text part thi tra text nhu hien tai.
  - Neu response co `functionCall`, convert thanh `{"tool":"...","args":...}`.
  - Map loi rate limit/timeout/malformed JSON ve `std::unexpected`.
- [ ] Verify `LoopDetector` reset moi task va khong chan nham tool call hop le.
- [ ] Khong include `harness/` trong `agent_loop.h/.cpp`.

### Tieu chi nghiem thu Role A

- Build pass tren WSL/Linux bang C++26.
- `AgentLoop` van build tren compiler chua co `<inplace_vector>`.
- Task can tool khong duoc ket thuc bang planning text.
- `trajectory_task_005`, `008`, `009`, `010` co `steps` khac rong neu model thuc hien dung.
- Khi model sinh sai format nhung co y dinh goi tool, agent nhac lai protocol thay vi coi la final answer.

---

## 4. Role B - Tools/Data

### Pham vi Role B

Role B phu trach:

- `Tool`, `ToolRegistry`, `Registry<T>`.
- Tool implementations.
- Tool descriptions cho prompt.
- Smart pointer ownership.
- Args parsing cua tung tool.
- `VLMEvaluator` skeleton neu da giao.

### Loi cua Role B

- Tool name model hay goi khong khop registry:
  - Model hay sinh: `create_file`, `append_file`, `list_files`, `google_search`, `calculate`, `exec`, `python_interpreter`.
  - Project can canonical: `write_file`, `read_file`, `execute_shell`, `web_search`, `calculator`, `memory`.
- Tool descriptions chua du ro de model sinh args dung.
- Args parser cua file/shell/calculator chua chap nhan format thuc te:
  - `result.txt,1081`
  - `{"path":"result.txt","content":"1081"}`
  - `{"filename":"result.txt","content":"1081"}`
  - `path=notes.txt`
- Tool error co nguy co throw exception thay vi return `std::unexpected`.
- Memory tool can ro `memory_save`/`memory_search` hoac mode trong `memory`.

### Checklist fix Role B

- [ ] Kiem tra toan bo `get_name()` trong `src/tools`.
- [ ] Lap bang canonical tool:

| Canonical | Bat buoc? | Ghi chu |
|---|---:|---|
| `calculator` | Yes | tinh bieu thuc so hoc |
| `execute_shell` | Yes | chay lenh shell co policy |
| `read_file` | Yes | doc file |
| `write_file` | Yes | ghi file |
| `web_search` | Yes | DuckDuckGo/SearXNG/API tuong duong |
| `memory_save` / `memory_search` hoac `memory` | Yes | SQLite memory |
| `time` | No | tool them |
| `json` | No | tool them |
| `git` | No | tool them |

- [ ] Them alias co kiem soat:
  - `create_file` -> `write_file`
  - `append_file` -> `write_file` voi append mode, hoac them `FileAppendTool`
  - `list_files` -> `execute_shell` voi `ls`/`dir`, hoac them tool rieng
  - `google_search` -> `web_search`
  - `calculate` -> `calculator`
  - `exec` -> `execute_shell`
- [ ] Khong expose `python_interpreter` neu project khong co tool do.
- [ ] Sua tool descriptions:
  - Ngan.
  - Noi dung dung voi `get_name()`.
  - Co example args.
  - Khong goi ten tool khong ton tai.
- [ ] Sua args parsing:
  - File tools chap nhan JSON va string args.
  - Calculator trim whitespace, reject ky tu nguy hiem/khong ho tro.
  - ExecTool co allow/deny policy va return loi ro.
  - WebSearchTool return loi ro khi network/API fail.
  - MemoryTool tach save/search ro rang.
- [ ] Moi tool return `std::unexpected(ToolError::InvalidArgument)` khi args sai.
- [ ] Khong dung raw `new/delete`; dung `unique_ptr`, `shared_ptr`, `make_unique`, `make_shared`.
- [ ] Verify `benchmark/run_eval.cpp` dang ky du tool bat buoc.
- [ ] `Registry<T>` generic build duoc va dung ownership dung.

### Tieu chi nghiem thu Role B

- `write_file` voi args JSON tao file dung.
- `write_file` voi args string `result.txt,1081` tao file dung.
- Alias pho bien khong lam task fail vi `Tool not found`.
- Tool loi args tra loi co message ro de model co the retry.
- Khong con false fail do prompt noi tool name khac registry.

---

## 5. Role C - Eval/Infra

### Pham vi Role C

Role C phu trach:

- `HarnessRunner`.
- `Task` loading.
- Trajectory output.
- Benchmark 10 task.
- Summary report.
- Multi-agent demo.
- Bao cao success rate that.

### Loi cua Role C

- Report hien tai co `3/10` nhung chua noi ro day co the la false positive.
- `steps: []` khong bi flag fail trong cac task can tool.
- Keyword evaluator pass khi output chi lap lai keyword trong ke hoach.
- Functional evaluator co the pass vi eval script tu chay tren artifact cu.
- Benchmark chua clean state truoc moi run.
- Summary chua ghi reason fail ro.

### Checklist fix Role C

- [ ] Resolve conflict marker trong `benchmark/demo_multi_agent.cpp`.
- [ ] Truoc moi benchmark run, clean artifact co the lam pass gia:
  - `notes.txt`
  - `result.txt`
  - `capital.txt`
  - `output.txt`
  - `calc.txt`
  - `data.txt`
  - cac file task-specific khac neu co trong `tasks.json`
- [ ] Khong clean file source/config/report can giu.
- [ ] Moi task trong `tasks.json` phai co:
  - `id`
  - `description`
  - `instruction`
  - `eval_type`
  - `eval_script` hoac `expected_keywords`
  - `max_steps`
  - nen them `category`: simple/medium/hard
  - nen them `requires_tool`: true/false
- [ ] Trajectory phai ghi du:
  - `task_id`
  - `model`
  - `success`
  - `total_time_ms`
  - `steps`
  - moi step co `thought`, `action`, `tool_result`, `latency_ms`
- [ ] Summary report them:
  - `tool_steps_count`
  - `requires_tool`
  - `failure_reason`
  - `evaluator_score`
  - `action_level_score`
- [ ] Neu task can tool ma `steps` rong, danh dau `NO_TOOL_EXECUTION` va fail action-level.
- [ ] Functional task chi PASS khi:
  - co tool step lien quan.
  - post-condition dung.
  - eval script output co `PASS`.
  - artifact duoc tao trong run hien tai, khong phai file cu.
- [ ] Keyword task chi PASS khi:
  - final output co keyword.
  - neu task yeu cau file/tool thi trajectory co tool step lien quan.
- [ ] Luu log/report theo timestamp, khong ghi de lan chay cu.
- [ ] Chay `test_multi_agent` va `demo_multi_agent`.
- [ ] Verify `report.txt` co ca `1081` va `Tokyo`.
- [ ] Chay full benchmark 10 task voi Gemini that, khong dung mock khi ghi bao cao.

### Tieu chi nghiem thu Role C

- Benchmark summary khong gay hieu nham `3/10` la success that.
- Moi task fail co reason:
  - `PARSER_FAIL`
  - `TOOL_NOT_FOUND`
  - `INVALID_ARGS`
  - `NO_TOOL_EXECUTION`
  - `POST_CONDITION_FAIL`
  - `RATE_LIMIT`
  - `TIMEOUT`
- Report co score theo category:
  - simple x/4
  - medium x/4
  - hard x/2
- Report co 2 con so rieng:
  - `evaluator_score`
  - `action_level_score`
- Multi-agent demo van pass sau khi Role A/B sua AgentLoop/tools.

---

## 6. Thu tu uu tien sua

1. Resolve conflict source:
   - `src/agent/agent_loop.cpp`
   - `benchmark/demo_multi_agent.cpp`
2. Dam bao build C++26 portable:
   - giu `-std=c++26`.
   - C++26 feature co fallback neu compiler chua ho tro.
3. Sua AgentLoop prompt + parser de tool call duoc execute that.
4. Dong bo tool canonical/aliases/descriptions/args parsing.
5. Sua harness clean state va flag `steps: []`.
6. Sua evaluator de loai false positive.
7. Chay `test_multi_agent`, `demo_multi_agent`.
8. Chay full benchmark 10 task voi Gemini/Gemma/Ollama backend that.
9. Cap nhat summary report voi evaluator score va action-level score.
10. Cap nhat README/bao cao neu co thay doi build/run/config.

---

## 7. Expected result sau vong fix dau

Muc tieu toi thieu sau vong fix dau:

| Nhom task | Muc tieu |
|---|---:|
| Simple | 4/4 |
| Medium | 3/4 hoac 4/4 |
| Hard | 1/2 tro len |
| Total | it nhat 8/10 |

Neu sau khi sua parser van fail, doc trajectory theo thu tu:

- `steps: []` -> loi A: prompt/parser/model protocol.
- Co step nhung `Tool not found` -> loi B: tool name/alias/registry.
- Co step, tool result loi args -> loi B: args parsing/tool implementation.
- Co step, tool result dung, evaluator fail -> loi C: evaluator/post-check.
- Functional PASS nhung khong co step -> loi C: false positive.
- Fail do 429/rate limit -> Role A/C: retry/backoff/sleep giua task.

---

## 8. Definition of Done

Chi coi benchmark hop le khi tat ca dieu kien sau dung:

- Source khong con conflict marker.
- Build pass tren WSL/Linux bang CMake.
- Khong bo yeu cau C++26; co it nhat 1 C++26 feature trong code hoac guarded fallback ro rang.
- `AgentLoop` khong phu thuoc `HarnessRunner`.
- Du tool bat buoc cua de bai duoc register runtime.
- Skills `.md` duoc load/inject.
- 10 task benchmark duoc chay tren backend model that.
- Trajectory cua task can tool co tool steps that.
- Summary report tach `evaluator_score` va `action_level_score`.
- README/bao cao giai thich duoc cac design pattern bat buoc va C++ features da dung.

