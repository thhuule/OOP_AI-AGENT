# Documentation Guide — Quy chuẩn Tech Doc của AI-AGENT Project

> **Mục đích:** thống nhất cách viết báo cáo đồ án trong `docs/` để ba Role tạo ra một tài liệu kỹ thuật liền mạch, có thể kiểm chứng và không mâu thuẫn với source hiện tại.
>
> **Phạm vi áp dụng:** `docs/bao_cao_du_an.md`, các báo cáo Role, UML/sequence/component docs, README, checklist và storyboard có claim kỹ thuật.
>
> **Nguồn sự thật:** source/test hiện tại → artifact benchmark hiện tại → báo cáo tiến độ → kế hoạch. Không lấy một câu trong tài liệu cũ làm bằng chứng nếu source hoặc test mới hơn cho kết quả khác.

## 1. Luồng bắt buộc của báo cáo tổng

`docs/bao_cao_du_an.md` phải đi theo đúng thứ tự sau:

```text
1. User's Requirement
        ↓
2. System Features
        ↓
3. Tech Solutions
        ↓
4. Architecture & OOP Design
        ↓
5. Detailed Logic & AI Integration
        ↓
6. Implementation & Code Structure
        ↓
7. Verification & Testing
        ↓
8. Evaluation & Benchmark
        ↓
9. Limitations & Future Work
```

Ý nghĩa của luồng:

```text
Người dùng cần gì?
        ↓
Hệ thống cung cấp chức năng gì?
        ↓
Dùng công nghệ nào để giải quyết?
        ↓
Các lớp được tổ chức ra sao?
        ↓
Agent và AI hoạt động thế nào?
        ↓
Code nằm ở đâu?
        ↓
Đã kiểm tra bằng cách nào?
        ↓
Kết quả đo được là gì?
        ↓
Còn giới hạn nào?
```

Không đổi thứ tự chỉ để phù hợp với file báo cáo riêng của một Role. Nội dung Role phải được đưa vào đúng vị trí trong luồng chung.

## 2. Nội dung bắt buộc của từng phần

### 2.1 User's Requirement

Trả lời:

- Người dùng cần hệ thống giải quyết bài toán gì?
- Đề bài bắt buộc những module, pattern, tool, benchmark và deliverable nào?
- Phạm vi nào là bắt buộc, bonus hoặc ngoài phạm vi?

Phải dẫn tới đề bài hoặc kế hoạch hiện hành. Không mô tả cách cài đặt ở phần này.

### 2.2 System Features

Mô tả khả năng quan sát được của hệ thống:

- kết nối LLM;
- ReAct AgentLoop;
- gọi Tool và quản lý Tool;
- Skill loading;
- loop detection;
- Harness/Evaluator/trajectory;
- Environment thật và sandbox;
- multi-agent demo nếu có.

Mỗi feature phải ghi trạng thái thực: `DONE`, `PARTIALLY DONE`, `BLOCKED`, `NOT STARTED` hoặc `CẦN XÁC NHẬN`.

### 2.3 Tech Solutions

Giải thích lựa chọn kỹ thuật và lý do:

- CURL, JSON, SQLite, Threads, CMake;
- C++17/20/23/26;
- RAII, smart pointer và error type;
- chính sách filesystem, shell, API key và timeout;
- portability fallback.

Không chỉ liệt kê tên công nghệ. Phải ghi nhu cầu → giải pháp → file sử dụng → bằng chứng test.

### 2.4 Architecture & OOP Design

Bao gồm:

- kiến trúc phân lớp;
- class/component diagram;
- inheritance, composition, dependency và ownership;
- Strategy, Template Method, Registry/Factory, Observer/Hook;
- SOLID và separation of concerns.

Không nhận một pattern là hoàn thành nếu chỉ có tên trong báo cáo mà thiếu source hoặc focused test bắt buộc.

### 2.5 Detailed Logic & AI Integration

Mô tả luồng chạy chi tiết:

- user instruction → prompt/history → LLM hoặc fallback → parse action;
- Registry lookup/create → Tool execute → observation;
- loop detection và điều kiện dừng;
- StepHook → trajectory;
- Harness load → cleanup → run → evaluate → export.

Phải phân biệt rõ:

- LLM thật với fake/mock LLM;
- deterministic benchmark fallback với model reasoning;
- NativeEnvironment với SandboxEnvironment;
- tool success với final task success.

### 2.6 Implementation & Code Structure

Ánh xạ kiến trúc vào repository:

| Module | Thư mục chính | Owner |
|---|---|---|
| Systems/Core | `src/client/`, `src/agent/`, `src/skills/` | A |
| Tools/Data | `src/tools/` | B |
| Evaluation/Infra | `src/harness/`, `benchmark/` | C |
| Environment | `src/environment/` | A sở hữu interface, C tích hợp Harness |
| Multi-agent | `src/multiagent/` | C |
| Technical docs | `docs/` | A chủ trì format, A/B/C chịu trách nhiệm nội dung của mình |

Chỉ dẫn file bằng link tương đối. Không chép toàn bộ source vào báo cáo; chỉ trích đoạn ngắn khi cần giải thích logic.

### 2.7 Verification & Testing

Ghi theo thứ tự:

1. build;
2. focused/unit test;
3. integration test;
4. CTest;
5. benchmark có điều kiện.

Mỗi claim test phải có:

- lệnh chạy;
- ngày hoặc revision/commit;
- môi trường;
- kết quả chính xác;
- phần chưa được kiểm tra.

Build pass không được dùng thay cho runtime/integration pass. Run cũ không chứng minh worktree mới.

### 2.8 Evaluation & Benchmark

Bao gồm:

- nguồn task `benchmark/tasks.json`;
- phân bố simple/medium/hard;
- evaluator score, action-level score và final success rate;
- run ID, provider, model và trạng thái mock/fallback nếu biết;
- trajectory và artifact minh chứng;
- failure taxonomy và phân tích regression.

Quy tắc bắt buộc:

- Fallback-assisted 10/10 chỉ là **pipeline evidence**, không phải model reasoning 10/10.
- `tokens_used = 0` nghĩa là chưa đo nếu client chưa trả metadata.
- Không dùng artifact cũ hoặc file ngoài run hiện tại để tạo false pass.
- Không in hoặc đưa API key vào tài liệu.

### 2.9 Limitations & Future Work

Tách thành ba nhóm:

1. blocker bắt buộc phải đóng trước khi nộp;
2. giới hạn được phép công khai trung thực;
3. backlog/bonus có thể dời sang tuần sau.

Không chuyển một yêu cầu bắt buộc chưa làm thành “future work” để né Definition of Done.

## 3. Format Markdown thống nhất

### 3.1 Ngôn ngữ

- Báo cáo tổng dùng **tiếng Việt**, giữ thuật ngữ kỹ thuật tiếng Anh khi tên tiếng Anh chính xác hơn.
- Lần đầu dùng thuật ngữ phải giải thích ngắn, ví dụ: “regression — chức năng từng chạy được nhưng bị hỏng sau thay đổi”.
- Tên class, hàm, file, lệnh và trạng thái đặt trong backtick.
- Không trộn tiếng Việt và tiếng Anh trong cùng một câu nếu không cần thiết.

### 3.2 Heading

- Mỗi file chỉ có một heading cấp 1: `#`.
- Chín phần của báo cáo tổng dùng `##`.
- Nội dung con dùng `###`; hạn chế sâu hơn `####`.
- Không lặp số hoặc lặp heading.

### 3.3 Đoạn văn, danh sách và bảng

- Mỗi đoạn trình bày một ý chính.
- Dùng danh sách cho tập hợp ngắn; dùng bảng khi cần so sánh ít nhất ba trường lặp lại.
- Có dòng trống trước/sau list, bảng, heading và code block.
- Bảng phải có header rõ; không để ô trạng thái mơ hồ như “gần xong”.

### 3.4 Code, lệnh và diagram

- Code block phải có language tag: `cpp`, `bash`, `json`, `mermaid` hoặc `text`.
- Lệnh build/test phải dùng đường chạy được từ repository root.
- Mermaid phải render được và mô tả đúng source cuối.
- Diagram không được tạo dependency bị cấm chỉ để sơ đồ dễ vẽ.

### 3.5 Link và nguồn

- Dùng link tương đối trong Markdown, ví dụ `[AgentLoop](../src/agent/agent_loop.cpp)`.
- Benchmark phải ghi run ID và link trực tiếp tới summary/trajectory liên quan.
- Nguồn ngoài như OpenClaw/Hermes phải có tên trang, link và phần ý tưởng đã tham khảo; không sao chép nội dung dài.
- Không dùng link tới file đã bị xóa hoặc tên cũ sau rename.

## 4. Format claim kỹ thuật

Mỗi tính năng hoặc nhận định quan trọng nên theo mẫu:

```markdown
### Tên tính năng

- **Trạng thái:** DONE | PARTIALLY DONE | BLOCKED | NOT STARTED | CẦN XÁC NHẬN
- **Owner:** Role A | Role B | Role C | phối hợp
- **Mục tiêu:** Tính năng giải quyết việc gì.
- **Source:** Link tới class/hàm/file thực tế.
- **Test:** Lệnh hoặc test case kiểm chứng.
- **Kết quả:** Kết quả đã quan sát được.
- **Limitation:** Phần chưa được chứng minh hoặc điều kiện còn thiếu.
```

Quy tắc kết luận:

| Bằng chứng | Cách được phép viết |
|---|---|
| Chỉ đọc source | “Source có implementation…” |
| Build pass | “Code compile/link thành công…” |
| Focused test pass | “Trường hợp được test hoạt động…” |
| Integration pass | “Các component được kiểm tra đã tích hợp…” |
| Benchmark provider thật | “Run X với provider/model Y đạt…” |
| Thiếu bằng chứng | `CẦN XÁC NHẬN` hoặc ghi rõ suy luận |

Không viết “hệ thống hoạt động hoàn toàn” nếu mới chỉ build hoặc test một phần.

## 5. Phân chia nội dung A/B/C

| Role | Nội dung chịu trách nhiệm | File nguồn chính |
|---|---|---|
| A | Requirements core, LLM/AgentLoop, Skill, LoopDetector, Architecture/OOP, C++ feature, UML | `report_oop_design.md`, UML docs |
| B | Tool inventory, Registry/Factory, alias/policy, tool error/security, nguồn tool bổ sung | `report_tools.md` |
| C | Harness/Evaluator, Environment integration, trajectory, test, benchmark, failure taxonomy, README/video | `report_evaluation.md`, README và checklist |

Mỗi Role chịu trách nhiệm tính đúng của claim thuộc layer mình. Báo cáo tổng do A chủ trì ghép format, nhưng không tự sửa claim kỹ thuật của B/C nếu chưa có xác nhận.

## 6. Quy trình ghép báo cáo

```text
A/B/C cập nhật báo cáo Role theo source
                ↓
Gắn link source/test/artifact
                ↓
Review chéo contract và dependency
                ↓
Đưa nội dung vào đúng một trong 9 phần
                ↓
Loại bỏ nội dung lặp và claim mâu thuẫn
                ↓
Chạy link/Markdown/Mermaid review
                ↓
Freeze docs theo commit source cuối
```

Nếu source thay đổi sau khi freeze tài liệu, owner của source phải báo cho owner tài liệu liên quan và mở lại checkbox review.

## 7. Template báo cáo tổng

```markdown
# Báo cáo kỹ thuật — AI-AGENT Project

## 1. User's Requirement — Yêu cầu người dùng và đồ án

## 2. System Features — Chức năng hệ thống

## 3. Tech Solutions — Giải pháp kỹ thuật

## 4. Architecture & OOP Design — Kiến trúc và thiết kế OOP

## 5. Detailed Logic & AI Integration — Logic chi tiết và tích hợp AI

## 6. Implementation & Code Structure — Triển khai và cấu trúc mã nguồn

## 7. Verification & Testing — Kiểm chứng và kiểm thử

## 8. Evaluation & Benchmark — Đánh giá và benchmark

## 9. Limitations & Future Work — Giới hạn và hướng phát triển
```

## 8. Checklist review trước khi merge

- [ ] Báo cáo tổng có đủ chín phần, đúng thứ tự.
- [ ] Mỗi file chỉ có một `#` và không có heading/số mục lặp.
- [ ] Ngôn ngữ và thuật ngữ nhất quán.
- [ ] Claim kỹ thuật quan trọng có owner, source, test/result và limitation.
- [ ] UML/sequence/component diagram khớp source cuối và render được.
- [ ] C++17/20/23/26 có ma trận file → mục đích → test → fallback.
- [ ] Registry/Factory và Template Method có focused evidence, không chỉ có tên.
- [ ] Số benchmark khớp artifact và ghi đúng run/provider/model.
- [ ] Fake/mock/fallback được công khai; không đánh đồng pipeline với model reasoning.
- [ ] Không lộ API key, `config.json`, database hoặc artifact ngoài phạm vi.
- [ ] Không còn link chết, tên file cũ hoặc nội dung lặp mâu thuẫn.
- [ ] A/B/C đã review phần thuộc layer mình trước khi freeze.

## 9. Definition of Done cho quy chuẩn tài liệu

Quy chuẩn được coi là áp dụng xong khi:

- `docs/bao_cao_du_an.md` được chuyển sang đúng chín phần;
- `report_oop_design.md`, `report_tools.md`, `report_evaluation.md` dùng chung thuật ngữ và format bằng chứng;
- các claim trạng thái khớp `KH_TUAN9.5.md`/`PROJECT_STATUS.md` mới nhất;
- link và Mermaid đã được kiểm tra;
- ba Role xác nhận phần mình trước khi chốt báo cáo.
