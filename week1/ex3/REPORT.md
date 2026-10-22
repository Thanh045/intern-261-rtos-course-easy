# Report — Week 1 / Ex3

`student_id`: 2550517    Commit của PREDICTION.md: 2cdfd15 
## 1. Bảng thống kê (tính từ CSV, không đọc tay)
| # | Delay | Tick | min (ms) | max (ms) | tb (ms) | jitter = max−min (us) | drift tổng (ms) |
|---|---|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 |13,000 ms |13,001 ms |13,000575ms |1 |+0.46 |
| 2 | `xTaskDelayUntil` | 1000 |13,000| 13,001|13,000565 |1 |+0.452 |
| 3 | `vTaskDelay` | 100 | 10,000|10,001 |10,00043625 |1 |−2399,651 |
| 4 | `xTaskDelayUntil` | 100 |10,000 |10,001 |10,00043875 |1 |−2399,649 |

- Script dùng để tính: ex3_stats.py
- CSV gốc: `______` (commit kèm)
- Ảnh: `![](ex3_cfg1.png)` `![](ex3_cfg2.png)` `![](ex3_cfg3.png)` `![](ex3_cfg4.png)`

## 2. Tại sao `vTaskDelay` bị drift
Công thức drift theo `PERIOD_MS` và `WORK_JITTER_US` của **bạn**, rồi so với số đo:

| | Tính theo công thức | Đo được | Lệch |
|---|---|---|---|
| Drift tổng (ms) |800 x 13 = 10400ms |10400,460ms |0.460ms |

## 3. Tại sao tick 100 Hz sai nhiều hơn
`PERIOD_MS` = 13 ; một tick = (13*100)/ 1000 = 10  ms. Chứng minh bằng số đo, không bằng lý thuyết:

## 4. `xTaskDelayUntil` trả về gì
Giá trị trả về nghĩa là gì, và trong 4 lần chạy của bạn nó có bao giờ báo
"đã trễ deadline" không? Bao nhiêu lần / `SAMPLE_COUNT`?
pdTRUE: khi task được thực hiện xong, vẫn còn thời gian cho đến mốc tiếp theo, hàm cho task nghỉ đến mức đó. pdFALSE: khi task thực hiện quá thời điểm mốc tiếp theo, hàm không cho task nghỉ và bắt task làm tiếp
Trong 800 lần gọi xTaskDelayUntil(), có 0 lần trả về pdFALSE. Cả 800 lần task đều còn thời gian chờ đến mốc chạy tiếp theo.
## 5. Lệch so với dự đoán
Mỗi dòng lệch trong bảng → lệch bao nhiêu + nguyên nhân vật lý.
Em dự đoán khoảng giữa hai cạnh là 13 ms. Khi hạ tick xuống 100 Hz, em đo được khoảng 10 ms, ngắn hơn khoảng 3 ms. Nguyên nhân là 13 ms được chuyển thành 1 tick, tương ứng 10 ms.

Với chênh lệch rất nhỏ, cần xét độ phân giải thiết bị đo, độ lệch clock và độ trễ thực thi

## 6. Câu hỏi đánh đổi
Tick 1000 Hz cho độ chính xác tốt hơn nhưng **tốn** gì? Dựa vào dữ liệu của bạn:
đo CPU overhead của tick ISR ở 100 Hz và 1000 Hz (gợi ý: cho một task ưu tiên thấp
nhất đếm vòng lặp trong 1 giây, so sánh 2 cấu hình). Điền số:


| Tick rate | Số vòng lặp idle đếm được trong 1 s | Overhead suy ra (%) |
|---|---|---|
| 100 Hz |1.838.229| 0%|
| 1000 Hz |1.833.996 |0.2303% |

Với ứng dụng `PERIOD_MS` = 13ms của bạn, bạn chọn tick rate nào? Bảo vệ bằng số trên.
Khi tăng tick rate từ 100 Hz lên 1000 Hz, số vòng đếm giảm 4.233 vòng, tương ứng khoảng 0,23%. Đây là chi phí tăng thêm so với cấu hình 100 Hz. Với yêu cầu chu kỳ 13 ms, em chọn 1000 Hz vì biểu diễn được 13 ms, trong khi 100 Hz chỉ chuyển thành 10 ms. Mức giảm khả năng đếm đo được khoảng 0,23% nên em chấp nhận.
