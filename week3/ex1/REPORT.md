# Report — Week 3 / Ex1

`student_id`: 2550517   Commit của PREDICTION.md: bcd0170 

## 1. Bằng chứng configASSERT
Mô tả/log/ảnh lúc priority sai (bước 4 README) + giải thích chính xác **vì sao**
assert được kích hoạt (nêu đúng tên macro và điều kiện).
Khi đặt THU_PRIORITY_SAI = 1, EXTI1 được cấu hình priority 4, trong khi ngưỡng cho phép gọi API FreeRTOS là 5. Vì 4 < 5, ISR có mức ưu tiên quá cao nhưng vẫn gọi vTaskNotifyGiveFromISR().
FreeRTOS kiểm tra trong vPortValidateInterruptPriority():
configASSERT(ucCurrentPriority >= ucMaxSysCallPriority);
Điều kiện sai nên chương trình dừng tại hàm xử lý assert. GDB ghi nhận:
- fault_code = 1.
- dong_assert = 805.
- File: portable/GCC/ARM_CM3/port.c.
- Call stack đi vào dung_khi_assert() rồi dung_khi_loi().
Kết luận: Assert đã phát hiện cấu hình priority không hợp lệ. Chương trình dừng để kiểm tra lỗi, không tự reset.
## 2. Latency ISR → task
| Cấu hình | Dự đoán (us) | Đo được (us) | Đạt `LATENCY_BUDGET_US`? |
|---|---|---|---|
| Có `portYIELD_FROM_ISR` |25 |15.393 |ĐẠT |
| Không có `portYIELD_FROM_ISR` |0-1000, trung bình 500 |501.284 |KHÔNG ĐẠT |

Ảnh logic analyzer đính kèm cho cả 2 cấu hình.

## 3. Giải thích quan hệ priority
Giải thích bằng lời của bạn: số NVIC priority nhỏ/lớn nghĩa là gì, vì sao
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` tồn tại, điều gì xảy ra nếu một
ISR ưu tiên cao hơn ngưỡng này gọi API FreeRTOS.
Số NVIC priority càng nhỏ thì ưu tiên ngắt càng cao. Ngưỡng configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5 giúp bảo vệ dữ liệu kernel trong critical section. ISR dùng API FromISR phải có priority từ 5 đến 15. EXTI priority 6 hợp lệ; priority 4 vi phạm nên kích hoạt configASSERT.
## 4. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý (jitter NVIC, pipeline, v.v.).
Có yield: dự đoán 25 µs, đo khoảng 15,393 µs, thấp hơn 9,6 µs. Ước lượng ban đầu cao hơn thời gian thực thi thực tế của đường xử lý ISR và chuyển task.
Không yield: tất cả lần đo nằm trong khoảng dự đoán 0–1000 µs. Trung bình 11 lần là 501,284 µs, gần dự đoán 500 µs. Độ trễ thay đổi chủ yếu do thời điểm nhấn so với tick tiếp theo.
## 5. Câu hỏi đánh đổi
Nếu tăng `ISR_NVIC_PRIO` của bạn lên một bậc (ưu tiên thấp hơn) trong khi vẫn
hợp lệ, latency đo được thay đổi bao nhiêu? Trả lời bằng số đo thật, không
đoán
Khi tăng ISR_NVIC_PRIO từ 6 lên 7, vẫn bật portYIELD_FROM_ISR, latency đo lần này là 15,5 µs, so với khoảng 15,4 µs trước đó: tăng khoảng 0,1 µs.
Chênh lệch này nhỏ hơn một bước lấy mẫu ở 8 MHz (0,125 µs), nên chưa thấy latency thay đổi đáng kể. Cả hai cấu hình đều đạt budget 50 µs.
Priority 7 có ưu tiên thấp hơn 6, nhưng nếu không có ngắt cạnh tranh thì thời gian xử lý gần như giữ nguyên..

