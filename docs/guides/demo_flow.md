# HƯỚNG DẪN CHI TIẾT LUỒNG QUAY VIDEO DEMO KIỂM THỬ (DEMO VIDEO FLOW)
## ĐỒ ÁN OOP 2026: AI AGENT VỚI OLLAMA API

> **Mục tiêu:** Demo trực tiếp luồng chạy kiểm thử (testing flow) của dự án, biên dịch mã nguồn, chạy các bài unit test / ctest, phân tích các tệp kết quả (trajectory) và giải thích mã nguồn trong VS Code / Terminal.
> **Thời lượng:** 8 – 10 phút.
> **Lưu ý quan trọng:** **KHÔNG sử dụng slide PowerPoint**. Video bắt đầu trực tiếp trong môi trường IDE (VS Code) và dòng lệnh (Terminal).

---

## BẢNG PHÂN CHIA THUYẾT MINH VÀ HÌNH ẢNH

| Phân đoạn | Thời gian | Người nói | Visuals (Màn hình hiển thị) | Nội dung thuyết minh (Voiceover) |
| :--- | :--- | :--- | :--- | :--- |
| **Phần 1** | 00:00 - 01:30 | **Thành viên A** | File `README.md` & các biểu đồ UML Mermaid trên VS Code | Giới thiệu dự án, vai trò các thành viên và phân tích Class/Sequence Diagram. |
| **Phần 2** | 01:30 - 02:15 | **Thành viên B** | Cửa sổ Terminal gõ các lệnh CMake build dự án | Demo quá trình dọn dẹp và biên dịch mã nguồn C++ từ đầu. |
| **Phần 3** | 02:15 - 03:15 | **Thành viên A** | VS Code mở file `agent_loop.h`/`.cpp` và `MultiAgentRunner.h` | Giải thích cấu trúc code ReAct, Template Method và tính năng C++26. |
| **Phần 4** | 03:15 - 04:00 | **Thành viên B** | VS Code mở `ToolRegistry.cpp` và `MemoryTool.cpp` | Giải thích Registry/Factory Pattern của Tools và cơ chế Vector Search. |
| **Phần 5** | 04:00 - 06:15 | **Thành viên C** | VS Code mở `tasks.json` & Terminal chạy `ctest` và các file test | Giải thích Strategy Pattern của Evaluator, chạy CTest và các bộ unit test. |
| **Phần 6** | 06:15 - 08:20 | **Thành viên C** | VS Code mở thư mục chạy `runs/` và các tệp tin `trajectory.json` | Chạy benchmark, bóc tách phân tích chi tiết Task 005 và Task 010. |
| **Phần 7** | 08:20 - 09:30 | **Thành viên C** | VS Code mở báo cáo đánh giá & file chứa phần Hạn chế | So sánh kết quả, trình bày hạn chế kỹ thuật và kết luận buổi demo. |

---

## KỊCH BẢN THUYẾT MINH CHI TIẾT TỪNG PHÂN ĐOẠN

### PHẦN 1: GIỚI THIỆU DỰ ÁN & PHÂN TÍCH THIẾT KẾ (00:00 – 01:30)
* **Người thực hiện:** **Thành viên A (Core/Systems)**

#### 🎥 **Visuals (Màn hình):**
* Mở VS Code tại thư mục dự án, hiển thị tệp tin [README.md](file:///home/thhuule/projects/OOP_AI-AGENT/README.md).
* Mở tệp tin sơ đồ thiết kế UML dạng Mermaid: [class_diagram.md](file:///home/thhuule/projects/OOP_AI-AGENT/docs/diagrams/class_diagram.md) và [sequence_agent_run.md](file:///home/thhuule/projects/OOP_AI-AGENT/docs/diagrams/sequence_agent_run.md). Dùng chuột trỏ vào các lớp `AgentLoop`, `Tool`, `LLMClient` và `Environment`.

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Xin chào thầy cô và các bạn. Hôm nay nhóm chúng em xin trình bày video demo kiểm thử thực tế cho dự án 'C++ AI Agent Framework'.
> 
> Bắt đầu tại file `README.md` của dự án, nhóm chúng em gồm 3 thành viên đã triển khai cấu trúc thư mục phân lớp rõ ràng. Em là **Thành viên A** chịu trách nhiệm Core Agent; **Thành viên B** phụ trách Tools và Data Memory; **Thành viên C** phụ trách Benchmark Harness và Multi-Agent.
> 
> Nhìn vào sơ đồ Class Diagram, triết lý thiết kế của nhóm là **Decoupling (Giảm liên kết phụ thuộc)**. Lớp `AgentLoop` đóng vai trò điều khiển ReAct Loop nhưng hoàn toàn độc lập với các công cụ cụ thể (`Tool`) nhờ interface trừu tượng. Sự kết nối giữa các tầng là một chiều: Harness gọi Agent Core, Agent Core gọi Tools, và Tools giao tiếp với hệ điều hành thông qua lớp trừu tượng `Environment`.
> 
> Sơ đồ Sequence Diagram thể hiện luồng chạy: Agent nhận yêu cầu, tư duy thông qua `think_and_act()`, gọi công cụ và ghi nhận quan sát (`observe()`) mà không hề phụ thuộc cứng vào bộ đánh giá Harness nhờ cơ chế StepHook."*

---

### PHẦN 2: QUY TRÌNH BIÊN DỊCH & SETUP MÔI TRƯỜNG (01:30 – 02:15)
* **Người thực hiện:** **Thành viên B (Tools/Data)**

#### 🎥 **Visuals (Màn hình):**
* Mở cửa sổ Terminal (Linux/WSL) chia đôi màn hình bên cạnh VS Code.
* Gõ các lệnh dọn dẹp thư mục build cũ (nếu có) và bắt đầu cấu hình, biên dịch:
  ```bash
  rm -rf build/
  cmake -S . -B build
  cmake --build build -j2
  ```
* Cuộn chuột hiển thị quá trình compiler biên dịch các file nguồn `.cpp` thành công mà không sinh bất kỳ lỗi hay cảnh báo (clean compile).

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Em là **Thành viên B**. Sau đây em xin trình bày quy trình thiết lập môi trường và biên dịch mã nguồn dự án.
> 
> Dự án của chúng em sử dụng hệ thống build CMake để quản lý các mục tiêu biên dịch. Em sẽ chạy lệnh `cmake -S . -B build` để cấu hình thư mục build và kiểm tra các thư viện phụ thuộc như `libcurl` và `sqlite3`. 
> 
> Tiếp theo, em chạy lệnh `cmake --build build -j2` để bắt đầu biên dịch toàn bộ các file nguồn C++. Như thầy cô thấy trên màn hình, mã nguồn biên dịch rất sạch, không sinh ra bất kỳ lỗi cú pháp hay warning nào từ compiler, và đã xuất ra thành công các file thực thi kiểm thử trong thư mục `build/`."*

---

### PHẦN 3: DUYỆT CODE CORE & TÍNH NĂNG C++26 (02:15 – 03:15)
* **Người thực hiện:** **Thành viên A (Core/Systems)**

#### 🎥 **Visuals (Màn hình):**
* Quay lại VS Code, mở file [agent_loop.h](file:///home/thhuule/projects/OOP_AI-AGENT/src/agent/agent_loop.h#L90-L118) và chỉ vào các dòng khai báo phương thức `run()` phi ảo cùng các phương thức ảo `build_system_prompt()`, `think_and_act()`, `execute_tool()`.
* Tiếp tục mở file [MultiAgentRunner.h](file:///home/thhuule/projects/OOP_AI-AGENT/src/multiagent/MultiAgentRunner.h#L30-L35), bôi đen dòng lệnh sử dụng tính năng `= delete` có ghi kèm lý do.

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Tiếp theo, em xin giải thích chi tiết thiết kế nhân Agent Core trong file `agent_loop.h`. Chúng em áp dụng **Template Method Pattern**. Phương thức `run()` định nghĩa khung xương cố định của thuật toán ReAct và được khai báo phi ảo để ngăn cấm ghi đè cấu trúc thuật toán. Các phương thức nhỏ bên dưới như `build_system_prompt()`, `think_and_act()` và `execute_tool()` là các primitives ảo được bảo vệ (`protected virtual`), cho phép các lớp con kế thừa thay đổi chi tiết thực thi hoặc phục vụ viết Mock Agent cho Unit Test.
> 
> Để tối ưu hóa an toàn quản lý luồng trong Multi-Agent, đây là file `MultiAgentRunner.h`. Chúng em sử dụng tính năng Modern C++26 mới nhất là **Deleted function với lý do cụ thể**. Khai báo `MultiAgentRunner(const MultiAgentRunner&) = delete("MultiAgentRunner owns worker threads and is non-copyable")` giúp compiler hiển thị trực tiếp thông báo lỗi đi kèm lý do giải thích rõ ràng nếu lập trình viên vô tình sao chép đối tượng quản lý thread này."*

---

### PHẦN 4: DUYỆT CODE ĐĂNG KÝ CÔNG CỤ & BỘ NHỚ VECTOR (03:15 – 04:00)
* **Người thực hiện:** **Thành viên B (Tools/Data)**

#### 🎥 **Visuals (Màn hình):**
* Mở file [ToolRegistry.cpp](file:///home/thhuule/projects/OOP_AI-AGENT/src/tools/ToolRegistry.cpp#L157-L210) trong VS Code, chỉ vào các dòng `register_creator` sử dụng lambda.
* Mở file [MemoryTool.cpp](file:///home/thhuule/projects/OOP_AI-AGENT/src/tools/MemoryTool.cpp) chỉ vào đoạn code SQLite khởi tạo DB và hàm tính toán Cosine Similarity.

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Ở tầng Tools và Data, đây là file `ToolRegistry.cpp`. Chúng em sử dụng **Registry & Factory Pattern** kết hợp. Các công cụ cụ thể được đăng ký động thông qua các hàm creator sử dụng biểu thức Lambda. Cơ chế Registry cũng hỗ trợ chuẩn hóa tên viết tắt qua Alias (như map `exec` về `execute_shell`) và bộ lọc bảo mật Allow/Deny List.
> 
> Về phần lưu trữ ký ức dài hạn, trong `MemoryTool.cpp`, chúng em sử dụng thư viện SQLite3 để quản lý tệp dữ liệu `memory.db` theo nguyên lý RAII. Khi lưu ký ức, Agent gọi Ollama API sinh vector embedding ngữ nghĩa. 
> 
> Còn đây là hàm tính toán khoảng cách **Cosine Similarity** được nhóm tự phát triển hoàn toàn bằng C++ để so sánh các vector ký ức, xếp hạng và trả về những thông tin có độ tương đồng ngữ nghĩa cao nhất mà không cần phụ thuộc vào thư viện bên ngoài."*

---

### PHẦN 5: BỘ NHIỆM VỤ BENCHMARK & CHẠY UNIT TESTS (04:00 – 06:15)
* **Người thực hiện:** **Thành viên C (Eval/Infra)**

#### 🎥 **Visuals (Màn hình):**
* Mở file [tasks.json](file:///home/thhuule/projects/OOP_AI-AGENT/benchmark/tasks.json) trong VS Code hiển thị cấu trúc của 10 nhiệm vụ.
* Chỉ vào file [evaluator.h](file:///home/thhuule/projects/OOP_AI-AGENT/src/harness/evaluator.h) hiển thị giao diện thuần ảo `Evaluator`, sau đó mở [KeywordEvaluator.cpp](file:///home/thhuule/projects/OOP_AI-AGENT/src/harness/KeywordEvaluator.cpp) và [FunctionalEvaluator.cpp](file:///home/thhuule/projects/OOP_AI-AGENT/src/harness/FunctionalEvaluator.cpp).
* Mở Terminal chạy kiểm thử tự động:
  ```bash
  ./build/test_tools
  ctest --test-dir build --output-on-failure
  ```
  *(Đợi terminal quét qua toàn bộ 5 test targets và hiển thị thông báo `100% tests passed`)*.

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Em là **Thành viên C**. Em xin trình bày về hạ tầng đánh giá. Đây là file `tasks.json` chứa 10 nhiệm vụ kiểm thử của Agent được phân bổ theo độ khó: 4 dễ, 4 trung bình và 2 khó. 
> 
> Chúng em áp dụng **Strategy Pattern** cho bộ chấm điểm: giao diện `Evaluator` định nghĩa phương thức chấm điểm thuần ảo, các lớp con `KeywordEvaluator` chấm theo từ khóa và `FunctionalEvaluator` chấm bằng cách thực thi tệp tin kiểm thử thực tế.
> 
> Để kiểm chứng tính ổn định của code, em chạy file test tập trung `./build/test_tools` để xác nhận 21 Unit Test của các Tools hoạt động chính xác. Tiếp theo, em chạy lệnh `ctest` trên toàn bộ dự án. Kết quả là 100% bài test bao gồm harness, multi_agent, tools, template_method và role_a đều vượt qua hoàn hảo."*

---

### PHẦN 6: CHẠY BENCHMARK THỰC TẾ & PHÂN TÍCH TRAJECTORY (06:15 – 08:20)
* **Người thực hiện:** **Thành viên C (Eval/Infra)**

#### 🎥 **Visuals (Màn hình):**
* Mở Terminal, chạy lệnh thực thi benchmark:
  ```bash
  ./build/harness
  ```
* Mở thư mục kết quả chạy gần nhất trong `build/runs/run_...` trên VS Code.
* **Task 005:** Mở file `trajectory.json` của task 005. Trỏ chuột vào bước Agent gọi `calculator` với tham số `"47 * 23"` nhận về kết quả `"1081"`, sau đó gọi công cụ `write_file` lưu `"1081"` vào tệp tin `result.txt`. Mở tệp tin `result.txt` đã được tạo ra trong thư mục để chứng minh file được tạo thực tế.
* **Task 010:** Mở file `trajectory.json` của task 010. Chỉ ra hai bước gọi liên tiếp là `write_file` để tạo file ban đầu, và `append_file` để thêm nội dung văn bản.

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Bây giờ, em thực thi bộ đo lường chính thức bằng lệnh `./build/harness`. Luồng chạy sẽ tự động dọn dẹp các tệp tin cũ để đảm bảo tính lặp lại và thực thi lần lượt các nhiệm vụ.
> 
> Hãy cùng mở tệp tin kết quả `trajectory.json` của **Task 005** để phân tích chi tiết. Ở bước đầu tiên, Agent nhận diện cần tính toán biểu thức nên đã gọi công cụ `calculator` với đối số `"47 * 23"` và nhận về đáp án `"1081"`. Bước tiếp theo, Agent gọi công cụ `write_file` để ghi giá trị `"1081"` vào tệp tin `result.txt`. Em sẽ mở tệp tin `result.txt` này trong workspace – tệp tin đã được tạo ra đúng vị trí với nội dung chính xác.
> 
> Tương tự ở **Task 010**, lịch sử lưu vết cho thấy Agent đã gọi công cụ `write_file` để tạo tệp tin thô, sau đó gọi tiếp `append_file` để nối thêm nội dung mới. Kết quả của cả hai bài kiểm thử đều được FunctionalEvaluator xác nhận là thành công và chấm điểm PASS."*

---

### PHẦN 7: SO SÁNH KẾT QUẢ, HẠN CHẾ & KẾT LUẬN (08:20 – 09:30)
* **Người thực hiện:** **Thành viên C (Eval/Infra)**

#### 🎥 **Visuals (Màn hình):**
* Mở file báo cáo kết quả đánh giá [report_evaluation.md](file:///home/thhuule/projects/OOP_AI-AGENT/docs/reports/report_evaluation.md) trong VS Code. Cuộn chuột hiển thị phần bảng so sánh tỉ lệ thành công (Success Rate 70%) và chỉ vào phần Hạn chế (Limitations).

#### 🎙️ **Voiceover (Lời thuyết minh):**
> *"Nhìn vào bảng so sánh kết quả trong báo cáo, nhờ việc tối ưu hóa bộ parser dấu ngoặc cân bằng giúp giải quyết triệt để lỗi phân tích cú pháp JSON và tích hợp bộ phát hiện vòng lặp vô hạn Loop Detector, tỉ lệ thành công của Agent đã nâng lên mức 70%.
> 
> Tuy nhiên, hệ thống vẫn tồn tại một số hạn chế kỹ thuật: Việc đo lường token tiêu thụ của LLM hiện tại chưa có bộ đếm token chính xác do giới hạn API Ollama. Module Multi-Agent cũng chưa được tích hợp trực tiếp vào quy trình tự động chấm điểm của Harness, và công cụ VLM hiện mới chỉ dừng lại ở mức khung xương interface.
> 
> Tóm lại, thông qua video demo kiểm thử này, nhóm chúng em đã chứng minh được toàn bộ mã nguồn của dự án hoạt động ổn định, đúng chuẩn hướng đối tượng C++26, tài nguyên được quản lý an toàn và có thể đánh giá tự động thông qua hệ thống Benchmark Harness.
> 
> Chúng em xin kết thúc video demo kiểm thử tại đây. Cảm ơn thầy cô đã lắng nghe!"*

---

## CHECKLIST TRƯỚC KHI XUẤT VIDEO
- [ ] Độ phân giải màn hình quay tối thiểu 1080p, Terminal và Code trong VS Code rõ nét.
- [ ] Lời thuyết minh to, rõ ràng, không bị rè tiếng, không bị lẫn tạp âm bên ngoài.
- [ ] Toàn bộ các file unit test (`test_tools`, `test_harness`, v.v.) và CTest đều đạt trạng thái **PASSED 100%**.
- [ ] Không hiển thị bất kỳ API key, token bí mật hay file `config.json` nào lên màn hình.
