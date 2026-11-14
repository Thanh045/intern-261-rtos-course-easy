# Report — Week 3 / Ex3

`student_id`: 2550517    Commit của PREDICTION.md: d6572b7

## 1. Thời gian byte & độ sâu stream buffer
Phép tính thời gian 1 byte ở `RX_BAUD`, và số lệnh tối đa chứa được trong
`STREAM_BUFFER_BYTES`.
Khung truyền chuẩn 8N1 gồm : 1 start bit + 8 data bits + 0 parity + 1 stop bit = 10 bits. Với tốc độ baud 115200, thời gian truyền 10 bit là (10 x 1) / 115200 là xấp xỉ 86,81 us.
Bộ đệm có dung lượng STREAM_BUFFER_BYTES = 64 bytes, và độ dài tối đa của một lệnh là MAX_CMD_LEN = 16 bytes.
Số lệnh có kích thước tối đa mà buffer có thể lưu trữ trước khi đầy là: 64/16 = 4
## 2. Demo 3 loại lệnh
Bằng chứng (log/ảnh) `demo_task` phản ứng đúng với `P`, `S`, `T` — mỗi loại ít
nhất 1 ví dụ có số liệu trước/sau.
OK P100
OK P200
OK T0
OK T1
OK S2
OK S1
OK P500



## 3. Test gửi nhanh
| | Số lệnh gửi | Số lệnh nhận đúng | Mất/lỗi |
|---|---|---|---|
| Gõ tay bình thường |7 |7 |0 |
| Gửi liên tục (script) |500 |214 | 286|

Nếu có mất byte: giải thích bằng cơ chế (stream buffer đầy, parser bị trễ,
hay nguyên nhân khác) — không chỉ nói "bị mất".
Nguyên nhân có thể do dữ liệu được gửi liên tục với tốc độ cao, không có khoảng nghỉ giữa các lệnh, khiến parser_task không kịp xử lý. Đặc biệt, khi parser_task thực hiện HAL_UART_Transmit để gửi phản hồi, task bị chặn trong lúc dữ liệu mới vẫn tiếp tục được đưa vào stream buffer. Nếu buffer 64 byte bị đầy, các byte tiếp theo có thể bị mất, dẫn đến lệnh sai định dạng hoặc không được xử lý đúng.
## 4. Vì sao stream buffer, không phải queue
Giải thích bằng lời của bạn.
Stream buffer phù hợp hơn queue vì dữ liệu UART được nhận dưới dạng luồng byte liên tục, không phải các bản ghi có kích thước cố định.
Trong chương trình, mỗi khi UART nhận được một byte, ISR sử dụng xStreamBufferSendFromISR() để đưa byte vào stream buffer. Sau đó, parser_task sử dụng xStreamBufferReceive() để lấy từng byte, ghép thành chuỗi lệnh cho đến khi gặp ký tự '\r'.
Stream buffer cho phép lưu trữ liên tiếp các byte và đọc theo số lượng mong muốn, phù hợp với việc xử lý các lệnh UART có độ dài khác nhau.
Queue cũng có thể truyền từng byte từ ISR sang task, nhưng mỗi phần tử queue có cơ chế quản lý riêng. Vì vậy, sử dụng queue cho luồng byte liên tục thường không phù hợp và có thể tốn thêm tài nguyên so với stream buffer.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
Kết quả thực nghiệm nhìn chung phù hợp với dự đoán ban đầu.
Khi gửi lệnh bằng cách gõ tay bình thường, chương trình nhận đúng 7/7 lệnh, không ghi nhận lỗi.
Khi gửi liên tục 500 lệnh bằng script ở tốc độ 115200 baud, chỉ ghi nhận 214 ACK hợp lệ và 286 lệnh không có ACK hợp lệ. Kết quả này phù hợp với dự đoán rằng dữ liệu gửi quá nhanh có thể khiến parser_task không kịp xử lý, làm stream buffer có nguy cơ bị đầy.
Nguyên nhân có thể do parser_task phải thực hiện HAL_UART_Transmit đồng bộ để phản hồi từng lệnh, làm giảm thời gian đọc dữ liệu trong stream buffer.
Không có sai lệch đáng kể về xu hướng giữa dự đoán và kết quả thực nghiệm.
## 6. Câu hỏi đánh đổi
Với `RX_BAUD` của bạn, nếu tăng gấp đôi `STREAM_BUFFER_BYTES`, tốc độ gửi tối
đa không mất byte tăng thêm bao nhiêu (đo thật, không suy luận)?
Khi tăng STREAM_BUFFER_BYTES từ 64 lên 128 bytes và gửi liên tục 500 lệnh không có khoảng nghỉ, số ACK hợp lệ tăng từ 214 lên 220, tức tăng 6 lệnh (khoảng 2,8%).
Tuy nhiên, cả hai cấu hình đều xảy ra tình trạng thiếu ACK. Tốc độ gửi đo được gần như không đổi, khoảng 1318 lệnh/giây, vì baud rate vẫn giữ ở 115200.
Việc tăng kích thước stream buffer giúp lưu trữ nhiều dữ liệu hơn khi parser_task xử lý chậm, nhưng không làm tăng tốc độ truyền UART hoặc tốc độ xử lý của task.