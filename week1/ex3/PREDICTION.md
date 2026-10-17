# Dự đoán — Week 1 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517    `PERIOD_MS` = 13   `WORK_JITTER_US` = 200    `SAMPLE_COUNT` = 800

1. Bảng dự đoán (điền trước khi đo):

| # | Delay | Tick | Chu kỳ tb (ms) | Jitter (us) | Drift tổng sau SAMPLE_COUNT (ms) |
|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 |13 |<1000 | Gần 0, có thể lệch|
| 2 | `xTaskDelayUntil` | 1000 |13 |<1000 | Gần 0 |
| 3 | `vTaskDelay` | 100 | 10 | <10000 | -2400|
| 4 | `xTaskDelayUntil` | 100 | 10|<10000 |-2400 |

2. Cấu hình nào có drift lớn nhất? Vì sao, viết bằng công thức có `WORK_JITTER_US`.
Em dự đoán hai cấu hình 100 Hz có độ lớn drift lớn nhất khi so với mục tiêu 13 ms, khoảng −2400 ms sau 800 khoảng:
800 × (10 − 13) = −2400 ms.
Tải giả mỗi vòng nằm trong [0, WORK_JITTER_US] = [0, 200] µs, nhỏ hơn một tick ở cả hai cấu hình. Vì vậy, em không dự đoán drift của vTaskDelay() đơn giản bằng tổng thời gian tải giả; cần đo để kiểm chứng ảnh hưởng của vị trí gọi delay so với tick.
3. `PERIOD_MS` = 13ms . Với tick 100 Hz, một tick = 10 ms, nên chu kỳ thực tế sẽ
   bị làm tròn thành 10 ms. Sai số hệ thống: -23.08 %.
4. Jitter ở tick 1000 Hz so với 100 Hz: lớn hơn / nhỏ hơn / bằng? Vì sao?
Em dự đoán jitter ở 1000 Hz nhỏ hơn vì tick có độ phân giải 1 ms thay vì 10 ms. Tuy nhiên, tick thấp không nhất thiết làm jitter thực đo lớn hơn; có thể các khoảng đều gần 10 ms nhưng lệch mục tiêu 13 ms. Em sẽ kiểm chứng bằng số liệu.
5. Nếu task này **không** phải ưu tiên cao nhất, đại lượng nào trong bảng bị ảnh hưởng
   nhiều nhất? 
Em dự đoán jitter bị ảnh hưởng rõ nhất, vì thời gian task phải chờ CPU có thể khác nhau giữa các lần chạy. Chu kỳ trung bình và drift cũng có thể bị ảnh hưởng nếu task ưu tiên cao hơn chiếm CPU lâu.
