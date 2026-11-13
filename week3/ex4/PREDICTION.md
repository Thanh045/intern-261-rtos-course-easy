# Dự đoán — Week 3 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517

1.Chạy `app/deadlock_demo.c` y nguyên như được cấp. Dự đoán: hệ thống sẽ
   treo sau khoảng bao lâu (ngay lập tức, hay sau vài chu kỳ)? Vì sao không
   phải ngay lập tức (gợi ý: cần cả 2 task "đua" vào đúng thời điểm)?
Deadlock có thể xảy ra ngay trong những chu kỳ đầu, nhưng không nhất thiết ngay lập tức. Nguyên nhân là task_alpha phải giữ mutex A và task_beta giữ mutex B cùng thời điểm, sau đó cả hai cùng yêu cầu mutex còn lại. Nếu một task lấy được cả hai mutex trước thì deadlock chưa xảy ra.

2.Khi treo, `uxTaskGetSystemState` sẽ báo trạng thái gì cho `task_alpha` và
   `task_beta`? Dự đoán tên mutex mỗi task đang chờ (dựa trên đọc code, chưa
   chạy công cụ).
Dự đoán uxTaskGetSystemState() sẽ báo:
task_alpha: Blocked, đang giữ mutex A và chờ mutex B.
task_beta: Blocked, đang giữ mutex B và chờ mutex A.
Hai task chờ nhau vô thời hạn nên không thể tiếp tục thực hiện công việc.

3. Tính reload value cho IWDG với `IWDG_TIMEOUT_MS` của bạn (LSI ~40 kHz).
4.Sau khi sửa lỗi deadlock và thêm supervisor + IWDG, nếu bạn cố tình tái
Với IWDG_TIMEOUT_MS = 1000 ms, tần số LSI xấp xỉ 40 kHz và Prescaler = 64:
Reload = (1000 x 10^-3 x 40000) / 64  - 1 = 624

  tạo lại tình huống deadlock cũ (tạm thời), dự đoán board sẽ phản ứng thế
   nào và sau bao lâu.

Khi deadlock xảy ra, task_alpha và task_beta không thể hoàn thành công việc nên ngừng gửi heartbeat.
supervisor_task phát hiện thiếu heartbeat và không gọi HAL_IWDG_Refresh(). Khi bộ đếm IWDG hết hạn, STM32 tự động reset và khởi động lại chương trình.
Dự đoán board sẽ reset trong vòng khoảng 1 giây kể từ lần refresh watchdog cuối cùng, với giả định LSI khoảng 40 kHz.