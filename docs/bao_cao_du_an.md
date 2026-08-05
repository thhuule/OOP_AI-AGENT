# Báo cáo dự án: Xây dựng hệ thống AI Agent bằng C++

## 1. Giới thiệu

Dự án này xây dựng một hệ thống AI Agent theo hướng lập trình hướng đối tượng (OOP) bằng ngôn ngữ C++. Hệ thống có khả năng kết nối với các mô hình ngôn ngữ lớn, điều phối các bước suy nghĩ và hành động, gọi các công cụ bên ngoài, ghi lại quá trình thực thi và đánh giá kết quả thông qua benchmark.

Mục tiêu chính của dự án là mô phỏng một agent có thể:
- nhận yêu cầu từ người dùng;
- lập kế hoạch và chọn hành động phù hợp;
- gọi các công cụ như máy tính, thao tác tệp, thực thi lệnh hoặc tìm kiếm web;
- đánh giá kết quả thông qua các bài kiểm thử chuẩn hóa.

## 2. Mục tiêu của dự án

1. Xây dựng một kiến trúc agent rõ ràng, tách bạch giữa các lớp trách nhiệm.
2. Áp dụng các nguyên tắc OOP như trừu tượng hóa, đóng gói, đa hình và phân lớp.
3. Tạo được một vòng lặp agent có thể thực hiện nhiều bước liên tiếp cho đến khi đạt kết quả cuối cùng.
4. Hỗ trợ đánh giá hiệu năng thông qua bộ test và benchmark chuẩn.
5. Xây dựng nền tảng mở rộng cho các phiên bản sau này, ví dụ thêm agent đa tác tử hoặc tích hợp thêm công cụ.

## 3. Kiến trúc hệ thống

Hệ thống được tổ chức thành các tầng chính sau:

- Tầng client: quản lý kết nối với mô hình ngôn ngữ như Gemini hoặc Ollama.
- Tầng agent core: điều phối vòng lặp suy nghĩ – hành động – phản hồi.
- Tầng tools: thực hiện các công cụ như tính toán, đọc/ghi file, chạy lệnh, tìm kiếm web.
- Tầng harness: thực hiện benchmark, đánh giá kết quả và ghi lại trajectory.
- Tầng multi-agent: hỗ trợ mô hình phối hợp nhiều tác tử trong một hệ thống lớn hơn.

Các thành phần quan trọng của dự án có thể xem tại các file sau:
- [README.md](../README.md)
- [src/agent/agent_loop.cpp](../src/agent/agent_loop.cpp)
- [src/tools/ToolRegistry.cpp](../src/tools/ToolRegistry.cpp)
- [src/harness/HarnessRunner.cpp](../src/harness/HarnessRunner.cpp)
- [benchmark/run_eval.cpp](../benchmark/run_eval.cpp)

## 4. Các thành phần chính

### 4.1 LLM Client

Module này đóng vai trò giao tiếp với mô hình ngôn ngữ. Nó nhận đầu vào là lịch sử hội thoại và trả về phản hồi của mô hình. Việc tách lớp client giúp hệ thống dễ thay đổi giữa các nhà cung cấp khác nhau như Gemini và Ollama.

### 4.2 Agent Loop

Agent Loop là trái tim của hệ thống. Nó duy trì vòng lặp xử lý các bước:
1. Nhận yêu cầu từ người dùng.
2. Tạo quyết định hành động.
3. Gọi công cụ nếu cần.
4. Ghi nhận kết quả.
5. Dừng lại khi đạt điều kiện hoàn thành hoặc phát hiện vòng lặp.

### 4.3 Tool Registry

Tool Registry giúp đăng ký và quản lý các công cụ có thể sử dụng bởi agent. Điều này làm cho hệ thống linh hoạt và dễ mở rộng mà không cần sửa nhiều code cứng trong agent loop.

### 4.4 Harness và Evaluator

Harness chịu trách nhiệm chạy benchmark, kiểm tra kết quả và lưu lại thông tin thực thi từng tác vụ. Evaluator dùng để xác định xem nhiệm vụ có thành công hay không, có thể là kiểm tra keyword, chạy script đánh giá hoặc các cơ chế khác.

### 4.5 Multi-agent

Dự án cũng cung cấp mô hình multi-agent với worker thread và dispatcher, cho phép nhiều tác tử phối hợp công việc cùng lúc. Đây là một phần mở rộng quan trọng, giúp hệ thống có thể phát triển sang các ứng dụng phức tạp hơn.

## 5. Các mẫu thiết kế áp dụng

Dự án áp dụng một số mẫu thiết kế phù hợp với kiến trúc hệ thống:

- Strategy Pattern: dùng cho Evaluator, cho phép đổi giữa các chiến lược đánh giá khác nhau.
- Registry/Factory Pattern: dùng cho Tool Registry, giúp tạo và quản lý các công cụ theo tên.
- Observer/Hook Pattern: dùng cho StepHook, cho phép ghi lại từng bước thực thi của agent.
- Separation of Concerns: mỗi module có trách nhiệm riêng, tránh phụ thuộc lẫn nhau quá chặt chẽ.

Điều này giúp cho hệ thống dễ bảo trì, dễ kiểm thử và dễ nâng cấp trong tương lai.

## 6. Quá trình đánh giá và benchmark

Một điểm nổi bật của dự án là khả năng đánh giá hiệu năng thông qua benchmark. Bộ benchmark đọc các task từ file [benchmark/tasks.json](../benchmark/tasks.json), chạy chúng tuần tự và ghi lại kết quả vào thư mục [benchmark/results](../benchmark/results).

Các chỉ số đánh giá bao gồm:
- điểm số của evaluator;
- điểm số ở mức action/tool;
- tỉ lệ hoàn thành cuối cùng;
- số lượng task thành công theo từng mức độ khó.

Các báo cáo liên quan có thể xem ở:
- [docs/report_evaluation.md](report_evaluation.md)
- [docs/report_oop_design.md](report_oop_design.md)

## 7. Điểm mạnh của dự án

- Kiến trúc rõ ràng và dễ hiểu.
- Tách biệt rõ giữa agent, công cụ và đánh giá.
- Có khả năng mở rộng với nhiều loại công cụ và evaluator mới.
- Dễ kiểm thử bằng các chương trình test độc lập.
- Có hỗ trợ benchmark và lưu lịch sử kết quả.

## 8. Hạn chế và hướng phát triển

Mặc dù dự án đã có nền tảng tốt, vẫn còn một số hạn chế:
- một số evaluator chưa hoàn thiện đầy đủ;
- việc thu thập token metadata vẫn còn giới hạn;
- benchmark hiện chưa hoàn toàn cô lập môi trường cho từng task;
- hệ thống có thể tiếp tục cải thiện về độ ổn định và tính tự động hóa.

Trong tương lai, nhóm có thể cải thiện bằng cách:
- bổ sung thêm evaluator mạnh hơn;
- tích hợp token tracking chính xác;
- tăng cường kiểm thử cho các trường hợp lỗi;
- mở rộng hệ thống multi-agent và cải thiện khả năng phối hợp giữa các tác tử.

## 9. Kết luận

Dự án này là một nền tảng tốt cho việc xây dựng một hệ thống AI Agent theo hướng lập trình hướng đối tượng bằng C++. Với kiến trúc tách tầng, các mẫu thiết kế phù hợp và khả năng benchmark rõ ràng, dự án không chỉ thể hiện được cách xây dựng agent thông minh mà còn cho thấy khả năng mở rộng và ứng dụng thực tiễn trong các hệ thống AI tự động hóa.

Thông qua dự án này, chúng ta có thể thấy rằng việc thiết kế hệ thống đúng cách là yếu tố quan trọng để phát triển một agent có thể tin cậy, dễ bảo trì và có thể tích hợp vào các ứng dụng thực tế trong tương lai.
