# Dự đoán — Week 3 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517

1. Với nút nhấn cơ khí thông thường, nảy phím thường kéo dài bao lâu
   (tra cứu/ước lượng)? So với `DEBOUNCE_MS` của bạn: đủ lớn để lọc hết không?
Nảy phím thường kéo dài vài ms, có thể ước lượng khoảng 1–10 ms. Khảo sát của Ganssle ghi nhận phần lớn mẫu dưới 6,2 ms, nhưng có ngoại lệ đến 157 ms. Vì vậy, 15 ms hợp lý cho nút thông thường.


2. Nếu `DEBOUNCE_MS` quá lớn, điều gì xảy ra khi người dùng nhấn nhả rất
   nhanh liên tiếp (double-click)?
Mỗi cạnh mới lại khởi động lại thời gian chờ. Nếu người dùng nhấn–nhả–nhấn quá nhanh, trạng thái trung gian chưa ổn định đủ lâu sẽ bị bỏ qua. Double-click có thể bị nhận thành một lần nhấn, đồng thời phản hồi nút chậm hơn.


3. Khi callback của `debounce_timer` bị block 50 ms (thử nghiệm bước 4), bạn
   dự đoán `longpress_timer` sẽ bị trễ bao nhiêu? Giải thích bằng cơ chế
   daemon task + queue lệnh, không chỉ đoán số.
longpress_timer có thể trễ 0–50 ms, tùy thời điểm hết hạn. Hai timer dùng chung daemon task; khi callback debounce block, daemon không chạy callback khác hay xử lý lệnh trong queue. Nếu longpress hết hạn trong khoảng đó, nó phải chờ daemon chạy lại.

4. `LONGPRESS_MS` của bạn là bao nhiêu? Giữ phím đúng ngưỡng này, bạn dự đoán
   callback `longpress_callback` chạy lần đầu ở thời điểm nào tính từ lúc
   nhấn xuống?
LONGPRESS_MS = 1000 ms. Callback đầu tiên dự kiến chạy khoảng 1 giây sau cạnh lên cuối của nút, cộng sai số tick và trễ lập lịch. Nếu nhả đúng ngưỡng này, callback có thể chưa kịp chạy.

