# Dự đoán — Week 2 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517    `LOW_HOLD_MS` = 20 `MID_WORK_MS` = 20
`HIGH_PERIOD_MS` = 50 `DEADLINE_MS` = 20

1. Cấu hình binary semaphore: thời gian `task_high` bị block dự đoán =   21 ms.
   Phép tính dựa trên đâu (liệt kê các thành phần cộng lại)?
0 ms: Low lấy tài nguyên, bắt đầu chờ 20 ms.
2 ms: High thức dậy, muốn lấy tài nguyên nhưng Low đang giữ → High bị block.
3 ms: Mid thức dậy, chiếm CPU của Low trong 20 ms.
23 ms: Mid nghỉ, Low chạy tiếp và trả tài nguyên → High mới lấy được.
23 − 2 = 21 ms
2. `task_high` có đạt `DEADLINE_MS` ở cấu hình binary semaphore không? Vì sao?
 Chỉ riêng thời gian chờ đã khoảng 21 ms, chưa kể khoảng 0,1 ms làm việc và chi phí thực thi.
3. Cấu hình mutex: thời gian `task_high` bị block dự đoán = 18 ms. Vì sao
   ngắn hơn (hoặc không) so với câu 1? 
20 ms Low giữ tài nguyên − 2 ms trước khi High yêu cầu
≈ 18 ms, vì có nâng độ ưu tiên của low bằng high nên mid không xen ngang được
4. Trên ảnh logic analyzer ở cấu hình binary semaphore, kênh `task_mid` sẽ ở
   trạng thái nào trong lúc `task_high` bị chặn — chạy liên tục, chạy ngắt
   quãng, hay không chạy? Vì sao? 
Mid dự đoán chạy một đoạn liên tục khoảng 20 ms trong lúc High chờ.
High đang Blocked, còn Low có ưu tiên thấp hơn Mid nên bị Mid chiếm CPU. “Liên tục” ở đây vẫn có thể bị ngắt phần cứng xen vào.
5. Nếu đổi `task_mid` sang ưu tiên **thấp hơn** `task_low`, inversion còn xảy ra
   không? Vì sao (đây là cách phân biệt priority inversion với việc chỉ đơn
   giản là "task_low chạy lâu"). 
Nếu Mid có ưu tiên thấp hơn Low, phần kéo dài do Mid sẽ không còn. Low tiếp tục làm việc và trả tài nguyên mà không bị Mid chen vào. High vẫn phải chờ Low giữ tài nguyên xong, nhưng đó là chờ tài nguyên bình thường, không phải phần chờ tăng thêm do task Mid.
