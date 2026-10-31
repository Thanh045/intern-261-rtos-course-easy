# Dự đoán — Week 2 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517

1. Phép tính kích thước queue tối thiểu (dùng `SAMPLE_PERIOD_MS`, thời gian xử lý
   ước lượng của `filter_task` và `uart_task` ở `UART_BAUD`):
SAMPLE_PERIOD_MS = 8 ms
Tốc độ tạo mẫu = 1000 / 8 = 125 mẫu/giây.

Ước lượng filter xử lý một mẫu dưới 1 ms:
mỗi lần chỉ trừ mẫu cũ, cộng mẫu mới và chia trung bình.
FILTER_WINDOW = 16 không có nghĩa là phải chờ đủ 16 mẫu mới gửi.

Dòng UART: "seq,ax,ay,az\r\n"
Ước lượng tối đa 33 bytes:
10 chữ số seq + 3 dấu phẩy + 3 × 6 ký tự trục + 2 ký tự CRLF.

UART 8N1 dùng 10 bit/byte:
Thời gian truyền = 33 × 10 / 115200
                 ≈ 2,865 ms.

Giả sử định dạng chuỗi và lập lịch thêm dưới 1 ms:
Thời gian phục vụ UART dưới 3,865 ms < 8 ms.

   ```
 
   ```
   Kích thước tối thiểu dự đoán: `queue_raw` = 1, `queue_filtered` = 1 .
   So với `QUEUE_LENGTH` được cấp: đủ hay không? 

QUEUE_LENGTH = 12 đủ và có thêm chỗ đệm cho jitter ngắn.
2. Khi hạ `UART_BAUD` xuống 9600 (tạm thời), queue nào sẽ đầy trước? Vì sao? 
queue_filtered dự đoán đầy trước. Truyền dòng 33 bytes mất khoảng 34,375 ms, lâu hơn chu kỳ tạo mẫu 8 ms, nên mẫu đến nhanh hơn UART xử lý.

Với timeout gửi bằng 0, filter bỏ mẫu khi queue_filtered đầy nên queue_raw không nhất thiết đầy theo. Nếu filter chờ vô hạn, nghẽn có thể lan ngược về queue_raw.  
3. Với `xQueueSend(..., 0)` (timeout = 0) khi queue đầy: điều gì xảy ra với 
mẫu đó? 
Khi queue đầy, hàm trả về ngay errQUEUE_FULL, không đưa mẫu vào queue. Code hiện tại tăng bộ đếm gửi thất bại và bỏ mẫu đó.
4. Với `xQueueSend(..., portMAX_DELAY)` khi queue đầy: `sensor_task` sẽ ở trạng thái
   nào? Có nguy cơ gì cho chu kỳ lấy mẫu?    
Khi queue đầy, sensor_task chuyển sang trạng thái Blocked để chờ có chỗ. Với cấu hình chờ vô hạn, thời gian chờ có thể vượt 8 ms, làm trễ chu kỳ lấy mẫu.   
5. Độ đầy `queue_raw` theo thời gian ở cấu hình bình thường sẽ trông như thế nào
   (tăng dần / dao động quanh một mức / luôn gần 0)? Vì sao?  
Dự đoán gần 0, thỉnh thoảng tăng lên 1 rồi giảm vì filter xử lý nhanh hơn tốc độ lấy mẫu. Logger lấy mẫu mỗi 2 ms có thể không thấy những lần tăng rất ngắn.
