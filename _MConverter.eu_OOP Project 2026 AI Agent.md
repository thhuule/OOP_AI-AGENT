**TRƯỜNG ĐẠI HỌC KHOA HỌC TỰ NHIÊN**

**ĐẠI HỌC QUỐC GIA TP.HCM**

**KHOA CÔNG NGHỆ THÔNG TIN**

━━━━━━━━━━━━━━━━━━━━━━━━

**ĐỒ ÁN MÔN HỌC**

**LẬP TRÌNH HƯỚNG ĐỐI TƯỢNG**

━━━━━━━━━━━━━━━━━━━━━━━━

**XÂY DỰNG AI AGENT VỚI OLLAMA API**

*Áp dụng mô hình hướng đối tượng cho hệ thống*

*Agent -- Tool -- Skill -- Harness -- Evaluator*

Năm 2026

# MỤC LỤC

I. Tổng quan đồ án 3

II\. Mục tiêu học tập 4

III\. Yêu cầu chức năng 4

IV\. Thiết kế hướng đối tượng 6

V. Kỹ thuật C++ bắt buộc sử dụng 9

VI\. Cấu trúc thư mục dự án 10

VII\. Harness Engineering & Evaluation 11

VIII\. Tiêu chí chấm điểm 12

IX\. Hướng dẫn nộp bài 14

X. Gợi ý mở rộng (điểm thưởng) 15

# I. TỔNG QUAN ĐỒ ÁN {#i.-tổng-quan-đồ-án}

Trong đồ án này, sinh viên sẽ xây dựng một **AI Agent framework** bằng C++17 trở lên kết nối với **Ollama API**, một local LLM inference server. Hệ thống mô phỏng kiến trúc của các framework agent hiện đại như OpenClaw, LangChain, LlamaIndex nhưng được thiết kế và triển khai hoàn toàn từ đầu với ngôn ngữ C++ thuần túy.

Chú ý **Google Colab** / Kaggle có thể được sử dụng để cung cấp backend hỗ trợ suy luận nhờ tài nguyên GPU miễn phí. Sinh viên có thể sử dụng mô hình **Gemma4** để có thể thực hiện tool call lẫn suy luận thị giác.

Mã nguồn mẫu sử dụng Google Colab với Gemma4 ở đây: [[https://colab.research.google.com/drive/1chSCUNnLCj4Gj4daq6Erp7h8tF0C28lE?usp=sharing]{.underline}](https://colab.research.google.com/drive/1chSCUNnLCj4Gj4daq6Erp7h8tF0C28lE?usp=sharing)

Một số giải pháp khác để có token miễn phí và model mạnh hơn là:

- NVIDIA NIM

- OpenRouter

- Google Gemini

Sinh viên lập nhóm từ **1-3** người.

Thay vì chỉ \"gọi API và in kết quả\", sinh viên phải thiết kế một **hệ thống phân lớp rõ ràng**: LLM client, Tool registry, Skill loader, Agent loop, và Harness evaluation. Mỗi tầng là một bài tập OOP độc lập nhưng kết hợp thành hệ thống hoàn chỉnh.

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>Bối cảnh kỹ thuật</strong></th>
</tr>
<tr>
<th><p>• Ollama: local LLM server, expose REST API tương thích OpenAI format</p>
<p>• Tool calling: LLM sinh text có format đặc biệt, agent parse và thực thi hàm C++</p>
<p>• Skill system: Markdown instruction files đặt vào system prompt</p>
<p>• Harness: tầng infrastructure bao quanh agent loop để đo lường và reproduce</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

# II. MỤC TIÊU HỌC TẬP {#ii.-mục-tiêu-học-tập}

Sau khi hoàn thành đồ án, sinh viên đạt được:

- **Thiết kế class hierarchy phù hợp cho hệ thống agent thực tế (không chỉ bài toán sách giáo khoa)**

- **Áp dụng tối thiểu 4 design patterns trong một hệ thống thống nhất**

- **Sử dụng thành thạo C++17 trở lên**

- **Hiểu và triển khai tầng abstraction giữa agent logic và infrastructure**

- **Viết evaluation harness có khả năng đo lường performance của LLM agent**

- **Đọc và tích hợp thư viện bên thứ ba (libcurl, nlohmann/json) vào dự án C++**

# III. YÊU CẦU CHỨC NĂNG {#iii.-yêu-cầu-chức-năng}

## 3.1 Tầng LLM Client {#tầng-llm-client}

Sinh viên xây dựng lớp OllamaClient với các yêu cầu sau:

- Gửi HTTP POST đến Ollama API endpoint /api/chat - đặc trưng của ollama (hoặc /chat/completions - tương thích openai hoặc endpoint tùy theo lựa chọn của sinh viên)

- Hỗ trợ cả text-only và multimodal (gửi ảnh base64) qua cùng một interface (nên chọn model hỗ trợ ảnh)

- Xử lý lỗi: timeout, connection refused, malformed JSON response

- Cấu hình được: base URL, model name, temperature, max_tokens

## 3.2 Tầng Tool Registry {#tầng-tool-registry}

Hệ thống tool phải đáp ứng:

- Đăng ký tool động tại runtime (không hardcode)

- Mỗi tool có: name, description (cho LLM đọc), execute function

- **Sinh viên tự triển khai tối thiểu 5 tool sau:**

  - exec: chạy lệnh shell, trả về stdout/stderr

  - read_file / write_file: đọc và ghi file

  - web_search: tìm kiếm qua SearXNG hoặc DuckDuckGo API

  - memory_save / memory_search: lưu và truy vấn memory qua SQLite

  - calculator: tính biểu thức số học

- Tool policy: có thể allow/deny theo danh sách tên

Sinh viên cần tham khảo OpenClaw hoặc Hermes để thêm vào ít nhất 3 tool thuộc 3 loại khác nhau.

## 3.3 Tầng Skill System {#tầng-skill-system}

- Load skill từ file .md trong thư mục skills/

- Skill selection: chọn skill phù hợp với task dựa trên keyword matching

- Sinh viên tự viết tối thiểu 3 skill file có nội dung thực sự hướng dẫn agent

- Skill được inject vào system prompt trước mỗi agent run

## 3.4 Agent Loop {#agent-loop}

- Vòng lặp ReAct: Observe → Think → Act → Observe\...

- Parse tool call từ LLM response (regex hoặc JSON parsing)

- Duy trì conversation history đúng format Ollama

- Giới hạn max_steps và xử lý graceful khi đạt giới hạn

## 3.5 Loop Detection {#loop-detection}

- Phát hiện ít nhất 2 loại loop: generic repeat và ping-pong

- Configurable threshold (warning vs critical)

- Khi phát hiện loop: log cảnh báo và dừng agent

## 3.6 Harness & Evaluator {#harness-evaluator}

- HarnessRunner: setup environment → run agent → evaluate → record

- Trajectory recording: lưu từng bước (thought, action, result, latency, tokens)

- Tối thiểu 2 Evaluator: KeywordEvaluator và FunctionalEvaluator

- Batch evaluation: chạy tập task, tính success rate

- Export kết quả ra JSON

# IV. THIẾT KẾ HƯỚNG ĐỐI TƯỢNG {#iv.-thiết-kế-hướng-đối-tượng}

## 4.1 Class Diagram tổng thể {#class-diagram-tổng-thể}

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>Các lớp cần thiết kế (tối thiểu)</strong></th>
</tr>
<tr>
<th><p>LLMClient (abstract) ← OllamaClient</p>
<p>Tool ← ExecTool, FileTool, WebSearchTool, MemoryTool, CalculatorTool</p>
<p>ToolRegistry (manages Tool instances)</p>
<p>SkillLoader (loads + selects SKILL.md files)</p>
<p>LoopDetector (detects agent loops)</p>
<p>AgentLoop (core ReAct loop)</p>
<p>Environment (abstract) ← NativeEnvironment, SandboxEnvironment</p>
<p>Evaluator (abstract) ← KeywordEvaluator, FunctionalEvaluator, VLMEvaluator</p>
<p>Trajectory + Step (data classes)</p>
<p>HarnessRunner (orchestrates all layers)</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

## 4.2 Design Patterns bắt buộc {#design-patterns-bắt-buộc}

| **Tiêu chí** | **Nội dung đánh giá** |
|----|----|
| **Strategy** | Evaluator hierarchy: KeywordEvaluator, FunctionalEvaluator, VLMEvaluator dùng chung interface evaluate() |
| **Template Method** | AgentLoop::run() định nghĩa skeleton, các bước observe()/act() có thể override |
| **Registry / Factory** | ToolRegistry đăng ký và tạo tool instance theo tên |
| **Observer / Hook** | HarnessRunner inject step_hook vào AgentLoop để record trajectory |

*Ghi chú:* Sinh viên có thể dùng thêm pattern khác (Decorator, Command, Builder\...) và được cộng điểm nếu áp dụng đúng chỗ tùy theo độ khó của Design Pattern.

## 4.3 Yêu cầu UML {#yêu-cầu-uml}

Sinh viên nộp các diagram sau (dùng mermaid):

1.  Class Diagram: toàn bộ hệ thống, thể hiện rõ inheritance, composition, dependency

2.  Sequence Diagram: một lần agent run hoàn chỉnh (từ lúc nhận task đến khi trả kết quả)

3.  Sequence Diagram: HarnessRunner chạy batch evaluation

4.  Component Diagram: tổng quan các module và dependency

## 4.4 Abstraction layers {#abstraction-layers}

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>Nguyên tắc thiết kế quan trọng</strong></th>
</tr>
<tr>
<th><p>AgentLoop KHÔNG biết Harness tồn tại =&gt; chỉ expose hook interface</p>
<p>Tool implementations KHÔNG phụ thuộc vào AgentLoop</p>
<p>Evaluator KHÔNG phụ thuộc vào cách agent thực thi =&gt; chỉ nhìn kết quả</p>
<p>LLMClient interface đủ generic để thay Ollama bằng OpenAI chỉ bằng 1 class mới</p>
<p>Vi phạm các nguyên tắc này bị trừ điểm thiết kế.</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

# V. KỸ THUẬT C++17 BẮT BUỘC SỬ DỤNG {#v.-kỹ-thuật-c17-bắt-buộc-sử-dụng}

Các chức năng C++17 liệt kê bên dưới chỉ cần dùng tối thiểu **4** là được.

| **Tính năng C++17** | **Ứng dụng trong đồ án** |
|----|----|
| **std::unique_ptr / shared_ptr** | Quản lý lifetime của Tool, Evaluator instances |
| **std::function + Lambda** | Tool::execute callback, step_hook trong HarnessRunner |
| **std::variant\<\...\>** | Action type: Click \| TypeText \| KeyPress \| Done |
| **std::filesystem** | SkillLoader scan thư mục skills/ |
| **if constexpr / std::visit** | Xử lý các loại Action trong agent loop |
| **Structured bindings** | Unpack JSON response fields |
| **std::optional\<T\>** | Tool result có thể rỗng, Evaluator partial score |
| **Range-based for + auto** | Duyệt tool registry, trajectory steps |
| **Abstract class / pure virtual** | LLMClient, Tool, Evaluator, Environment interfaces |
| **Template class** | Generic Registry\<T\> hoặc EventBus\<Event\> |

Ngoài ra cần dùng ít nhất **2** kĩ thuật từ C++20, **2** kĩ thuật từ C++23 và **1** kĩ thuật từ C++26.

# VI. CẤU TRÚC THƯ MỤC DỰ ÁN (GỢI Ý) {#vi.-cấu-trúc-thư-mục-dự-án-gợi-ý}

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>Cây thư mục gợi ý</strong></th>
</tr>
<tr>
<th><p>Agent_MSSV1_MSSV2_MSSV3/</p>
<p>├── CMakeLists.txt (Không có không sao)</p>
<p>├── README.md ( hướng dẫn build và chạy)</p>
<p>├── src/</p>
<p>│ ├── agent/</p>
<p>│ │ ├── agent_loop.h/.cpp</p>
<p>│ │ ├── loop_detector.h/.cpp</p>
<p>│ │ └── skill_loader.h/.cpp</p>
<p>│ ├── client/</p>
<p>│ │ ├── llm_client.h ← abstract interface</p>
<p>│ │ └── ollama_client.h/.cpp</p>
<p>│ ├── tools/</p>
<p>│ │ ├── tool.h ← abstract Tool</p>
<p>│ │ ├── tool_registry.h/.cpp</p>
<p>│ │ └── [exec, file, web, memory, calculator]_tool.cpp</p>
<p>│ └── harness/</p>
<p>│ ├── harness_runner.h/.cpp</p>
<p>│ ├── trajectory.h/.cpp</p>
<p>│ ├── evaluator.h ← abstract Evaluator</p>
<p>│ └── [keyword, functional]_evaluator.cpp</p>
<p>├── skills/</p>
<p>│ ├── task_planner.md</p>
<p>│ ├── error_recovery.md</p>
<p>│ └── [skill tự viết].md</p>
<p>├── benchmark/</p>
<p>│ ├── tasks.json</p>
<p>│ └── run_eval.cpp</p>
<p>├── tests/</p>
<p>│ └── [unit tests]</p>
<p>└── docs/</p>
<p>├── class_diagram.png</p>
<p>├── sequence_diagram_agent.png</p>
<p>└── component_diagram.png</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

Chú ý code phải được commit qua git, các thành viên chênh lệch số lượng mã nguồn không quá 20% (chỉ làm báo cáo hoặc test mà không đóng góp mã nguồn thì không tính là đóng góp đủ cho đồ án)

Số lượng commit cần tối thiểu 6 cho một thành viên (\>=12 cho nhóm 2 thành viên và \>=18 cho nhóm 3 thành viên): điều này giúp đảm bảo mã nguồn được commit là đủ nhỏ ở dạng module có thể quản lí được.

Khoảng cách thời gian giữa 2 commit gần nhất không lệch quá 7 ngày, đảm bảo mỗi tuần đều phải có ít nhất một tính năng được tạo ra.

Khi nộp nhớ tạo Personal access token (quyền read only) để giáo viên có thể truy cập vào git của các bạn để chấm bài từ một repository private.

# VII. HARNESS ENGINEERING & EVALUATION {#vii.-harness-engineering-evaluation}

## 7.1 Trajectory Format {#trajectory-format}

Mỗi lần chạy agent phải tạo ra file JSON theo format gợi ý (không nhất thiết giống hoàn toàn, đáp ứng chức năng là được)

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>trajectory_{task_id}.json</strong></th>
</tr>
<tr>
<th><p>{</p>
<p>"task_id": "task_001",</p>
<p>"model": "qwen3-vl:7b",</p>
<p>"success": true,</p>
<p>"total_tokens": 1842,</p>
<p>"total_time_ms": 12400,</p>
<p>"steps": [</p>
<p>{</p>
<p>"step_id": 0,</p>
<p>"thought": "Cần tính 15 * 17 trước...",</p>
<p>"action": {"type": "tool_call", "tool": "calculator", "args": "15*17"},</p>
<p>"tool_result": "255",</p>
<p>"tokens_used": 312,</p>
<p>"latency_ms": 890</p>
<p>}</p>
<p>]</p>
<p>}</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

## 7.2 Task Definition Format {#task-definition-format}

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>tasks.json</strong></th>
</tr>
<tr>
<th><p>[</p>
<p>{</p>
<p>"id": "task_001",</p>
<p>"description": "Tính 15 * 17 và lưu kết quả vào file result.txt",</p>
<p>"instruction": "Tính 15 nhân 17. Lưu kết quả vào file result.txt",</p>
<p>"eval_type": "functional",</p>
<p>"eval_script": "test -f result.txt &amp;&amp; grep 255 result.txt &amp;&amp; echo PASS",</p>
<p>"max_steps": 10</p>
<p>}</p>
<p>]</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

## 7.3 Benchmark yêu cầu tối thiểu {#benchmark-yêu-cầu-tối-thiểu}

Sinh viên phải cung cấp tập benchmark gồm ít nhất 10 task, phân bố:

- 4 task đơn giản: tính toán, đọc/ghi file, lấy thời gian

- 4 task trung bình: kết hợp 2-3 tool liên tiếp, có điều kiện

- 2 task khó: multi-step, agent cần tự quyết định thứ tự tool call

Báo cáo phải ghi rõ **success rate** của mô hình được chọn trên tập benchmark này.

# VIII. TIÊU CHÍ CHẤM ĐIỂM {#viii.-tiêu-chí-chấm-điểm}

<table style="width:97%;">
<colgroup>
<col style="width: 23%" />
<col style="width: 53%" />
<col style="width: 19%" />
</colgroup>
<thead>
<tr>
<th style="text-align: center;"><strong>Tiêu chí</strong></th>
<th style="text-align: center;"><strong>Nội dung đánh giá</strong></th>
<th style="text-align: center;"><strong>Điểm tối đa</strong></th>
</tr>
<tr>
<th><strong>Thiết kế OOP (25đ)</strong></th>
<th><p>• Class diagram đầy đủ, đúng notation UML (5đ)</p>
<p>• Inheritance hierarchy hợp lý, không vi phạm LSP (5đ)</p>
<p>• Áp dụng đủ 4 design patterns, đúng context (10đ)</p>
<p>• Separation of concerns: Agent không biết Harness (5đ)</p></th>
<th style="text-align: center;"><strong>25</strong></th>
</tr>
<tr>
<th><strong>Kỹ thuật C++ (20đ)</strong></th>
<th><p>• Dùng đủ tính năng C++17 trở lên trong bảng V (12đ)</p>
<p>• Memory management: không leak, dùng smart pointers (4đ)</p>
<p>• Exception handling có ý nghĩa (4đ)</p></th>
<th style="text-align: center;"><strong>20</strong></th>
</tr>
<tr>
<th><strong>Chức năng (25đ)</strong></th>
<th><p>• 5 tool hoạt động đúng (10đ)</p>
<p>• Agent loop + loop detection (5đ)</p>
<p>• Skill system load và inject đúng (5đ)</p>
<p>• Harness runner + trajectory output (5đ)</p></th>
<th style="text-align: center;"><strong>25</strong></th>
</tr>
<tr>
<th><strong>Benchmark (15đ)</strong></th>
<th><p>• 10 task hợp lệ, đa dạng (5đ)</p>
<p>• Evaluator chạy đúng, kết quả JSON hợp lệ (5đ)</p>
<p>• Báo cáo success rate có phân tích (5đ)</p></th>
<th style="text-align: center;"><strong>15</strong></th>
</tr>
<tr>
<th><strong>Tài liệu (15đ)</strong></th>
<th><p>• README: build, run, cấu hình Ollama rõ ràng (4đ)</p>
<p>• Báo cáo: mô tả thiết kế, khó khăn, kết quả (6đ)</p>
<p>• Slide thuyết trình mạch lạc (5đ)</p></th>
<th style="text-align: center;"><strong>15</strong></th>
</tr>
<tr>
<th><strong>TỔNG</strong></th>
<th></th>
<th style="text-align: center;"><strong>100</strong></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

# IX. HƯỚNG DẪN NỘP BÀI {#ix.-hướng-dẫn-nộp-bài}

## 9.1 Deadline {#deadline}

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>Lịch nộp bài</strong></th>
</tr>
<tr>
<th><p>Tuần 11 (trước 21:00 Chủ nhật): Nộp bản thiết kế: class diagram + sequence diagram</p>
<p>Tuần 12 (trước 21:00 Chủ nhật): Nộp source code + báo cáo hoàn chỉnh</p>
<p>Tuần 13: Thuyết trình + demo trực tiếp (lịch cụ thể giảng viên thông báo)</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

## 9.2 Format nộp {#format-nộp}

- Tên file ZIP: **MSSV1_MSSV2_MSSV3_OopAgent.zip** (Hoặc chỉ cần có 3 MSSV của 3 thành viên là được không cần hậu tố OopAgent)

- Trong ZIP phải có đủ cấu trúc thư mục theo mục VI

- Cần có hướng dẫn biên dịch mã nguồn

- Nộp qua hệ thống LMS của khoa - Moodle (không nhận qua email)

## 9.3 Yêu cầu demo {#yêu-cầu-demo}

Trong buổi thuyết trình, hóm sinh viên phải demo LIVE các phần sau:

1.  Khởi động Ollama từ Google Colab/Kaggle (hoặc nếu backed chạy rồi thì không cần), chạy một agent task hoàn chỉnh từ command line

2.  Thêm một tool mới (giảng viên chỉ định) vào registry và chạy lại

3.  Chạy batch benchmark, show file JSON output

4.  Giải thích một design pattern đã dùng và chỉ ra trong code

Chú ý: **với mỗi câu hỏi sinh viên trả lời sẽ được giáo viên chỉ định ngẫu nhiên. Điểm sẽ ảnh hưởng điểm của cả nhóm.**

## 9.4 Quy định về tính nguyên vẹn học thuật {#quy-định-về-tính-nguyên-vẹn-học-thuật}

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>⚠ Lưu ý quan trọng</strong></th>
</tr>
<tr>
<th><p>• Được dùng AI (ChatGPT, Claude...) để tra cứu, học khái niệm, debug</p>
<p>• KHÔNG được nộp code do AI generate toàn bộ mà không hiểu</p>
<p>• Giảng viên sẽ hỏi ngẫu nhiên về bất kỳ phần code nào trong buổi demo</p>
<p>• Không giải thích được code của mình =&gt; bị trừ điểm tương ứng phần đó</p>
<p>• Phát hiện copy từ nhóm khác =&gt; cả hai nhóm nhận điểm 0</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

# X. GỢI Ý MỞ RỘNG (Điểm thưởng tối đa +15đ) {#x.-gợi-ý-mở-rộng-điểm-thưởng-tối-đa-15đ}

Các tính năng sau không bắt buộc nhưng được cộng điểm nếu triển khai đúng và có demo:

## 10.1 GUI Agent (Screenshot + Action): +8đ {#gui-agent-screenshot-action-8đ}

- Thêm tool capture_screenshot() chụp màn hình desktop

- Dùng VLM (qwen3-vl, gemma4 qua Ollama) nhận ảnh và ra action

- Triển khai action executor: click(x,y), type_text(), key_press() qua libxdo (Linux)

- Demo: agent tự mở browser, tìm kiếm, copy kết quả

## 10.2 Persistent Memory với Vector Search: +4đ {#persistent-memory-với-vector-search-4đ}

- Thay SQLite keyword search bằng embedding-based similarity search

- Dùng nomic-embed-text (qua Ollama) để tạo embedding cho mỗi memory entry

- Tìm kiếm bằng cosine similarity trong C++

## 10.3 Multi-agent Coordination: +3đ {#multi-agent-coordination-3đ}

- HarnessRunner có thể spawn sub-agent (thread mới) cho subtask

- Agent giao tiếp qua message queue (std::queue + mutex)

- Demo: task phức tạp được phân chia cho 2 agent chạy song song

<table style="width:96%;">
<colgroup>
<col style="width: 96%" />
</colgroup>
<thead>
<tr>
<th><strong>Hướng nghiên cứu tiếp theo</strong></th>
</tr>
<tr>
<th><p>Đồ án này là thu nhỏ của các hệ thống nghiên cứu thực sự:</p>
<p>• OSWorld, ScreenSpot-Pro: benchmark GUI agent có harness đầy đủ</p>
<p>• OpenClaw: production agent framework, tool catalog phong phú</p>
<p>• UI-TARS, Agent S2: VLM-based GUI agent state-of-the-art</p>
<p>Sinh viên có thể phát triển thành đề tài nghiên cứu / luận văn trong tương lai nếu thích vì thật sự hướng nghiên cứu này hiện nay vẫn còn khá nhiều khoảng trống.</p></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

*--Hết --*
