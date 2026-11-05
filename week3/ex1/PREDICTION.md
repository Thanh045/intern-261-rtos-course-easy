# Dự đoán — Week 3 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517

1. Với `ISR_NVIC_PRIO` của bạn và nhóm priority `NVIC_PRIORITYGROUP_4`, số này
   nằm trong khoảng cho phép (`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` ..
   `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`) không? Trình bày phép so sánh.
                                                  
STM32F103 có 4 bit priority NVIC, tương ứng 16 mức từ 0 đến 15.
NVIC_PRIORITYGROUP_4 dành cả 4 bit cho preemption priority.
Cấu hình:
- configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5
- ISR_NVIC_PRIO = 6
- configLIBRARY_LOWEST_INTERRUPT_PRIORITY = 15
Vì 5 <= 6 <= 15 nên priority 6 hợp lệ để gọi API FreeRTOS FromISR.

2.Khi đặt priority EXTI cao hơn ngưỡng cho phép rồi gọi
   `vTaskNotifyGiveFromISR`, bạn dự đoán hành vi quan sát được là gì (reset?
   treo? thông báo gì trên UART/debugger)? 
Chế độ thử lỗi đặt priority EXTI1 = 4.
Số 4 nhỏ hơn 5 nên ngắt có ưu tiên phần cứng cao hơn ngưỡng
được phép gọi API FreeRTOS.
Khi nhấn nút và ISR gọi vTaskNotifyGiveFromISR(), dự đoán
configASSERT trong phần kiểm tra priority ngắt của port sẽ kích hoạt.
Với hàm xử lý lỗi hiện tại:
- Chương trình lưu tên file và dòng assert.
- fault_code = 1.
- Tắt ngắt và dừng trong vòng lặp vô hạn.
- Không tự reset và không tự in thông báo UART.
- Có thể đọc fault_code, tep_assert, dong_assert và backtrace bằng GDB.

3.Dự đoán latency ISR → task (có `portYIELD_FROM_ISR`) theo đơn vị us, dựa
   trên tần số CPU 72 MHz và số lệnh ước lượng trong đường đi ISR → context
   switch. So với `LATENCY_BUDGET_US`: đạt hay không?
CPU chạy 72 MHz, tương ứng 72 cycle/us.
Ước lượng toàn bộ đường đi từ cạnh nút đến GPIO ACK gồm:
vào ngắt, xử lý HAL/EXTI, gửi notification, thoát ISR,
chuyển ngữ cảnh và toggle GPIO
Giả sử đường đi tốn khoảng 1000–3000 cycle:
- 1000 / 72 ≈ 13,9 us.
- 3000 / 72 ≈ 41,7 us.
Dự đoán latency cỡ 25us 
Dự kiến đạt LATENCY_BUDGET_US = 50 us trong điều kiện ít tải.

4.Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ chạy **khi nào** (ngay lập tức
   hay phải chờ tick ngắt kế tiếp)? Latency tăng thêm cỡ bao nhiêu?
ACK chuyển sang Ready nhưng chưa chắc chạy ngay; thường chờ lần lập lịch tiếp theo. Với tick 1 ms, có thể tăng khoảng 0–1000 µs, nhưng có thể sớm hơn nếu có nguyên nhân lập lịch khác.
