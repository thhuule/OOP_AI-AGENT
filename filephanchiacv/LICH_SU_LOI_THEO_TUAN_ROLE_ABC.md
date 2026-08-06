# Lịch sử lỗi theo tuần và Role — AI-AGENT OOP 2026

> **Mục đích:** phục hồi các lỗi đã phát sinh nhưng chưa được ghi chép đầy đủ trong quá trình làm đồ án.
>
> **Phân công:** Role A — Systems/Core; Role B — Tools/Data; Role C — Evaluation/Infra.
>
> **Nguồn phục hồi:** Git reflog/commit statistics, kế hoạch Tuần 6–9, báo cáo sửa lỗi, benchmark summary, `eval_results.json` và trajectory.
>
> **Lưu ý:** đây là lịch sử lỗi của codebase theo phạm vi ownership, không dùng để kết luận cá nhân nào gây lỗi khi nhiều thành viên sử dụng chung tài khoản hoặc commit không ghi rõ tác giả phần việc.

---

## 1. Quy ước mức độ tin cậy

| Mức | Ý nghĩa |
|---|---|
| **Đã xác nhận** | Có commit sửa, benchmark, trajectory hoặc source hiện tại làm bằng chứng trực tiếp |
| **Có bằng chứng gián tiếp** | Kế hoạch/commit ghi “fix” hoặc file thay đổi lớn nhưng không còn log tái hiện lỗi ban đầu |
| **Cần xác minh** | Có rủi ro hoặc dấu hiệu, nhưng chưa đủ bằng chứng để kết luận lỗi đã thực sự xảy ra |

Một lỗi chỉ được đánh dấu `CLOSED` khi có bản sửa và regression test hoặc bằng chứng xác minh tương đương.

---

## 2. Tổng quan lỗi theo tuần

| Tuần | Role A | Role B | Role C | Ảnh hưởng chính |
|---|---:|---:|---:|---|
| Tuần 3 | 2 | 2 | 3 | Lịch sử Git và skeleton chưa ổn định, thiếu test thực thi |
| Tuần 4 | 1 | 1 | 5 | Sai cấu trúc, sai task format, dependency/build artifacts |
| Tuần 5 | 3 | 2 | 4 | Build conflict, pipeline chưa nối hoàn chỉnh, config hygiene |
| Tuần 6 | 5 | 3 | 5 | Gemini integration, loop detection, multi-agent/concurrency |
| Tuần 7–8 | 7 | 6 | 8 | Tool call không được thực thi, evaluator false positive |
| Cuối Tuần 8/đầu Tuần 9 | 4 | 5 | 6 | File args sai, artifact sai, scoring/failure reason sai |

Số lượng trên là số nhóm lỗi phục hồi được, không phải điểm trừ hoặc tỷ lệ đóng góp.

---

## 3. Tuần 3 — Khởi tạo kiến trúc và skeleton

### 3.1 Role A — Systems/Core

#### W3-A-01 — Cấu trúc Agent/Core chưa ổn định

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** `main.cpp` từng được đổi tên thành `agent_loop.h`; các interface/client được tạo và sửa nhiều lần trong chuỗi rebase/amend.
- **Ảnh hưởng:** Khó xác định ranh giới giữa entry point, AgentLoop và LLM interface; tăng nguy cơ header/source không khớp.
- **Bằng chứng:** Git reflog có chuỗi create/rename/amend đối với `main.cpp`, `agent_loop.h` và `llm_client.h`.
- **Trạng thái:** `RESOLVED_BY_RESTRUCTURE`.
- **Regression cần có:** build tất cả target và kiểm tra dependency layer sau mỗi lần di chuyển file.

#### W3-A-02 — Commit không truy vết được yêu cầu

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Nhiều commit chỉ có tên `Tuần 3`, `Create src`, `Update ...`.
- **Ảnh hưởng:** Không biết commit nào triển khai requirement nào hoặc sửa lỗi gì.
- **Trạng thái:** `PROCESS_GAP`.
- **Biện pháp:** commit phải có Task ID, requirement, verification và checklist update.

### 3.2 Role B — Tools/Data

#### W3-B-01 — Tool directory và interface bị tạo/xóa nhiều lần

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Git reflog ghi `Create tools`, `Delete src/tools`, sau đó tạo lại `tool.h`, `ToolRegistry`, Calculator/File tool.
- **Ảnh hưởng:** Ownership và API của tool chưa ổn định; dễ làm code sử dụng interface cũ.
- **Trạng thái:** `RESOLVED_BY_RESTRUCTURE`.
- **Regression cần có:** compile test cho `Tool`/`ToolRegistry` và inventory canonical tool.

#### W3-B-02 — Tool implementation chưa có unit test

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** `src/tests/unit tests` hiện là file rỗng.
- **Ảnh hưởng:** Lỗi args parsing và artifact chỉ được phát hiện khi chạy benchmark thật.
- **Trạng thái:** `CLOSED` ngày 2026-08-06 bởi main snapshot `e9e1d35` và CTest 3/3.
- **Cần bổ sung:** test Calculator, File, Registry, Exec, JSON, Memory và Web error path.

### 3.3 Role C — Evaluation/Infra

#### W3-C-01 — Harness skeleton bị tạo/xóa và đổi interface

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Git reflog ghi `Create harness`, `Delete src/harness`, tạo lại `evaluator.h` và `run_eval.cpp`.
- **Ảnh hưởng:** Task/Harness/Evaluator contract chưa ổn định.
- **Trạng thái:** `RESOLVED_BY_RESTRUCTURE`.

#### W3-C-02 — Unit test chỉ là placeholder

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Commit từng ghi `Create unit tests`, nhưng file hiện tại không có nội dung và không có executable unit-test rõ ràng.
- **Ảnh hưởng:** Không có bằng chứng test cho harness/evaluator ở giai đoạn đầu.
- **Trạng thái:** `OPEN`.

#### W3-C-03 — Lịch sử Git bị rewrite nhiều lần

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Revert/reapply, reset, rebase, filter-branch xuất hiện liên tiếp.
- **Ảnh hưởng:** Có thể mất commit cũ, làm sai thời điểm và ownership khi phục hồi lịch sử lỗi.
- **Trạng thái:** `PROCESS_GAP`.
- **Biện pháp:** tránh rewrite nhánh chung; dùng PR/merge và tag checkpoint theo tuần.

---

## 4. Tuần 4 — Task format, Harness và cấu trúc thư mục

### 4.1 Role A — Systems/Core

#### W4-A-01 — `SkillLoader` đặt sai tầng/thư mục

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** `src/SkillLoader.cpp` phải chuyển sang `src/agent/SkillLoader.cpp`.
- **Ảnh hưởng:** Cấu trúc không phản ánh ownership của Agent/Core; include path dễ sai.
- **Bằng chứng:** commit `4b70ebc`.
- **Trạng thái:** `RESOLVED`.

### 4.2 Role B — Tools/Data

#### W4-B-01 — Dependency/include path của JSON chưa đúng

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Phải sửa clangd/CMake include path và thêm nlohmann/json headers.
- **Ảnh hưởng:** Build hoặc IDE không tìm được JSON dependency.
- **Bằng chứng:** commit `1407275`.
- **Trạng thái:** `RESOLVED`.
- **Lưu ý:** không chỉnh sửa nội dung vendored headers trong `include/nlohmann/`.

### 4.3 Role C — Evaluation/Infra

#### W4-C-01 — Benchmark đặt sai thư mục

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Phải xóa `src/benchmark` và chuyển `tasks.json` sang `benchmark/tasks.json`.
- **Ảnh hưởng:** Entry point và benchmark artifacts trộn với production source.
- **Bằng chứng:** commit `4b70ebc`.
- **Trạng thái:** `RESOLVED`.

#### W4-C-02 — `tasks.json` không đúng format đề bài

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** `tasks.json`, `HarnessRunner` và `Task` phải sửa đồng thời “cho đúng yêu cầu format của đề”.
- **Ảnh hưởng:** Load sai field, evaluator dispatch sai hoặc mất `max_steps`/expected data.
- **Bằng chứng:** commit `89f32ab`.
- **Trạng thái:** `RESOLVED`.
- **Regression cần có:** schema validation cho toàn bộ task.

#### W4-C-03 — Evaluator dispatch chưa đúng

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** `HarnessRunner` phải cập nhật logic dispatch theo `eval_type`.
- **Ảnh hưởng:** Task có thể bị chấm bằng evaluator sai.
- **Bằng chứng:** commit `1407275`.
- **Trạng thái:** `RESOLVED`.

#### W4-C-04 — Commit nhầm build artifacts

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Commit `00af899` và `0d90ff7` chứa nhiều file dưới `build/`, object file và executable.
- **Ảnh hưởng:** Repository nặng, diff nhiễu, kết quả build phụ thuộc máy cũ.
- **Trạng thái:** `RESOLVED_LATER_IN_WEEK5`.

#### W4-C-05 — Thay đổi cấu trúc kéo theo build metadata cũ

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Commit đổi vị trí header/source đồng thời thay hàng nghìn dòng trong CMake cache/build metadata.
- **Ảnh hưởng:** Khó phân biệt source change với generated change và dễ gây conflict.
- **Trạng thái:** `RESOLVED_LATER_IN_WEEK5`.

---

## 5. Tuần 5 — Kết nối pipeline và xử lý build

### 5.1 Role A — Systems/Core

#### W5-A-01 — `AgentLoop` chưa kết nối đúng với Harness pipeline

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Phải có commit riêng `Sửa để kết nối HarnessRunner & AgentLoop`.
- **Ảnh hưởng:** `run_eval` không thể chạy agent end-to-end hoặc không nhận output đúng.
- **Bằng chứng:** commit `b7de113`.
- **Trạng thái:** `RESOLVED`.

#### W5-A-02 — Skill injection chưa được xác minh trong pipeline

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Sau integration phải sửa `agent_loop.cpp`, `run_eval.cpp`, `main.cpp` và `task_planner.md` để “Check xong Pipeline, Skill Inject, StepHook”.
- **Ảnh hưởng:** Skill có thể được load nhưng không thực sự đi vào system prompt.
- **Bằng chứng:** commit `997da57`.
- **Trạng thái:** `RESOLVED_WITHOUT_FOCUSED_TEST`.

#### W5-A-03 — AgentLoop còn nhiều lỗi tích hợp cuối tuần

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Commit `81504a6 — Fix Lỗi còn lại tuần 5` thay đổi lớn `agent_loop.cpp`, `run_eval.cpp`, `main.cpp`.
- **Ảnh hưởng:** Cho thấy pipeline trước đó chưa ổn định hoàn toàn.
- **Trạng thái:** `RESOLVED_PARTIALLY`; lỗi parser/tool-call vẫn xuất hiện ở Tuần 7–8.

### 5.2 Role B — Tools/Data

#### W5-B-01 — `FileTool` cần sửa lớn sau integration

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Commit `81504a6` thay đổi lớn `FileTool.cpp`.
- **Ảnh hưởng:** Contract args/ghi file ở giai đoạn này chưa ổn định; lỗi tương tự tiếp tục xuất hiện trong benchmark Tuần 8.
- **Trạng thái:** `REGRESSED_LATER`.

#### W5-B-02 — Chưa có regression test cho file args

- **Mức tin cậy:** Đã xác nhận từ trạng thái test hiện tại và lỗi benchmark sau đó.
- **Triệu chứng:** Không có test JSON/comma args/append.
- **Ảnh hưởng:** `FileWriteTool` có thể build pass nhưng tạo sai filename/content.
- **Trạng thái:** `OPEN`.

### 5.3 Role C — Evaluation/Infra

#### W5-C-01 — Build conflict

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Commit `25b80a8 — resolve build conflicts` thay đổi CMake, AgentLoop, main và generated build files.
- **Ảnh hưởng:** Build không ổn định giữa các branch/máy.
- **Trạng thái:** `RESOLVED`.

#### W5-C-02 — Build directory vẫn được theo dõi trong Git

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Commit `2e43806` xóa hàng nghìn dòng generated files và thêm ignore rule.
- **Ảnh hưởng:** Repository phình to, conflict và stale binary.
- **Trạng thái:** `RESOLVED`.

#### W5-C-03 — `StepHook`/trajectory chưa được xác minh sớm

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Chỉ sau commit `997da57` pipeline, skill injection và StepHook mới được kiểm tra chung.
- **Ảnh hưởng:** Có thể tạo trajectory rỗng hoặc thiếu action ở giai đoạn trước.
- **Trạng thái:** `RESOLVED_PARTIALLY`; trajectory args vẫn mất ở run sau.

#### W5-C-04 — `config.json` từng được commit

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Commit `75e8336` thêm `config.json` vào repository.
- **Ảnh hưởng:** Rủi ro lộ API key/URL hoặc khiến config cá nhân trở thành config chung.
- **Trạng thái:** `SECURITY_REVIEW_REQUIRED`.
- **Biện pháp:** kiểm tra lịch sử mà không in key; nếu từng có key thật thì revoke/rotate; chỉ commit `config.json.example`.

---

## 6. Tuần 6 — Gemini, LoopDetector và Multi-agent foundation

### 6.1 Role A — Systems/Core

#### W6-A-01 — Gemini structured `functionCall` chưa được normalize

- **Mức tin cậy:** Đã xác nhận bởi benchmark Tuần 7–8.
- **Triệu chứng:** `GeminiClient` chỉ lấy text part đầu tiên; structured `functionCall` không được chuyển thành action của `AgentLoop`.
- **Ảnh hưởng:** Model muốn gọi tool nhưng trajectory không có tool step.
- **Trạng thái:** `RESOLVED_LATER`.

#### W6-A-02 — Mapping response/protocol Gemini và AgentLoop chưa đồng bộ

- **Mức tin cậy:** Đã xác nhận bởi benchmark.
- **Triệu chứng:** Agent parser mong `ACTION: tool(args)` trong khi Gemini/Gemma sinh raw JSON, fenced JSON hoặc provider call.
- **Ảnh hưởng:** Planning/tool intent bị coi là final answer.
- **Trạng thái:** `RESOLVED_LATER`.

#### W6-A-03 — LoopDetector chỉ theo dõi tool name

- **Mức tin cậy:** Đã xác nhận trong phân tích run `212302_253`.
- **Triệu chứng:** Nhiều lệnh `execute_shell` khác args vẫn bị coi là lặp.
- **Ảnh hưởng:** Task 009 bị dừng nhầm do infinite-loop detection.
- **Trạng thái:** `RESOLVED_LATER`.
- **Regression cần có:** cùng tool/cùng args, cùng tool/khác args và ping-pong.

#### W6-A-04 — C++26 portability chưa an toàn

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Dùng/include `std::inplace_vector` có nguy cơ fail trên compiler thiếu header/feature.
- **Ảnh hưởng:** Không build được dù CMake yêu cầu C++26.
- **Trạng thái:** `MITIGATED_BY_GUARDED_FALLBACK`; cần test compiler path.

#### W6-A-05 — Conversation/history và final-answer logic chưa có test

- **Mức tin cậy:** Được xác nhận gián tiếp bởi run 9/10 sau này.
- **Triệu chứng:** Tool trả đúng observation nhưng final answer có thể trở thành câu chào chung.
- **Ảnh hưởng:** Task fail dù tool execution thành công.
- **Trạng thái:** `OPEN_REGRESSION_TEST`.

### 6.2 Role B — Tools/Data

#### W6-B-01 — Tool error handling chưa đồng nhất

- **Mức tin cậy:** Có bằng chứng gián tiếp từ kế hoạch polish.
- **Triệu chứng:** Các tool cần được sửa để trả `InvalidArgument`/`ExecutionFailed` thay vì throw hoặc lỗi chung.
- **Ảnh hưởng:** Model không biết nguyên nhân để retry.
- **Trạng thái:** `PARTIALLY_RESOLVED`.

#### W6-B-02 — Action/args contract thay đổi nhưng thiếu test

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Chuyển sang `std::variant` cho action nhưng không có unit test thực thi cho parser → action → tool args.
- **Ảnh hưởng:** Type an toàn hơn nhưng dữ liệu args vẫn có thể sai/mất.
- **Trạng thái:** `OPEN_TEST_GAP`.

#### W6-B-03 — Tool compatibility với Gemini chưa được chứng minh

- **Mức tin cậy:** Đã xác nhận bởi benchmark sau đó.
- **Triệu chứng:** Tool names và args do Gemini sinh không khớp registry/execute contract.
- **Ảnh hưởng:** `Tool not found` hoặc `InvalidArgument`.
- **Trạng thái:** `RESOLVED_LATER`.

### 6.3 Role C — Evaluation/Infra

#### W6-C-01 — Model metadata có nguy cơ hardcode

- **Mức tin cậy:** Có bằng chứng gián tiếp từ kế hoạch.
- **Triệu chứng:** Trajectory cần đổi model từ chuỗi cố định sang giá trị lấy từ config.
- **Ảnh hưởng:** Báo cáo sai backend/model thật.
- **Trạng thái:** `RESOLVED_LATER`.

#### W6-C-02 — Batch progress và category score chưa đầy đủ

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Phải nâng cấp `HarnessRunner` để in tiến độ và phân loại simple/medium/hard.
- **Ảnh hưởng:** Khó theo dõi run và tính success rate theo đề.
- **Bằng chứng:** commit `94193e4`.
- **Trạng thái:** `RESOLVED`.

#### W6-C-03 — `MultiAgentRunner::startAll()` capture không an toàn

- **Mức tin cậy:** Đã xác nhận trong kế hoạch Tuần 7–8.
- **Triệu chứng:** Cần copy `task_func` trước khi spawn thread để tránh undefined behavior.
- **Ảnh hưởng:** Worker có thể dùng reference invalid hoặc chạy sai function.
- **Trạng thái:** `RESOLVED_LATER`.

#### W6-C-04 — Rate-limit khi nhiều agent gọi Gemini

- **Mức tin cậy:** Cần xác minh theo từng run; rủi ro đã được ghi nhận.
- **Triệu chứng:** Hai agent hoặc batch nhiều task có thể vượt RPM.
- **Ảnh hưởng:** HTTP 429, task fail không phải do logic agent.
- **Trạng thái:** `RISK`; cần backoff/serialization và failure reason `RATE_LIMIT`.

#### W6-C-05 — Trajectory format chưa có schema test

- **Mức tin cậy:** Có bằng chứng gián tiếp.
- **Triệu chứng:** Format phải được “verify lại” theo đề; các lỗi mất args/tokens xuất hiện sau đó.
- **Ảnh hưởng:** Artifact JSON hợp lệ cú pháp nhưng thiếu dữ liệu debug quan trọng.
- **Trạng thái:** `OPEN_TEST_GAP`.

---

## 7. Tuần 7–8 — Full integration và benchmark Gemini thật

### 7.1 Role A — Systems/Core

#### W78-A-01 — Planning text bị coi là final answer

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Model trả “I will use write_file”, “Call read_file”, “Tool: ls” nhưng AgentLoop kết thúc.
- **Ảnh hưởng:** Không tool nào được execute dù model có ý định hành động.
- **Trạng thái:** `RESOLVED_LATER`.

#### W78-A-02 — System prompt không liệt kê canonical tool/protocol

- **Mức tin cậy:** Đã xác nhận trong báo cáo benchmark 29/07.
- **Triệu chứng:** Prompt chỉ nói tool tồn tại, không mô tả rõ tên, args và một format bắt buộc.
- **Ảnh hưởng:** Model sinh tool name/format không hợp lệ.
- **Trạng thái:** `RESOLVED_LATER`.

#### W78-A-03 — Parser không hỗ trợ đủ format Gemini/Gemma

- **Mức tin cậy:** Đã xác nhận.
- **Thiếu format:** raw JSON, fenced JSON, `ACTION:`, whitespace variants, provider calls và structured `functionCall`.
- **Ảnh hưởng:** `steps: []`, action-level bằng 0.
- **Trạng thái:** `RESOLVED_LATER`.

#### W78-A-04 — JSON string args không được unescape đúng

- **Mức tin cậy:** Đã xác nhận trong run `212302_253`.
- **Triệu chứng:** Parser chỉ bỏ dấu quote ngoài, không decode escape sequence.
- **Ảnh hưởng:** Shell/file tool nhận quote/backslash literal.
- **Trạng thái:** `RESOLVED_LATER`.

#### W78-A-05 — Mất chi tiết `ToolError`

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Mọi lỗi tool bị đổi thành `Tool Execution Error`.
- **Ảnh hưởng:** Model không phân biệt `NotFound`, `InvalidArgument`, `ExecutionFailed` để phục hồi.
- **Trạng thái:** `RESOLVED_LATER`.

#### W78-A-06 — LoopDetector false positive

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Chỉ so sánh tool name; nhiều command khác nhau bị coi là lặp.
- **Ảnh hưởng:** Task 009 dừng sớm.
- **Trạng thái:** `RESOLVED_LATER`.

#### W78-A-07 — Conflict marker trong `agent_loop.cpp`

- **Mức tin cậy:** Đã xác nhận trong báo cáo sửa lỗi.
- **Ảnh hưởng:** Source/build không sạch.
- **Trạng thái:** `RESOLVED`; kiểm tra hiện tại không còn marker đầu dòng.

### 7.2 Role B — Tools/Data

#### W78-B-01 — Canonical tool name không đồng bộ

- **Mức tin cậy:** Đã xác nhận.
- **Model thường sinh:** `create_file`, `append_file`, `list_files`, `google_search`, `calculate`, `exec`, `python_interpreter`.
- **Registry mong đợi:** `write_file`, `read_file`, `web_search`, `calculator`, `execute_shell`, `memory`.
- **Ảnh hưởng:** `Tool not found` hoặc gọi tool không tồn tại.
- **Trạng thái:** `RESOLVED_PARTIALLY_BY_ALIASES`.

#### W78-B-02 — Tool description không đủ rõ

- **Mức tin cậy:** Đã xác nhận trong báo cáo sửa lỗi.
- **Triệu chứng:** Description không khớp canonical name hoặc không có example args.
- **Ảnh hưởng:** Model sinh args sai format.
- **Trạng thái:** `RESOLVED_PARTIALLY`.

#### W78-B-03 — `FileWriteTool` parse args sai

- **Mức tin cậy:** Đã xác nhận trực tiếp từ trajectory.
- **Triệu chứng:** Split filename bằng whitespace thay vì dấu phẩy đầu tiên.
- **Ví dụ:** `notes.txt,Agent test run` tạo file `notes.txt,Agent`; `result.txt,1081` tạo filename sai và content rỗng.
- **Ảnh hưởng:** Là lỗi gốc của phần lớn task fail trong run `212302_253`.
- **Trạng thái:** `RESOLVED`.
- **Regression cần có:** JSON args, comma args, content chứa comma, field thiếu và filename rỗng.

#### W78-B-04 — Không hỗ trợ JSON object args

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** JSON object bị biến thành filename bắt đầu bằng `{"filename":` hoặc `{"content":`.
- **Ảnh hưởng:** Tạo artifact malformed trong repository root.
- **Trạng thái:** `RESOLVED`; cần regression test.

#### W78-B-05 — Tool trả `OK` dù artifact sai

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Filename/content sai nhưng tool result vẫn là success.
- **Ảnh hưởng:** Agent tưởng thao tác thành công; action-level score đạt 1.0 giả.
- **Trạng thái:** `RESOLVED`; cần post-write/read-back test.

#### W78-B-06 — Chưa có append operation đúng

- **Mức tin cậy:** Đã xác nhận ở task 010.
- **Triệu chứng:** Agent tạo `data.txt` nhưng không nối dòng `appended`.
- **Ảnh hưởng:** Task hard fail/incomplete.
- **Trạng thái:** `RESOLVED_BY_APPEND_TOOL`.

### 7.3 Role C — Evaluation/Infra

#### W78-C-01 — Benchmark 3/10 nhưng hành động thực tế 0/10

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Báo cáo 29/07 có evaluator 3/10, nhưng review cho thấy không task nào hoàn tất hành động.
- **Ảnh hưởng:** Success rate gây hiểu nhầm.
- **Trạng thái:** `RESOLVED_BY_ACTION_LEVEL_SCORE`.

#### W78-C-02 — Keyword evaluator false positive

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Task 002 và 007 pass vì planning text lặp lại keyword.
- **Ảnh hưởng:** Không có tool step vẫn được tính PASS.
- **Trạng thái:** `RESOLVED_PARTIALLY`.

#### W78-C-03 — Functional evaluator false positive

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Task 004 pass vì eval script tự chạy `hello.sh`, không phải vì agent chạy.
- **Ảnh hưởng:** Evaluator đo script thay vì agent action.
- **Trạng thái:** `RESOLVED_PARTIALLY`.

#### W78-C-04 — Không fail action-level khi `steps: []`

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Task cần tool nhưng trajectory rỗng vẫn có thể evaluator PASS.
- **Ảnh hưởng:** False success.
- **Trạng thái:** `RESOLVED_BY_NO_TOOL_EXECUTION_CHECK`.

#### W78-C-05 — Không clean/isolate artifact trước run

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Functional evaluator có thể dùng file từ run cũ; sau đó còn thấy artifact malformed trong root.
- **Ảnh hưởng:** Pass giả hoặc lỗi dây chuyền giữa task.
- **Trạng thái:** `PARTIALLY_RESOLVED`; cần test cleanup cho mọi artifact khai báo.

#### W78-C-06 — Failure reason quá chung

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Nhiều lỗi khác nhau đều ghi `POST_CONDITION_FAIL`.
- **Ảnh hưởng:** Không biết lỗi thuộc parser, tool, artifact hay evaluator.
- **Trạng thái:** `PARTIALLY_RESOLVED`.

#### W78-C-07 — Trajectory làm mất args thật

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Export ghi `args: ""` dù model đã cung cấp args; args chỉ còn trong `thought`.
- **Ảnh hưởng:** Khó debug và không đúng trajectory requirement.
- **Trạng thái:** `RESOLVED`.

#### W78-C-08 — Conflict marker trong `demo_multi_agent.cpp`

- **Mức tin cậy:** Đã xác nhận trong báo cáo sửa lỗi.
- **Ảnh hưởng:** Build/demo không sạch.
- **Trạng thái:** `RESOLVED`.

---

## 8. Cuối Tuần 8/đầu Tuần 9 — Chuỗi benchmark 0/10 → 10/10

### 8.1 Diễn tiến các run

| Run | Evaluator | Action-level | Final | Kết luận |
|---|---:|---:|---:|---|
| `benchmark_20260729` | 3/10 | 0/10 qua manual review | 0/10 thực tế | Ba false positive, không tool action hoàn chỉnh |
| `run_20260729_154405_342` | 0.2 | 0 | 0 | Cả 10 task bị `NO_TOOL_EXECUTION` |
| `run_20260801_020539_767` | 0.2 | 1.0 | 0.2 | Có tool step nhưng 8 task không đạt hậu điều kiện |
| `run_20260801_212302_253` | 0.2 | 1.0 | 0.2 | File args/artifact sai; action score lạc quan giả |
| `run_20260801_215732_690` | 0.9 | 1.0 | 0.9 | Task 003 đọc đúng nhưng final answer sai |
| `run_20260801_220549_361` | 1.0 | 1.0 | 1.0 | 10/10 lịch sử; vẫn cần run sạch hiện tại |

### 8.2 Role A — Các lỗi còn lộ ra trong chuỗi run

#### W89-A-01 — Task có tool result đúng nhưng final answer sai

- **Mức tin cậy:** Đã xác nhận ở run `215732_690`, task 003.
- **Triệu chứng:** `read_file` trả `Agent test run`, nhưng final answer là câu chào chung.
- **Ảnh hưởng:** Evaluator fail dù action-level success.
- **Trạng thái:** `RESOLVED_IN_NEXT_RUN`; chưa có focused regression test.

#### W89-A-02 — Agent kết thúc task nhiều hậu điều kiện quá sớm

- **Mức tin cậy:** Đã xác nhận ở task 010 run `212302_253`.
- **Triệu chứng:** Tạo/đọc `initial data` nhưng chưa append đã final.
- **Ảnh hưởng:** `INCOMPLETE_TASK`.
- **Trạng thái:** `RESOLVED_IN_NEXT_RUN`.

#### W89-A-03 — Error recovery phụ thuộc lỗi chi tiết

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Khi ToolError bị rút gọn, model retry mù; sau khi giữ `NotFound`, task 010 phục hồi tốt hơn.
- **Trạng thái:** `RESOLVED`.

#### W89-A-04 — Token/context measurement chưa có

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** `tokens_used` và `total_tokens` bằng 0 trong trajectory.
- **Ảnh hưởng:** Không đánh giá được chi phí/context growth.
- **Trạng thái:** `OPEN`.

### 8.3 Role B — Các lỗi còn lộ ra trong chuỗi run

#### W89-B-01 — Sai filename/content dây chuyền nhiều task

- **Mức tin cậy:** Đã xác nhận.
- **Task ảnh hưởng:** 002, 003, 005, 006, 007, 009, 010.
- **Nguyên nhân:** `FileWriteTool` parse sai và append chưa hoàn chỉnh.
- **Trạng thái:** `RESOLVED`.

#### W89-B-02 — Artifact malformed còn sót

- **Mức tin cậy:** Đã xác nhận trong output task 001 run `215732_690`.
- **Triệu chứng:** Root còn file tên bắt đầu bằng `{"content":` và `{"filename":`.
- **Ảnh hưởng:** Nhiễu benchmark và chứng minh cleanup chưa bao phủ mọi filename sai.
- **Trạng thái:** `HISTORICAL_ARTIFACT_RISK`.

#### W89-B-03 — Append phải bảo toàn nội dung cũ

- **Mức tin cậy:** Đã xác nhận qua task 010.
- **Ảnh hưởng:** Nếu append thực chất overwrite, task có thể mất `initial data`.
- **Trạng thái:** `RESOLVED`; cần read-back regression test.

#### W89-B-04 — Alias/policy phải normalize theo cùng thứ tự

- **Mức tin cậy:** Có bằng chứng gián tiếp từ các lỗi tool name.
- **Ảnh hưởng:** Alias có thể lookup được nhưng bị allow/deny sai nếu policy check trước normalize.
- **Trạng thái:** `CURRENT_CODE_REVIEW_REQUIRED`.

#### W89-B-05 — Thiếu bộ test thực thi

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** `src/tests/unit tests` rỗng.
- **Ảnh hưởng:** Bản sửa benchmark chưa được khóa bằng regression test.
- **Trạng thái:** `OPEN`.

### 8.4 Role C — Các lỗi còn lộ ra trong chuỗi run

#### W89-C-01 — Task 001 instruction/evaluator mismatch

- **Mức tin cậy:** Đã xác nhận ở run `212302_253`.
- **Triệu chứng:** Instruction yêu cầu liệt kê file thư mục hiện tại; evaluator đòi `.cpp` và `.h` nằm trong thư mục con.
- **Ảnh hưởng:** Một cách thực hiện hợp lệ vẫn FAIL.
- **Trạng thái:** `RESOLVED_BY_TASK_ALIGNMENT`; cần đảm bảo không làm yếu hậu điều kiện.

#### W89-C-02 — Action-level score 1.0 giả

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Tool trả `OK` nên action-level pass dù artifact sai tên/nội dung.
- **Ảnh hưởng:** Báo cáo che mờ lỗi Role B.
- **Trạng thái:** `PARTIALLY_RESOLVED`; action-level cần gắn với hậu điều kiện artifact.

#### W89-C-03 — Failure taxonomy chưa đủ chi tiết

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** 8 lỗi khác nguyên nhân vẫn là `POST_CONDITION_FAIL`.
- **Cần phân biệt:** `PARSER_FAIL`, `TOOL_NOT_FOUND`, `INVALID_ARGS`, `TOOL_EXECUTION_FAILED`, `ARTIFACT_MISSING`, `ARTIFACT_CONTENT_MISMATCH`, `LOOP_DETECTED`, `INCOMPLETE_TASK`, `EVALUATOR_ERROR`, `RATE_LIMIT`.
- **Trạng thái:** `PARTIALLY_RESOLVED`.

#### W89-C-04 — Cleanup chưa bao phủ malformed artifact

- **Mức tin cậy:** Đã xác nhận bởi output task 001 ở run 9/10.
- **Triệu chứng:** Filename lỗi vẫn còn ở repository root.
- **Ảnh hưởng:** Run sau không hoàn toàn sạch.
- **Trạng thái:** `OPEN_REGRESSION_TEST`.

#### W89-C-05 — 10/10 là bằng chứng lịch sử

- **Mức tin cậy:** Đã xác nhận.
- **Triệu chứng:** Run 10/10 nằm trong artifacts cũ; chưa tự động chứng minh code/worktree hiện tại vẫn 10/10.
- **Ảnh hưởng:** Có thể báo cáo nhầm success rate nếu code đã thay đổi.
- **Trạng thái:** `CURRENT_CLEAN_RUN_REQUIRED` trước báo cáo cuối.

#### W89-C-06 — VLM evaluator còn skeleton

- **Mức tin cậy:** Đã xác nhận từ `VLMEvaluator.cpp` còn TODO.
- **Ảnh hưởng:** Không được mô tả như evaluator ảnh hoàn chỉnh.
- **Trạng thái:** `KNOWN_LIMITATION`.

---

## 9. Lỗi mở cần ưu tiên

| Ưu tiên | Error ID | Role | Việc cần làm | Evidence hoàn thành |
|---:|---|---|---|---|
| 1 | W3-B-02 / W89-B-05 | B | Tạo unit-test executable cho tool/parser | Test target build và PASS |
| 2 | W6-A-05 / W89-A-01 | A | Test observation → final answer | Tool trả đúng và final chứa observation |
| 3 | W89-C-02 | C | Siết action-level theo hậu điều kiện | Artifact sai phải action FAIL |
| 4 | W89-C-03 | C | Hoàn thiện failure taxonomy | Mỗi fixture lỗi cho reason khác nhau |
| 5 | W89-C-04 | C | Test cleanup malformed/stale artifacts | Run sạch không thấy file cũ |
| 6 | W89-A-04 | A/C | Ghi token usage thật hoặc trạng thái unsupported | Trajectory không dùng số 0 gây hiểu nhầm |
| 7 | W5-C-04 | C | Audit lịch sử `config.json` an toàn | Key cũ được rotate nếu từng commit |
| 8 | W89-C-05 | C | Chạy benchmark sạch hiện tại | Run directory mới, provider thật, 10 task |

---

## 10. Mẫu ghi lỗi cho các tuần tiếp theo

```md
### ERROR-ID — Tên lỗi

- Week:
- Role:
- Detected date:
- Detected by: test | benchmark | review | user report
- Task/run:
- Severity: LOW | MEDIUM | HIGH | CRITICAL
- Confidence: CONFIRMED | INDIRECT | NEEDS_VERIFICATION
- Status: OPEN | IN_PROGRESS | RESOLVED | CLOSED

#### Symptom

- Hiện tượng quan sát được.

#### Root cause

- Nguyên nhân gốc, không chỉ mô tả triệu chứng.

#### Impact

- Task/module/score bị ảnh hưởng.

#### Fix

- File và thay đổi đã thực hiện.

#### Evidence

- Commit/PR:
- Test command:
- Test result:
- Benchmark/trajectory:

#### Regression prevention

- Test hoặc guard ngăn lỗi quay lại.
```

---

## 11. Cập nhật Tuần 9.5 — Role C

### W95-C-01 — Harness chưa dùng Environment abstraction

- **Tuần:** 9.5.
- **Role:** C, tích hợp interface do A sở hữu.
- **Ngày xác minh:** 2026-08-05.
- **Phát hiện bởi:** review source và focused test.
- **Mức độ:** HIGH.
- **Độ tin cậy:** Đã xác nhận.
- **Trạng thái:** `CLOSED`.

#### Hiện tượng

- Harness từng tự gọi filesystem để cleanup và kiểm tra artifact dù `Environment` hierarchy đã tồn tại.
- Cleanup thất bại chưa có failure reason riêng.

#### Cách sửa

- Inject `std::shared_ptr<Environment>` vào `HarnessRunner`.
- Dùng `NativeEnvironment` mặc định khi chạy thật và `SandboxEnvironment` khi test.
- Cleanup/artifact existence đi qua interface; lỗi cleanup là `ARTIFACT_CLEANUP_FAILED`.
- Thêm Environment source vào CMake và focused test cho sandbox/cleanup failure.

#### Bằng chứng

- Fix log: `filephanchiacv/ROLE_C_ENVIRONMENT_FIX_LOG_20260805.md`.
- Build toàn bộ target: PASS.
- `./build/test_harness`: `ALL HARNESS TESTS PASSED`.
- `./build/test_multi_agent`: `ALL PASSED`.
- CTest: 2/2 PASS.

### W95-C-02 — Tài liệu Eval/video không khớp source và trajectory mới nhất

- **Tuần:** 9.5.
- **Role:** C.
- **Ngày xác minh:** 2026-08-05.
- **Phát hiện bởi:** audit tài liệu với source/test/run artifacts.
- **Mức độ:** MEDIUM.
- **Độ tin cậy:** Đã xác nhận.
- **Trạng thái:** `CLOSED`.

#### Hiện tượng

- Báo cáo mô tả action-level dựa trên chữ trong result, trong khi source dùng `TrajectoryStep::success`.
- Báo cáo chưa phân biệt pipeline 10/10 với năng lực lập kế hoạch của model khi fallback chạy trước LLM.
- Storyboard yêu cầu trình bày số bước task 005/010 không khớp hai trajectory gần nhất.

#### Cách sửa

- Sửa `docs/report_evaluation.md` theo đúng source, ghi rõ nhánh taxonomy nào có focused test và nhánh nào mới chỉ được implement.
- Bổ sung hai run 2026-08-05 và giới hạn fallback/token.
- Sửa README, submission checklist và storyboard để không tuyên bố fallback-assisted 10/10 là bằng chứng model reasoning.
- Storyboard bây giờ yêu cầu chỉ trình bày bước thật sự tồn tại trong trajectory được chọn.

#### Bằng chứng

- `benchmark/results/run_20260805_034207_664/trajectory_task_005.json`: 2 tool steps.
- `benchmark/results/run_20260805_034207_664/trajectory_task_010.json`: 2 tool steps.
- `benchmark/results/run_20260801_220549_361/trajectory_task_010.json`: 4-step historical recovery flow.
- Build toàn bộ target: PASS.
- `test_harness`, `test_multi_agent`, CTest 2/2: PASS.
- `git diff --check`: PASS.

#### Việc chuyển Tuần 10

- Focused test cho rate limit, timeout, loop và parser failure signal.
- Token telemetry thật.
- Ghi nguồn action `llm`/`fallback` vào trajectory.
- Artifact isolation nâng cao và benchmark thật sau khi A/B freeze.

---

## 12. Nguồn bằng chứng

- `filephanchiacv/FIX_LOI_ROLE_ABC_BENCHMARK.md`.
- `filephanchiacv/PHAN_TICH_RUN_20260801_212302_253.md`.
- `filephanchiacv/KH_Tuan6_Updated.md`.
- `filephanchiacv/KH_Tuan7_8_ChiTiet.md`.
- `benchmark/results/benchmark_20260729_summary.md`.
- `benchmark/results/run_20260729_154405_342/benchmark_summary.txt`.
- `benchmark/results/run_20260801_020539_767/benchmark_summary.txt`.
- `benchmark/results/run_20260801_212302_253/benchmark_summary.txt`.
- `benchmark/results/run_20260801_215732_690/benchmark_summary.txt`.
- `benchmark/results/run_20260801_220549_361/benchmark_summary.txt`.
- `benchmark/results/run_20260805_032212_365/benchmark_summary.txt`.
- `benchmark/results/run_20260805_034207_664/benchmark_summary.txt`.
- `filephanchiacv/ROLE_C_ENVIRONMENT_FIX_LOG_20260805.md`.
- `filephanchiacv/KH_TUAN9.5.md`.
- Git reflog và commit statistics trong repository.

> **Kết luận:** lỗi chỉ được coi là khép kín khi có quan hệ truy vết `triệu chứng → nguyên nhân gốc → commit sửa → regression test → bằng chứng xác minh`. Build pass hoặc benchmark pass một lần chưa đủ để đảm bảo lỗi không quay lại.

---

## 13. Cập nhật sau tích hợp Role B — Tuần 9.5

### W95-ABC-03 — Registry/Factory mới làm Harness segfault

- **Tuần:** 9.5.
- **Owner sửa:** Role B.
- **Role phụ thuộc:** A và C.
- **Ngày phát hiện:** 2026-08-05, sau khi pull commit Role B.
- **Phát hiện bởi:** build + `test_harness` + CTest.
- **Mức độ:** CRITICAL cho integration gate.
- **Độ tin cậy:** Triệu chứng `CONFIRMED`; nguyên nhân gốc `INDIRECT` cho đến khi B có patch/test.
- **Trạng thái:** `OPEN`.

#### Hiện tượng

- Dự án build đủ năm target.
- `test_multi_agent` vẫn in `ALL PASSED`.
- `test_harness` pass hai fixture đầu rồi segfault khi bắt đầu đường AgentLoop đăng ký Tool.
- CTest chỉ đạt 1/2 vì test `harness` segfault.

#### Nguyên nhân đang nghi ngờ

- `ToolRegistry::register_tool()` đọc `tool->get_name()` và `std::move(tool)` trong cùng một lời gọi.
- Thứ tự đánh giá có thể làm `tool` bị move trước khi đọc tên.
- Chưa được ghi là nguyên nhân đã xác nhận cho tới khi patch B và regression test chứng minh trước/sau.

#### Ảnh hưởng

- C-9.5-01 bị chặn; Role C chưa thể xác nhận offline integration gate.
- Role A chưa nên freeze UML/ownership Registry trước khi B chốt contract cuối.
- Không đủ điều kiện chạy benchmark provider thật của worktree hiện tại.

#### Điều kiện đóng

- B sửa đúng owner layer, không workaround trong Harness hoặc AgentLoop.
- Có focused regression test cho valid/null/duplicate registration và Factory contract.
- `test_harness`, `test_multi_agent`, CTest đều pass sau clean build.
- A/C nhận commit hash, test log và contract bàn giao.

### W95-C-04 — Gom checklist A/B/C và sửa bộ nhớ trạng thái

- **Tuần:** 9.5.
- **Role:** C quản lý kế hoạch/tài liệu tích hợp.
- **Ngày cập nhật:** 2026-08-05.
- **Trạng thái:** `CLOSED` cho thay đổi tài liệu; task integration vẫn theo W95-ABC-03.

#### Thay đổi

- Gom checklist theo từng Role vào mục 9 của `filephanchiacv/KH_TUAN9.5.md`.
- Bỏ file checklist riêng của Role C để cả nhóm theo dõi một nguồn duy nhất.
- Sửa `PROJECT_STATUS.md`: Role B `BLOCKED`, Role C `PARTIALLY DONE`, Harness/CTest phản ánh kết quả sau pull B.
- Giữ các mục W95-C-01/W95-C-02 phía trên như bằng chứng lịch sử trước regression; không dùng kết quả pass cũ để kết luận worktree mới.

#### Bằng chứng

- `filephanchiacv/KH_TUAN9.5.md` — checklist A/B/C và Definition of Done.
- `filephanchiacv/PROJECT_STATUS.md` — trạng thái ngắn hiện tại.
- `docs/report_evaluation.md` — giới hạn run lịch sử và fallback-assisted evidence.

> **Luồng cập nhật từ nay:** sau mỗi lần một Role hoàn thành task, cập nhật checkbox trong `KH_TUAN9.5.md`; khi trạng thái tổng thay đổi, cập nhật `PROJECT_STATUS.md`; khi có lỗi/fix/test quan trọng, thêm mục theo thời gian vào file lịch sử này.

---

## 14. Xác minh Role C sau commit merge mới — 2026-08-06

### W95-ABC-05 — Patch B từng có trong source nhưng merge conflict chưa được resolve

- **Owner sửa:** Role B cho `ToolRegistry.cpp`/`CMakeLists.txt`; repo maintainer cho `AGENTS.md`.
- **Role xác minh:** C.
- **Commit quan sát:** `1582f91` (`Fix merge conflicts`).
- **Mức độ:** CRITICAL cho build/integration.
- **Trạng thái:** `CLOSED` sau khi chuyển sang main snapshot sạch `e9e1d35`.

#### Bằng chứng

- Patch an toàn “lấy tên Tool trước khi move” tồn tại ở một nhánh trong `ToolRegistry.cpp`.
- `benchmark/test_tools.cpp` và target `test_tools` đã xuất hiện.
- Conflict marker vẫn còn trong `ToolRegistry.cpp`, `CMakeLists.txt`, `AGENTS.md` và ban đầu cả `KH_TUAN9.5.md`.
- `cmake -S . -B build` thất bại tại `CMakeLists.txt:74`: parser gặp token `<<<<<<<`.
- Vì configure fail, Role C không chạy binary cũ để nhận là bằng chứng cho revision hiện tại.

#### Static review bổ sung

- Test source có null, duplicate instance, create/fresh/unknown, alias, allow/deny và `register_all_tools`.
- Chưa thấy test duplicate creator dù checklist B từng ghi duplicate tổng quát.
- Header nói duplicate creator overwrite; một nhánh source lại trả `false`.
- `docs/report_tools.md` vẫn có các đoạn nói Factory/alias/policy chưa hoàn thiện, nên B-9.5-03 chưa thể ghi `DONE`.

#### Điều kiện bàn giao lại cho C

1. Không còn conflict marker trong source/build files.
2. B chốt duplicate creator semantics và đồng bộ header/source/test/report.
3. Configure + build sạch thành công.
4. C chạy `test_tools`, `test_harness`, `test_multi_agent` và CTest trên đúng revision đó.

### W95-C-06 — Cập nhật bằng chứng và tracking, không sửa source B

- **Trạng thái:** `CLOSED` cho phần audit/tài liệu; integration vẫn `BLOCKED` bởi W95-ABC-05.
- C đã dọn conflict trong checklist `KH_TUAN9.5.md` để chỉ giữ một trạng thái dựa trên bằng chứng.
- C đã cập nhật `PROJECT_STATUS.md` và `docs/report_evaluation.md` theo gate ngày 2026-08-06.
- C không chọn nhánh hoặc sửa `ToolRegistry.cpp`/`CMakeLists.txt` thay Role B.

---

## 15. Xác minh main mới và phục hồi thay đổi hiện tại — 2026-08-06

### W95-ABC-07 — Chuyển sang lịch sử main mới mà không mất WIP

- **Branch mới:** `Test-Tuan9-v2`.
- **Base:** `origin/main` tại `e9e1d35`.
- **Backup:** `backup-Test-Tuan9-20260806` và `stash@{0}` vẫn giữ đường phục hồi.
- **Lý do:** remote `main` được tạo lại thành root snapshot, không có merge-base với lịch sử branch cũ.
- **Cách xử lý:** tạo branch mới từ `origin/main`, apply lại stash và resolve đúng một conflict trong `KH_TUAN9.5.md`; không dùng `--allow-unrelated-histories` hoặc `reset --hard`.
- **Trạng thái:** `CLOSED`.

### W95-C-08 — Offline integration gate trên `e9e1d35`

- **Role xác minh:** C.
- **Trạng thái:** `CLOSED`.

#### Bằng chứng

- Configure thành công; build đầu timeout nhưng incremental build hoàn tất sáu target, exit 0.
- `test_tools`: toàn bộ focused Role B tests pass.
- `test_harness`: `ALL HARNESS TESTS PASSED`.
- `test_multi_agent`: `ALL PASSED`.
- CTest: 3/3, 100% pass.
- Không chạy `run_eval` thật; benchmark artifact cũ vẫn chỉ là historical pipeline evidence.

#### Khoảng trống còn lại

- `test_tools.cpp` chưa có duplicate-creator test và error-path tests riêng cho Exec/Git/Web/Memory.
- `docs/report_tools.md` còn hai claim Factory stale và nguồn OpenClaw/Hermes chưa có URL trực tiếp.
- A cần đồng bộ UML/OOP report; C review cuối sau khi B sửa tài liệu.
