# Dự đoán — Week 2 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517  `WRITER_COUNT` =    4 `MSG_LEN` = 24

1. Ở `UART_GUARD_MODE = 0`: dự đoán lỗi lẫn lộn sẽ xuất hiện sau khoảng bao
   nhiêu dòng log? Vì sao MSG_LEN và WRITER_PERIOD_MS của bạn ảnh hưởng đến
   con số này?
Em dự đoán có thể xuất hiện lỗi lẫn lộn ngay trong cỡ 10 dòng log đầu, vì mỗi byte cần truyền 10 bit theo dạng 8IN1, 24 byte cần thời gian là (24*10)/115200 = 2.083 ms, dài hơn 1 tick 1ms nên task khác có cơ hội chạy và gửi xen vào khi message chưa hoàn tất. Message càng dài và thời gian nghỉ càng ngắn thì khả năng tranh chấp càng cao.
Với code gửi từng byte hiện tại, 24 bytes không được bảo vệ như một khối, nên các byte từ writer khác có thể xen giữa.
2. Dự đoán thứ tự **độ trễ in** (latency) từ thấp đến cao giữa mutex / critical
   section / gatekeeper. Vì sao?
Em dự đoán thứ tự lần lượt là critical section → mutex → gatekeeper. Critical section không cho task khác ngắt quãng đoạn truyền, nhưng có thể khiến writer chưa được chạy phải chờ lâu. Mutex Mất thời gian gọi xSemaphoreTake và xSemaphoreGive (kiểm tra trạng thái lock, quản lý danh sách chờ).. Gatekeeper cần thêm thao tác queue và chuyển sang task gửi UART.
3. Với critical section, task nào trong hệ thống có nguy cơ bị ảnh hưởng xấu
   nhất nếu đoạn in quá dài? (gợi ý: nghĩ đến ngắt, không chỉ task)
Các ngắt có độ ưu tiên bị critical section che, như SysTick, sẽ bị trì hoãn. Task đang chờ tick để thức dậy và task cần phản hồi nhanh vì vậy có thể chạy muộn. Nếu ngắt UART/DMA cũng bị che, việc xử lý nhận dữ liệu có thể chậm và gây mất dữ liệu.
4. RAM dùng thêm của gatekeeper so với mutex: dự đoán tăng hay giảm, vì sao?
Em dự đoán gatekeeper dùng nhiều RAM hơn. Ngoài cấu trúc queue, nó cần vùng chứa message, TCB và stack của task gatekeeper. Với code hiện tại, riêng stack gatekeeper là 256 × 4 = 1024 bytes, vùng dữ liệu queue là 4 × 24 = 96 bytes. Mutex chỉ cần đối tượng quản lý khóa và overhead cấp phát.
5. Nếu `WRITER_COUNT` tăng gấp đôi, cách nào trong 3 cách bị ảnh hưởng về độ trễ
   nhiều nhất? Vì sao?
Em dự đoán gatekeeper dễ tăng độ trễ do message xếp hàng khi tốc độ tạo message vượt khả năng UART. Nếu mọi writer thực sự phát một message mỗi 7 ms, 8 writer yêu cầu khoảng 8 × 24 / 0,007 ≈ 27429 bytes/s, vượt khả năng lý tưởng khoảng 115200 / 10 = 11520 bytes/s của UART.

