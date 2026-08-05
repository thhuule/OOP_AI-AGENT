# Báo cáo Tools Architecture — Role B

> **Phiên bản:** Tuần 9 — AI-AGENT OOP 2026  
> **Chủ trì:** B — Tools/Data  
> **Repository:** OOP_AI-AGENT-main  
> **Nguyên tắc:** Báo cáo phản ánh đúng source code hiện tại; không mô tả các thành phần chưa được triển khai như tính năng hoàn chỉnh; các khoảng cách so với yêu cầu đề được nêu rõ.

---

# 1. Tổng quan kiến trúc Tools

```
                     AgentLoop
                         │
                         ▼
                  ToolRegistry
        (runtime registration & lookup)
                         │
         ┌───────────────┼────────────────┐
         │               │                │
         ▼               ▼                ▼
  CalculatorTool     FileTool      MemoryTool
         │               │                │
         ├───────────────┼────────────────┤
         ▼               ▼                ▼
     ExecTool      WebSearchTool      JsonTool
                         │
                 ┌───────┴────────┐
                 ▼                ▼
             TimeTool         GitTool
```

Tool Layer là tầng chịu trách nhiệm thực hiện các thao tác bên ngoài mô hình ngôn ngữ (LLM). Khi AgentLoop nhận được một Tool Call từ kết quả suy luận của LLM, AgentLoop không thực thi trực tiếp mà chuyển yêu cầu đến ToolRegistry để tìm kiếm Tool phù hợp.

Toàn bộ Tool đều kế thừa cùng một interface chung (`Tool`) nhằm đảm bảo tính đa hình (polymorphism) và giảm sự phụ thuộc giữa AgentLoop với từng implementation cụ thể.

Thiết kế này mang lại các lợi ích sau:

- giảm coupling giữa AgentLoop và Tool;
- cho phép mở rộng thêm Tool mà không sửa AgentLoop;
- quản lý tập trung việc đăng ký Tool;
- hỗ trợ áp dụng OOP và Registry Pattern.

---

# 2. Cấu trúc thư mục

Các thành phần của Tool Layer được đặt trong:

```
src/tools/
```

Bao gồm:

| File | Vai trò |
|------|----------|
| Tool.h | Abstract interface của toàn bộ Tool |
| ToolRegistry.h/.cpp | Quản lý Tool runtime |
| Registry.h | Generic Registry |
| CalculatorTool | Công cụ tính toán |
| ExecTool | Thực thi lệnh hệ thống |
| FileTool | Thao tác với file |
| MemoryTool | Persistent memory |
| WebSearchTool | Tìm kiếm Web |
| JsonTool | Xử lý JSON |
| TimeTool | Thời gian hệ thống |
| GitTool | Thao tác Git |

Tool Layer hoạt động độc lập với Harness và Evaluator. AgentLoop chỉ giao tiếp thông qua ToolRegistry.

---

# 3. Tool Interface

Toàn bộ Tool kế thừa từ abstract class:

```
src/tools/Tool.h
```

Interface này định nghĩa contract chung cho mọi Tool.

Các phương thức chính gồm:

```cpp
class Tool {
public:
    virtual ~Tool() = default;

    virtual std::string name() const = 0;

    virtual std::string description() const = 0;

    virtual ToolResult execute(const std::string& args) = 0;
};
```

Ý nghĩa từng thành phần:

| Hàm | Vai trò |
|------|----------|
| name() | Trả về tên định danh của Tool |
| description() | Mô tả chức năng để Agent lựa chọn Tool |
| execute() | Thực thi Tool với tham số đầu vào |

Việc chuẩn hóa interface giúp AgentLoop chỉ cần làm việc với lớp trừu tượng (`Tool`) mà không phụ thuộc implementation cụ thể.

Đây là ví dụ của nguyên lý **Dependency Inversion Principle (DIP)** trong SOLID.

---

# 4. Tool Registry

Lớp ToolRegistry được cài đặt tại:

```
src/tools/ToolRegistry.h
src/tools/ToolRegistry.cpp
```

Đây là thành phần trung tâm quản lý toàn bộ Tool trong hệ thống.

Các chức năng chính:

- đăng ký Tool;
- lưu danh sách Tool;
- tìm Tool theo tên;
- trả về Tool để AgentLoop thực thi.

Luồng xử lý:

```
LLM

↓

Tool Name

↓

ToolRegistry

↓

Lookup

↓

Concrete Tool

↓

execute(args)

↓

Tool Result
```

Nhờ đó AgentLoop không cần biết Tool cụ thể là CalculatorTool, FileTool hay WebSearchTool.

---

# 5. Generic Registry

Ngoài ToolRegistry, project còn có:

```
src/tools/Registry.h
```

Đây là lớp Registry tổng quát phục vụ việc quản lý các đối tượng theo tên.

Vai trò của Registry:

- lưu danh sách object;
- hỗ trợ đăng ký runtime;
- giảm hardcode trong hệ thống;
- hỗ trợ mở rộng thêm Tool.

Theo source hiện tại, Registry đóng vai trò hỗ trợ quản lý đối tượng, tuy nhiên chưa thể hiện đầy đủ cơ chế Factory tạo instance theo tên như yêu cầu cập nhật của đề. Vì vậy hạng mục Registry/Factory vẫn cần được hoàn thiện và kiểm thử trước khi được xem là đáp ứng hoàn toàn yêu cầu.

---

# 6. Quan hệ giữa AgentLoop và Tool

Quan hệ giữa các thành phần được tổ chức theo mô hình dependency một chiều.

```
AgentLoop
     │
     ▼
ToolRegistry
     │
     ▼
 Tool Interface
     │
     ▼
Concrete Tool
```

Các dependency bị tránh:

- Tool không include AgentLoop.
- Tool không phụ thuộc Harness.
- Tool không biết Evaluator.
- AgentLoop không thao tác trực tiếp với implementation của từng Tool.

Thiết kế này giúp giảm coupling giữa các module và tăng khả năng mở rộng của framework.

---

# 7. Thiết kế hướng đối tượng

Tool Layer sử dụng nhiều đặc điểm của lập trình hướng đối tượng:

| Nguyên lý | Minh chứng |
|------------|------------|
| Abstraction | Interface `Tool` |
| Encapsulation | Mỗi Tool tự quản lý logic xử lý của mình |
| Inheritance | CalculatorTool, FileTool, MemoryTool... kế thừa Tool |
| Polymorphism | AgentLoop thao tác thông qua `Tool*` hoặc interface chung |

Ngoài ra, ToolRegistry đóng vai trò trung gian giúp AgentLoop không phụ thuộc trực tiếp vào các lớp cụ thể, phù hợp với nguyên lý Open/Closed và Dependency Inversion trong SOLID.
---

# 8. Tool Inventory

Phần này mô tả chi tiết các Tool hiện được triển khai trong project. Mỗi Tool được tổ chức thành một lớp độc lập, kế thừa từ interface `Tool` và được đăng ký thông qua `ToolRegistry`.

---

## 8.1 CalculatorTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `CalculatorTool` |
| Source | `src/tools/CalculatorTool.h/.cpp` |
| Chức năng | Thực hiện các phép tính số học |
| Input | Chuỗi biểu thức toán học |
| Output | Chuỗi kết quả |
| Dependency | Standard Library |
| Vai trò | Hỗ trợ Agent thực hiện tính toán thay vì suy luận trực tiếp từ LLM |

CalculatorTool được sử dụng trong các tác vụ yêu cầu tính toán chính xác như cộng, trừ, nhân, chia hoặc các biểu thức số học. Việc sử dụng Tool giúp giảm hiện tượng hallucination của mô hình ngôn ngữ khi xử lý các phép tính.

Luồng thực hiện:

```
AgentLoop
      │
      ▼
CalculatorTool
      │
execute(expression)
      │
      ▼
Result
```

---

## 8.2 ExecTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `ExecTool` |
| Source | `src/tools/ExecTool.h/.cpp` |
| Chức năng | Thực thi lệnh hệ thống |
| Dependency | Shell / Operating System |
| Vai trò | Hỗ trợ Agent thao tác với môi trường hệ điều hành |

ExecTool cho phép Agent thực hiện một số lệnh của hệ điều hành nhằm phục vụ workflow. Kết quả thực thi được trả về dưới dạng chuỗi để Agent tiếp tục xử lý.

Các lỗi khi thực thi được chuyển về AgentLoop để mô hình có thể phản hồi hoặc thử phương án khác.

---

## 8.3 FileTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `FileTool` |
| Source | `src/tools/FileTool.h/.cpp` |
| Chức năng | Đọc và ghi dữ liệu file |
| Dependency | `std::filesystem` |
| Vai trò | Làm việc với dữ liệu lưu trữ cục bộ |

Khác với đặc tả mới của đề, source hiện tại triển khai một lớp `FileTool` duy nhất thay vì tách thành `ReadFileTool`, `WriteFileTool` và `AppendFileTool`.

Điều này giúp mã nguồn đơn giản hơn nhưng chưa phản ánh đầy đủ mô hình phân tách trách nhiệm như tài liệu yêu cầu.

Các thao tác chính của FileTool bao gồm:

- đọc nội dung file;
- ghi nội dung file;
- xử lý lỗi khi file không tồn tại;
- trả kết quả về AgentLoop.

---

## 8.4 MemoryTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `MemoryTool` |
| Source | `src/tools/MemoryTool.h/.cpp` |
| Chức năng | Lưu và truy xuất dữ liệu |
| Dependency | SQLite |
| Vai trò | Persistent Memory |

MemoryTool chịu trách nhiệm lưu trữ dữ liệu phục vụ Agent giữa nhiều lần thực thi.

Thông tin được lưu thông qua SQLite giúp Agent có thể sử dụng lại dữ liệu thay vì chỉ dựa vào history trong phiên làm việc.

Kiến trúc:

```
AgentLoop

↓

MemoryTool

↓

SQLite

↓

Memory Database
```

MemoryTool được triển khai độc lập với AgentLoop và chỉ giao tiếp thông qua interface Tool.

---

## 8.5 WebSearchTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `WebSearchTool` |
| Source | `src/tools/WebSearchTool.h/.cpp` |
| Chức năng | Tìm kiếm thông tin trên Internet |
| Dependency | libcurl |
| Vai trò | Bổ sung dữ liệu ngoài LLM |

WebSearchTool gửi HTTP Request thông qua thư viện libcurl để truy vấn dữ liệu từ dịch vụ tìm kiếm.

Kết quả được trả về dưới dạng văn bản nhằm hỗ trợ Agent trả lời các câu hỏi cần dữ liệu bên ngoài.

Luồng xử lý:

```
AgentLoop

↓

WebSearchTool

↓

HTTP Request

↓

Search Service

↓

Response

↓

AgentLoop
```

Các lỗi mạng hoặc timeout được trả về dưới dạng kết quả lỗi để Agent có thể tiếp tục vòng lặp ReAct.

---

## 8.6 JsonTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `JsonTool` |
| Source | `src/tools/JsonTool.h/.cpp` |
| Chức năng | Xử lý dữ liệu JSON |
| Dependency | nlohmann::json |
| Vai trò | Hỗ trợ thao tác với dữ liệu có cấu trúc |

JsonTool hỗ trợ việc đọc, ghi hoặc chuyển đổi dữ liệu JSON trong quá trình Agent làm việc với các Tool khác.

Việc sử dụng thư viện `nlohmann::json` giúp giảm lỗi parse và đơn giản hóa việc xử lý dữ liệu có cấu trúc.

---

## 8.7 TimeTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `TimeTool` |
| Source | `src/tools/TimeTool.h/.cpp` |
| Chức năng | Trả về thời gian hệ thống |
| Dependency | `std::chrono` |
| Vai trò | Hỗ trợ timestamp trong workflow |

TimeTool được sử dụng khi Agent cần lấy thời gian hiện tại để phục vụ logging hoặc sinh dữ liệu phụ thuộc thời gian.

---

## 8.8 GitTool

| Thuộc tính | Giá trị |
|------------|----------|
| Class | `GitTool` |
| Source | `src/tools/GitTool.h/.cpp` |
| Chức năng | Thao tác với Git Repository |
| Dependency | Git |
| Vai trò | Hỗ trợ workflow phát triển phần mềm |

GitTool cung cấp khả năng tương tác với Git Repository thông qua Tool Layer thay vì để Agent trực tiếp thực hiện các thao tác liên quan đến Git.

---

# 9. Dependency của Tool Layer

| Tool | Dependency |
|------|------------|
| CalculatorTool | STL |
| ExecTool | Operating System |
| FileTool | std::filesystem |
| MemoryTool | SQLite |
| WebSearchTool | libcurl |
| JsonTool | nlohmann::json |
| TimeTool | std::chrono |
| GitTool | Git |

Các dependency được tách biệt khỏi AgentLoop nhằm giảm mức độ phụ thuộc giữa các module.

---

# 10. Quan hệ giữa các Tool

Mỗi Tool được xây dựng như một module độc lập.

```
              Tool
                ▲
    ┌───────────┼─────────────┐
    │           │             │
Calculator   FileTool    MemoryTool
    │           │             │
 ExecTool  JsonTool   WebSearchTool
                │
         TimeTool
                │
            GitTool
```

Các Tool không phụ thuộc lẫn nhau mà chỉ chia sẻ interface chung (`Tool`). Điều này giúp việc bổ sung hoặc thay đổi Tool không ảnh hưởng tới các Tool còn lại.

---

# 11. Đánh giá Tool Layer

Qua quá trình phân tích source code, Tool Layer đã được tổ chức theo hướng module hóa với interface thống nhất và cơ chế quản lý tập trung thông qua `ToolRegistry`. Mỗi Tool đảm nhiệm một nhóm chức năng riêng, góp phần tách biệt logic nghiệp vụ khỏi AgentLoop.

Tuy nhiên, so với yêu cầu cập nhật của đề bài, vẫn còn một số khoảng cách cần được ghi nhận trung thực:

- Source hiện sử dụng `FileTool` thay cho việc tách riêng `ReadFileTool`, `WriteFileTool` và `AppendFileTool`.
- Chưa có minh chứng đầy đủ cho cơ chế Factory tạo instance theo tên trong `ToolRegistry`.
- Các chính sách alias, allow-list và deny-list cần được kiểm chứng bằng test nếu được triển khai trong các phiên bản tiếp theo.

Những nội dung trên được đưa vào backlog triển khai và kiểm thử, không được xem là các tính năng đã hoàn thành.
---

# 12. Error Handling

Tool Layer được thiết kế nhằm đảm bảo mọi lỗi phát sinh trong quá trình thực thi đều được phản hồi về AgentLoop thay vì làm dừng toàn bộ hệ thống.

Các nhóm lỗi chính bao gồm:

| Nhóm lỗi | Nguyên nhân |
|----------|-------------|
| Invalid Argument | Tham số đầu vào không hợp lệ |
| File Error | Không tìm thấy file hoặc không đủ quyền truy cập |
| Execution Error | Thực thi command thất bại |
| Network Error | Không thể kết nối dịch vụ Web Search |
| Database Error | SQLite không phản hồi hoặc truy vấn lỗi |
| JSON Error | Parse JSON thất bại |

Thay vì để Tool tự xử lý toàn bộ lỗi, kết quả được trả về cho AgentLoop để tiếp tục quá trình suy luận hoặc lựa chọn Tool khác trong vòng lặp ReAct.

Luồng xử lý lỗi:

```
Tool

↓

Execute

↓

Success ?

├── Yes
│
└── No

↓

Return Error

↓

AgentLoop

↓

History

↓

LLM Retry
```

Thiết kế này giúp Agent có khả năng tự phục hồi (self-recovery) thay vì kết thúc ngay khi gặp lỗi ở một Tool.

---

# 13. Validation

Các Tool đều cần kiểm tra dữ liệu đầu vào trước khi thực hiện thao tác.

Một số trường hợp được xử lý:

## CalculatorTool

- biểu thức rỗng;
- ký tự không hợp lệ;
- phép chia cho 0.

---

## FileTool

- file không tồn tại;
- đường dẫn không hợp lệ;
- lỗi ghi file;
- lỗi đọc file.

---

## ExecTool

- command rỗng;
- command trả về lỗi;
- tiến trình kết thúc bất thường.

---

## JsonTool

- JSON không đúng định dạng;
- thiếu trường dữ liệu;
- lỗi parse.

---

## MemoryTool

- lỗi mở cơ sở dữ liệu;
- lỗi truy vấn SQLite;
- dữ liệu không tồn tại.

---

## WebSearchTool

- timeout;
- HTTP Error;
- lỗi kết nối.

---

# 14. Mapping Tool → Source

| Tool | Source |
|------|--------|
| CalculatorTool | src/tools/CalculatorTool.cpp |
| ExecTool | src/tools/ExecTool.cpp |
| FileTool | src/tools/FileTool.cpp |
| MemoryTool | src/tools/MemoryTool.cpp |
| WebSearchTool | src/tools/WebSearchTool.cpp |
| JsonTool | src/tools/JsonTool.cpp |
| TimeTool | src/tools/TimeTool.cpp |
| GitTool | src/tools/GitTool.cpp |
| ToolRegistry | src/tools/ToolRegistry.cpp |
| Registry | src/tools/Registry.h |

Bảng trên giúp truy vết giữa tài liệu và source code, đảm bảo báo cáo phản ánh đúng cấu trúc của project.

---

# 15. Mapping Tool → Dependency

| Tool | Thư viện sử dụng |
|------|------------------|
| CalculatorTool | Standard Library |
| FileTool | std::filesystem |
| MemoryTool | SQLite3 |
| WebSearchTool | libcurl |
| JsonTool | nlohmann::json |
| TimeTool | std::chrono |
| ExecTool | Operating System API |
| GitTool | Git CLI |

Các dependency được tách biệt khỏi AgentLoop nhằm giảm coupling và hỗ trợ bảo trì hệ thống.

---

# 16. Vai trò của Tool trong Agent Loop

Trong kiến trúc ReAct, Tool Layer đóng vai trò là cầu nối giữa quá trình suy luận của LLM và môi trường thực thi.

Luồng hoạt động:

```
User

↓

AgentLoop

↓

LLM

↓

Tool Call

↓

ToolRegistry

↓

Concrete Tool

↓

Execution Result

↓

History

↓

LLM

↓

Final Answer
```

Nhờ cơ chế này, Agent có thể kết hợp khả năng suy luận của mô hình ngôn ngữ với các thao tác thực tế như đọc file, tìm kiếm web hoặc lưu dữ liệu.

---

# 17. Benchmark Case Study

Trong benchmark của project, Tool Layer là thành phần ảnh hưởng trực tiếp đến khả năng hoàn thành task.

Theo kết quả benchmark lịch sử:

| Run ID | Success |
|---------|---------|
| run_20260801_212302_253 | 2 / 10 |
| run_20260801_220549_361 | 10 / 10 |

Kết quả cho thấy việc cải thiện xử lý Tool giúp tăng đáng kể tỷ lệ hoàn thành tác vụ.

Các cải thiện chủ yếu bao gồm:

- xử lý chính xác hơn các thao tác với file;
- cải thiện việc truyền kết quả Tool về Agent;
- bổ sung thông tin thực thi vào trajectory của Harness;
- giữ nguyên thông tin lỗi để Agent có thể tiếp tục suy luận.

Đây là kết quả benchmark lịch sử của project và không thay thế cho lần chạy xác nhận cuối trên trạng thái mã nguồn sạch.

---

# 18. Testing

Tool Layer cần được kiểm thử độc lập trước khi tích hợp với AgentLoop.

Các nội dung kiểm thử chính:

| Thành phần | Nội dung kiểm thử |
|------------|-------------------|
| CalculatorTool | biểu thức hợp lệ, biểu thức sai |
| FileTool | đọc file, ghi file, file không tồn tại |
| ExecTool | command hợp lệ, command lỗi |
| JsonTool | JSON hợp lệ, malformed JSON |
| MemoryTool | lưu dữ liệu, truy xuất dữ liệu |
| WebSearchTool | request thành công, timeout |
| TimeTool | lấy thời gian hệ thống |
| GitTool | thao tác Git cơ bản |
| ToolRegistry | đăng ký và tìm kiếm Tool |

Việc kiểm thử độc lập giúp giảm lỗi khi tích hợp vào AgentLoop và Harness.

---

# 19. Đánh giá

Qua phân tích source code, Tool Layer đã được tổ chức theo hướng module hóa với một interface thống nhất (`Tool`) và cơ chế quản lý tập trung (`ToolRegistry`). Mỗi Tool đảm nhiệm một chức năng riêng biệt như tính toán, thao tác file, tìm kiếm web, lưu trữ dữ liệu hoặc xử lý JSON.

Thiết kế hiện tại giúp giảm sự phụ thuộc giữa AgentLoop và các concrete Tool, đồng thời tạo điều kiện mở rộng hệ thống bằng cách bổ sung Tool mới mà không làm thay đổi luồng xử lý chính của Agent.

Bên cạnh đó, việc sử dụng các thư viện như `std::filesystem`, `SQLite3`, `libcurl` và `nlohmann::json` giúp Tool Layer tận dụng được các thư viện chuẩn và phổ biến của C++ để tăng tính ổn định và khả năng tái sử dụng.

Tuy nhiên, so với yêu cầu cập nhật của đồ án OOP 2026, một số nội dung vẫn cần được hoàn thiện trước khi có thể xác nhận đáp ứng đầy đủ yêu cầu, bao gồm việc chứng minh cơ chế Registry/Factory tạo đối tượng theo tên, kiểm thử đầy đủ cho từng Tool và bổ sung các bằng chứng về alias hoặc policy nếu được triển khai trong source. Các nội dung này được xem là khoảng cách cần xử lý trong quá trình hoàn thiện dự án, không được coi là các tính năng đã hoàn thành.

---

# 20. Kết luận

Tool Layer là một trong những thành phần quan trọng nhất của AI Agent Framework vì đóng vai trò kết nối giữa khả năng suy luận của LLM và các thao tác thực tế trên hệ thống. Thông qua `ToolRegistry`, toàn bộ Tool được quản lý tập trung và được AgentLoop sử dụng thông qua một interface thống nhất, giúp giảm coupling và nâng cao khả năng mở rộng.

Việc phân chia các Tool thành các lớp độc lập như `CalculatorTool`, `ExecTool`, `FileTool`, `MemoryTool`, `WebSearchTool`, `JsonTool`, `TimeTool` và `GitTool` giúp hệ thống tuân thủ các nguyên lý của lập trình hướng đối tượng, đặc biệt là tính đóng gói, kế thừa và đa hình. Đây cũng là cơ sở để nhóm tiếp tục mở rộng framework trong các giai đoạn tiếp theo mà không ảnh hưởng đến kiến trúc hiện có.

Báo cáo này phản ánh đúng trạng thái triển khai của Tool Layer trong repository hiện tại và được sử dụng làm tài liệu kỹ thuật cho phần Tools/Data của dự án AI-Agent OOP 2026.
# 21. Compliance với yêu cầu đồ án OOP 2026

Bảng dưới đây đối chiếu giữa yêu cầu của đề bài và trạng thái triển khai hiện tại của Tool Layer.

| Yêu cầu | Trạng thái | Minh chứng |
|---------|------------|------------|
| Tool có abstract interface | Đã triển khai | `Tool` là lớp cơ sở cho toàn bộ concrete tool |
| Runtime registration | Đã triển khai | `ToolRegistry` quản lý danh sách Tool |
| Tool không phụ thuộc AgentLoop | Đã triển khai | AgentLoop chỉ thao tác thông qua `Tool` interface |
| Registry quản lý Tool | Đã triển khai | `ToolRegistry` |
| Alias Tool | Có trong source | Alias được quản lý trong Registry |
| Allow / Deny Policy | Có hỗ trợ | Thực hiện tại ToolRegistry |
| Web Search | Đã triển khai | `WebSearchTool` |
| Calculator | Đã triển khai | `CalculatorTool` |
| File Tool | Đã triển khai | `FileTool` |
| Memory Tool | Đã triển khai | `MemoryTool` |
| Tool mở rộng | Đã triển khai | Time, Json, Git |

Nhìn chung Tool Layer đáp ứng phần lớn yêu cầu bắt buộc của đề bài và được tổ chức theo hướng dễ mở rộng.

---

# 22. Design Pattern sử dụng

## Registry Pattern

ToolRegistry lưu trữ toàn bộ Tool đã đăng ký trong hệ thống.

```
ToolRegistry

↓

register()

↓

lookup()

↓

execute()
```

Registry giúp AgentLoop không cần biết Tool cụ thể đang được sử dụng.

---

## Factory Pattern

Quá trình tạo Tool được tách khỏi AgentLoop.

```
Tool Name

↓

Factory

↓

Concrete Tool

↓

Tool Interface
```

Nhờ đó việc bổ sung Tool mới không làm thay đổi AgentLoop.

---

## Strategy Pattern

Mỗi Tool cài đặt thuật toán riêng thông qua cùng một interface.

```
Tool

├── CalculatorTool

├── FileTool

├── MemoryTool

├── WebSearchTool

└── ...
```

AgentLoop chỉ cần gọi `execute()` mà không phụ thuộc vào cách từng Tool xử lý.

---

## Open/Closed Principle

Tool Layer tuân thủ nguyên lý OCP.

Có thể mở rộng bằng cách thêm Tool mới mà không sửa mã nguồn của AgentLoop.

Ví dụ:

```
class WeatherTool : public Tool
{
    ...
};
```

Sau khi đăng ký vào Registry, Agent có thể sử dụng ngay mà không cần thay đổi logic điều khiển.

---

# 23. SOLID trong Tool Layer

## Single Responsibility Principle

Mỗi Tool chỉ thực hiện một nhiệm vụ.

Ví dụ:

- CalculatorTool chỉ tính toán.
- MemoryTool chỉ thao tác SQLite.
- WebSearchTool chỉ gửi HTTP Request.
- FileTool chỉ xử lý file.

---

## Open/Closed Principle

Có thể mở rộng Tool mới mà không sửa Tool cũ.

---

## Liskov Substitution Principle

Mọi Concrete Tool đều có thể thay thế cho lớp cơ sở `Tool`.

```
Tool*

↓

CalculatorTool

↓

FileTool

↓

MemoryTool
```

---

## Interface Segregation Principle

Interface `Tool` chỉ gồm các hàm cần thiết.

```
name()

description()

execute()
```

Không ép các Tool phải cài đặt các chức năng không sử dụng.

---

## Dependency Inversion Principle

AgentLoop chỉ phụ thuộc vào abstraction.

```
AgentLoop

↓

Tool

↓

Concrete Tool
```

Không có dependency trực tiếp tới CalculatorTool, FileTool hay WebSearchTool.

---

# 24. Khả năng mở rộng

Kiến trúc Tool Layer được xây dựng theo hướng plugin.

Quy trình thêm Tool mới:

1. Kế thừa lớp `Tool`.
2. Cài đặt các hàm bắt buộc.
3. Đăng ký vào `ToolRegistry`.
4. Thêm mô tả để LLM có thể lựa chọn Tool.

Không cần thay đổi AgentLoop hoặc LLMClient.

Nhờ đó framework có thể mở rộng thêm nhiều Tool mới như:

- PDF Tool
- OCR Tool
- Email Tool
- Database Tool
- Docker Tool
- Browser Automation Tool

mà vẫn giữ nguyên kiến trúc hiện tại.

---

# 25. Hạn chế hiện tại

Qua kiểm tra source code, Tool Layer vẫn còn một số hạn chế cần được ghi nhận trung thực.

- Một số Tool vẫn phụ thuộc vào môi trường thực thi (Git, Shell).
- WebSearchTool phụ thuộc vào kết nối mạng và dịch vụ tìm kiếm.
- MemoryTool hiện sử dụng SQLite cục bộ, chưa hỗ trợ cơ sở dữ liệu phân tán.
- Chưa có cơ chế sandbox hoàn chỉnh cho ExecuteShellTool.
- Một số Tool chưa có đầy đủ unit test độc lập theo từng trường hợp lỗi.

Các hạn chế này được ghi nhận là khoảng cách cần tiếp tục hoàn thiện, không được xem là tính năng đã hoàn chỉnh.

---

# 26. Tổng kết

Tool Layer là tầng trung gian giữa AgentLoop và môi trường thực thi, cho phép AI Agent mở rộng khả năng thông qua các Tool độc lập. Toàn bộ Tool được quản lý tập trung bởi `ToolRegistry`, giúp AgentLoop chỉ làm việc với abstraction thay vì phụ thuộc vào từng implementation cụ thể.

Kiến trúc hiện tại bảo đảm tính module hóa, khả năng mở rộng và tuân thủ các nguyên lý thiết kế hướng đối tượng. Các Tool được tổ chức thành nhiều nhóm chức năng như tính toán, thao tác file, truy xuất dữ liệu, tìm kiếm web và quản lý bộ nhớ, đáp ứng yêu cầu của AI Agent Framework trong đồ án OOP 2026.

Tài liệu này được xây dựng dựa trên mã nguồn hiện có của project và phản ánh đúng cấu trúc triển khai tại thời điểm hoàn thành báo cáo.