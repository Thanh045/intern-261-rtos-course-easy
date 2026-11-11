# Dự đoán — Week 3 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517

1. Thời gian truyền 1 byte ở `RX_BAUD` của bạn là bao nhiêu us? (8N1, tính cả
   start/stop bit). Trong thời gian đó `parser_task` có kịp xử lý 1 byte từ
   stream buffer không, nếu nó là task ưu tiên cao nhất hệ thống?
Khung truyền chuẩn 8N1 gồm : 1 start bit + 8 data bits + 0 parity + 1 stop bit = 10 bits. Với tốc độ baud 115200, thời gian truyền 10 bit là (10 x 1) / 115200 là xấp xỉ 86,81 us.
 parser_task kịp xử lý.Vi điều khiển chạy ở tần số hàng chục MHz (chu kỳ lệnh chỉ vài chục nano-giây), nên 86,81us  tương đương với hàng nghìn chu kỳ CPU. Khi có ưu tiên cao nhất hệ thống và TRIGGER_LEVEL = 1, ngay khi ISR nhận xong byte và gửi vào buffer, task được đánh thức ngay lập tức. Việc đọc byte và ghi vào mảng chỉ tốn vài micro-giây, hoàn tất trước khi byte tiếp theo trên dây UART được truyền xong

2. `STREAM_BUFFER_BYTES` của bạn chứa tối đa bao nhiêu lệnh dài `MAX_CMD_LEN`
   byte trước khi đầy?
Bộ đệm có dung lượng STREAM_BUFFER_BYTES = 64 bytes, và độ dài tối đa của một lệnh là MAX_CMD_LEN = 16 bytes.

Số lệnh có kích thước tối đa mà buffer có thể lưu trữ trước khi đầy là: 64/16 = 4

3.Nếu `xStreamBufferSendFromISR` trả về 0 (stream buffer đầy) trong
   `uart_rx_isr_handler`, byte đó sẽ bị mất. Theo bạn, trường hợp nào thực tế
   khiến điều này xảy ra (gửi quá nhanh / `parser_task` ưu tiên quá thấp /
   cái khác)?
Buffer bị đầy và trả về 0 xảy ra khi tốc độ dữ liệu nạp vào nhanh hơn tốc độ giải phóng của parser_task:

Hàm apply_command() bị nghẽn (blocking): Khi đọc đến ký tự kết thúc \r, task gọi apply_command(). Nếu trong hàm này có các thao tác chiếm dụng thời gian như in log UART đồng bộ (HAL_UART_Transmit có chờ), gọi delay, hoặc xử lý tính toán nặng, task sẽ không kịp quay lại đọc buffer.

Mất quyền thực thi: Khi một task khác hoặc một đoạn mã giữ vùng cấm (Critical Section / tắt ngắt) quá lâu, scheduler không thể chuyển ngữ cảnh sang parser_task.

Gửi dữ liệu dạng dồn dập (Burst Traffic): Phía máy tính gửi một chuỗi hàng chục byte liên tục không có khoảng nghỉ vượt quá khả năng đệm 64 bytes khi task đang bận xử lý frame trước.

4.Dự đoán: gửi lệnh bằng gõ tay bình thường có mất byte không? Gửi bằng
   script gửi liên tục không nghỉ thì sao?

Gõ tay thông thường: Không mất byte.Tốc độ gõ phím của con người trung bình từ 100ms đến 500ms mỗi ký tự, lớn hơn gấp hàng nghìn lần so với thời gian truyền 1 byte 86,8us. Buffer luôn được task dọn rỗng sau từng phím gõ
.Gửi bằng script liên tục không nghỉ: Rất dễ bị mất byte.Script có thể bơm dữ liệu liên tục sát ngưỡng băng thông tối đa. Chỉ cần CPU trễ một vài mili-giây khi thực thi xong một lệnh và áp dụng cấu hình mới, lượng byte tồn đọng từ các lệnh gửi dồn tiếp theo sẽ nhanh chóng lấp đầy 64 bytes của buffer, dẫn đến việc ngắt ISR phải bỏ rơi byte.
