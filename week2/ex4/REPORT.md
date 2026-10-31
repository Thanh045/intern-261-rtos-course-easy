# Report — Week 2 / Ex4

`student_id`: 2550517   Commit của PREDICTION.md: 6c4541e 

## 1. Bảng so sánh RAM
| | Bytes thực tế | Cách tính |
|---|---|---|
| Event group (4 điểm đồng bộ) |40 |10232 − 10192; gồm cấu trúc 28 bytes và overhead/căn chỉnh 12 bytes
| Event group (3 điểm) + notification (1 điểm) | 40|10232 − 10192; gồm cấu trúc 28 bytes và overhead/căn chỉnh 12 bytes |
| Chênh lệch |0 | |

## 2. Bảng so sánh tốc độ đánh thức
| Cách | Thời gian đánh thức (cycle) | Thời gian đánh thức (us) |
|---|---|---|
| Event-group-bit (producer_1 hoặc 2) |1125 | 15,625|
| Task notification (producer_0) |958 |13,306 |

Mô tả cách đo (2 điểm DWT: lúc gọi set/notify, lúc sync_task bắt đầu chạy):
Em đọc DWT->CYCCNT ngay trước khi producer 0 gọi xEventGroupSetBits() hoặc xTaskNotifyGive(), rồi đọc lại ngay sau khi sync_task thoát hàm chờ. Hiệu hai giá trị là số chu kỳ CPU đã trôi qua; với CPU 72 MHz, chia cho 72 để đổi sang µs. Em lấy trung bình 1000 mẫu, chỉ cho producer 0 báo hiệu để không tính thêm thời gian chờ các producer khác.
## 3. Khi nào KHÔNG dùng được notification
Tình huống bạn dự đoán ở PREDICTION.md, kiểm chứng lại: đúng hay sai, vì sao.
Thử nghiệm cho hai task cùng chờ một sự kiện. Với event group, một lần set bit đánh thức cả hai task. Với notification gửi tới task 0, chỉ task 0 nhận được; task 1 bị timeout. Vì vậy, không thể thay trực tiếp event group bằng một notification khi cần đánh thức nhiều task cùng chờ. Muốn dùng notification phải gửi riêng cho từng task.
## 4. Timeout
Log/ảnh một lần `sync_task` bị timeout (cố tình làm 1 producer chậm hơn
`SYNC_TIMEOUT_MS`), và giá trị trả về của `xEventGroupWaitBits` lúc đó.
Cho producer 1 trễ 160 ms, lớn hơn SYNC_TIMEOUT_MS = 80 ms. Khi timeout, xEventGroupWaitBits() trả về 0xd, thiếu bit 1 so với mặt nạ đủ 0xf. Vì (0xd & 0xf) != 0xf, chương trình xác định chưa đủ bit và tăng bộ đếm timeout.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.
Cấu trúc event group đo được 28 bytes, đúng dự đoán. Heap thực bị trừ 40 bytes, gồm thêm 12 bytes header và căn chỉnh. Khi thay một điểm đồng bộ bằng notification, phần cấp phát thêm vẫn là 0 bytes; tổng event group vẫn chiếm 40 bytes vì ba producer còn lại vẫn dùng nó. Notification đánh thức nhanh hơn: 958 cycle so với 1125 cycle, giảm 167 cycle ≈ 2,319 µs, phù hợp dự đoán.

## 6. Câu hỏi đánh đổi
Với `EVENT_BIT_COUNT` của **bạn**, nếu tăng thêm 1 bit (1 producer nữa) thì chu
kỳ đồng bộ trung bình đo được thay đổi thế nào? Trả lời bằng số từ dữ liệu của
bạn, rồi đo lại để kiểm chứng.
Khi tăng EVENT_BIT_COUNT từ 4 lên 5, chu kỳ đồng bộ trung bình tăng từ 11,80 ms lên 12,07 ms, tăng khoảng 0,27 ms (2,26%) trên 30 mẫu mỗi cấu hình. Việc chờ thêm producer có thể làm chu kỳ dài hơn; kết quả cũng chịu ảnh hưởng của thời gian ngẫu nhiên giữa các lần báo hiệu và thời gian in UART.
Số producer	Số mẫu so sánh	Trung bình	Nhỏ nhất	Lớn nhất
4	              30	11,80 ms	10 ms	14 ms
5	              30	12,07 ms	10 ms	14 ms