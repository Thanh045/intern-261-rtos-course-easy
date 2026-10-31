# Report — Week 2 / Ex1

`student_id`: 2550517   Commit của PREDICTION.md: 845032a


## 1. Kích thước queue tối thiểu
| | Dự đoán | Đo được | Khớp? |
|---|---|---|---|
| `queue_raw` |1 |1 | Có|
| `queue_filtered` |1|1 |Có |

Nếu lệch: nguyên nhân (thời gian xử lý thực tế khác ước lượng, jitter I2C, ...).
Thử cả hai queue dài 1 phần tử ở 115200 baud. Sau khi UART task nhận 11.687 mẫu, không ghi nhận lỗi đọc cảm biến, gửi queue thất bại, thiếu số thứ tự hoặc lỗi UART. Vì vậy, độ dài 1 đủ trong cấu hình và thời gian đã thử, phù hợp dự đoán.
## 2. Đồ thị độ đầy queue theo thời gian
Cấu hình bình thường: `![](queue_fill_normal.png)`
Cấu hình UART chậm (9600 baud): `![](queue_fill_slow_uart.png)`

Giải thích hình dạng đồ thị ở mỗi cấu hình.
Ở 115200 baud, UART xử lý kịp nên queue không tích tụ. Khi giảm xuống 9600 baud, queue_filtered tăng dần rồi dao động gần mức đầy 12 phần tử. Queue_raw vẫn gần 0 vì filter không chờ khi gửi thất bại mà bỏ mẫu.
## 3. Khi nào queue đầy
Mô tả **chính xác** thời điểm `queue_filtered` bắt đầu đầy khi hạ baud, và số
mẫu bị mất/trễ (đếm bằng số thứ tự mẫu gắn trong payload).
Ở 9600 baud và timeout gửi bằng 0, lần đầu logger quan sát queue_filtered đầy là 244 ms tính từ lúc scheduler chạy. Đây là thời điểm quan sát với chu kỳ đo 2 ms, không phải thời điểm chính xác tuyệt đối của lần đầy đầu tiên.

Tại lần dừng GDB, filter đã bỏ 3.827 mẫu do gửi queue_filtered thất bại. UART task nhận 1.932 mẫu, phát hiện 3.802 số thứ tự bị thiếu. Hai bộ đếm khác nhau vì các khoảng thiếu gần cuối chưa tới UART. Lỗi đọc cảm biến, gửi queue_raw thất bại và lỗi UART đều bằng 0.
## 4. Hành vi 3 kiểu timeout
| Timeout | Kết quả quan sát khi queue đầy |
|---|---|
| 0 | 	Gửi thất bại ngay, bỏ mẫu. Đo được 3.827 lần gửi queue_filtered thất bại.|
| giá trị hữu hạn |Khi queue đầy, filter chờ tối đa 4 tick. Nếu vẫn chưa có chỗ, gửi thất bại và bỏ mẫu. Tại lần quan sát, ghi nhận 4.628 lần gửi queue_filtered thất bại và 4.602 số thứ tự thiếu tại UART. |
| `portMAX_DELAY` |Khi queue đầy, task bị Block cho đến khi có chỗ trống. Tại thời điểm quan sát, UART đã nhận 2655 mẫu, không ghi nhận gửi queue thất bại hoặc thiếu số thứ tự. Cách này tránh bỏ mẫu do queue đầy nhưng có thể làm trễ chu kỳ lấy mẫu của sensor. |

## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.
Kích thước queue tối thiểu thử nghiệm là 1 phần tử cho mỗi queue, phù hợp dự đoán: ở 115200 baud, UART nhận 11687 mẫu, không ghi nhận gửi queue thất bại hoặc thiếu số thứ tự.

Ở cấu hình bình thường, queue_raw ghi nhận bằng 0, queue_filtered dao động 0–1. Queue_raw có thể chứa mẫu trong thời gian rất ngắn nhưng phép đo mỗi 2 ms không bắt được.

Khi giảm UART xuống 9600 baud, queue_filtered được quan sát đầy lần đầu tại 244 ms. Thời gian truyền thực tế phụ thuộc độ dài dòng CSV; con số 34,375 ms trong dự đoán là trường hợp tối đa 33 byte.
## 6. Câu hỏi đánh đổi
Với `SAMPLE_PERIOD_MS` của **bạn**, nếu `FILTER_WINDOW` tăng gấp đôi thì độ trễ
từ cảm biến đến UART tăng thêm bao nhiêu ms? Trả lời bằng số từ dữ liệu của bạn,
rồi đo lại để kiểm chứng. 
FILTER_WINDOW	Min	         Max	Trung bình
16	                     2845 µs	3025 µs	2933 µs
32	                     2844 µs	3028 µs	2932 µs


Chênh lệch trung bình:
\[
2932-2933=-1us

Thời gian từ đọc cảm biến đến truyền xong UART gần như không đổi. Chênh lệch 1 µs chưa cho thấy window 32 nhanh hơn; độ dài CSV và thời điểm ngắt có thể ảnh hưởng số đo.