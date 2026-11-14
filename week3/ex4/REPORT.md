# Report — Week 3 / Ex4

`student_id`: 2550517    Commit của PREDICTION.md: 1c02039

## 1. Bảng CPU %
| Task | CPU % (lần đo 1) | CPU % (lần đo 2) |
|---|---|---|
| cpu_stats |1% |1% |
| Idle|99%|98%

Giải thích Idle task chiếm bao nhiêu % và vì sao.
| cpu_stats | Idle task chiếm khoảng 98–99% CPU vì phần lớn các task đang ở trạng thái Blocked hoặc Delay nên không cần sử dụng CPU.
Đặc biệt, task_alpha và task_beta bị deadlock do chờ mutex của nhau, không thể tiếp tục thực hiện công việc. Các task còn lại chủ yếu hoạt động định kỳ và dành phần lớn thời gian để chờ.
Do đó, FreeRTOS dành phần lớn thời gian CPU còn trống cho Idle task.
## 2. Bằng chứng tìm deadlock
Log/ảnh `uxTaskGetSystemState` **trước khi sửa**: trạng thái từng task, chỉ
rõ `task_alpha`/`task_beta` đang `eBlocked` và đang chờ mutex nào. Giải thích
chính xác thứ tự lấy mutex nào gây ra deadlock (AB-BA).
Kết quả uxTaskGetSystemState() cho thấy task_alpha và task_beta đều ở trạng thái Blocked.
Nguyên nhân là task_alpha lấy mutex A trước rồi chờ mutex B, trong khi task_beta lấy mutex B trước rồi chờ mutex A. Hai task giữ mutex mà task còn lại cần, tạo thành vòng chờ AB–BA và gây deadlock.
Số vòng xử lý của cả hai task đều bằng 0 và không tăng qua các lần thống kê, chứng tỏ chúng không thể tiếp tục hoàn thành công việc.
Trong khi đó, các task khác vẫn hoạt động, cho thấy deadlock chỉ ảnh hưởng trực tiếp đến hai task Alpha và Beta.

## 3. Sửa lỗi
Mô tả thay đổi bạn làm để hết deadlock (thứ tự lấy mutex thống nhất). Xác
nhận chạy ổn định sau khi sửa (thời gian chạy thử).
Để khắc phục deadlock, tôi đã thống nhất thứ tự lấy mutex của cả task_alpha và task_beta thành A → B, thay vì lấy ngược thứ tự A → B và B → A như ban đầu.
Sau khi sửa và nạp lại chương trình, số vòng xử lý của cả hai task liên tục tăng từ 1143 lên 1225 rồi 1306. Đồng thời, log cho thấy cả hai không đang chờ mutex tại các thời điểm lấy mẫu.
Mặc dù trạng thái hai task vẫn hiển thị Blocked do sử dụng vTaskDelay(), đây là trạng thái chờ bình thường, không phải deadlock.
Kết quả chứng minh lỗi deadlock đã được khắc phục trong thời gian quan sát.

## 4. Watchdog
Phép tính reload IWDG. Log/video board tự reset khi deadlock được tái tạo lại
có chủ đích, kèm thời gian từ lúc treo đến lúc reset — so với
`IWDG_TIMEOUT_MS`.
Với IWDG_TIMEOUT_MS = 1000 ms, Prescaler = 64 và LSI ≈ 40 kHz, giá trị Reload tính được là 624.
Khi cố tình tái tạo deadlock, task_alpha và task_beta ngừng gửi heartbeat. Supervisor phát hiện thiếu heartbeat và không refresh IWDG, khiến STM32 tự động reset.
Kiểm tra bằng GDB cho thấy reset_do_iwdg = 1, xác nhận lần reset trước do IWDG.
Kết quả đo bằng Python qua UART:
Trung bình: 697,2 ms.
Thời gian đo được ngắn hơn timeout lý thuyết vì watchdog đã bắt đầu đếm trước khi supervisor phát hiện thiếu heartbeat. Phép đo còn bao gồm thời gian khởi động lại STM32 và gửi thông báo BOOT.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
Kết quả thực tế nhìn chung phù hợp với dự đoán.
Deadlock: Xảy ra ngay chu kỳ đầu thay vì sau vài chu kỳ do hai task lấy mutex ngược thứ tự.
Trạng thái: Đúng dự đoán, Alpha và Beta đều Blocked, chờ mutex của nhau.
Watchdog: Dự đoán 1000 ms, thực tế đo trung bình 697,2 ms từ DEADLOCK đến BOOT do watchdog đã đếm trước khi phát hiện lỗi.
Sau khi sửa thứ tự lấy mutex A → B, hai task hoạt động ổn định, không còn deadlock trong thời gian quan sát.
## 6. Câu hỏi đánh đổi
Với `SUPERVISOR_BITS` và chu kỳ check-in của bạn, nếu một task thật sự chạy
chậm (không deadlock, chỉ chậm) trong đúng 1 chu kỳ, supervisor có false-trip
(reset oan) không? Trả lời bằng số đo thời gian thực tế của chu kỳ check-in.

Với SUPERVISOR_BITS = 4, supervisor chờ heartbeat tối đa 300 ms và IWDG timeout khoảng 1000 ms.
Khi tăng chu kỳ check-in của task_phu_1 lên 400 ms, GDB đo được chu kỳ thực tế là 400 ms. Trong thời gian quan sát, Alpha và Beta vẫn hoạt động bình thường, không ghi nhận reset oan.
Như vậy, task check-in chậm hơn thời gian chờ 300 ms có thể khiến supervisor báo thiếu heartbeat, nhưng không nhất thiết gây reset vì IWDG vẫn có thể được refresh trước khi hết hạn.
Trong thời gian quan sát, chưa ghi nhận reset oan. Điều này cho thấy một lần chờ heartbeat vượt 300 ms không nhất thiết khiến IWDG reset, vì watchdog có timeout khoảng 1000 ms và vẫn có thể được refresh ở lượt kiểm tra sau.
