# Report — Week 3 / Ex2

`student_id`: 2550517  Commit của PREDICTION.md: d6572b7

## 1. Nảy phím trước/sau
Ảnh logic analyzer: tín hiệu thô (nhiều cạnh nảy) và tín hiệu "PRESS" sau khi
qua `debounce_timer` (một cạnh sạch).

## 2. Callback bị block — thí nghiệm bước 4
| | Dự đoán | Đo được |
|---|---|---|
| Độ trễ `longpress_callback` khi `debounce_callback` block 50ms |0-50ms | Chưa quan sát thấy trễ trong các lần thử, độ phân giải 1 ms|

Log timestamp (`xTaskGetTickCount()`) của cả hai callback quanh lúc bị block.
PRESS tick=27452
HELD tick=28437
PRESS tick=28516
HELD tick=29501
HELD tick=30501
HELD tick=31501
HELD tick=32501
HELD tick=33501
HELD tick=34501
HELD tick=35501
PRESS tick=36389
HELD tick=37374
HELD tick=38374
PRESS tick=38455
HELD tick=39440
PRESS tick=39540
HELD tick=40525
HELD tick=41525
PRESS tick=42132


## 3. Giải thích daemon task
Giải thích **bằng cơ chế**, không chỉ hiện tượng: vì sao một callback block sẽ
ảnh hưởng timer khác, liên hệ đến timer command queue và việc có đúng MỘT
daemon task xử lý tất cả software timer.
FreeRTOS dùng chung một timer daemon task để xử lý lệnh start/stop/reset trong timer command queue và thực thi callback của các software timer. Khi debounce_callback gọi vTaskDelay(50 ms), daemon bị block: các task khác vẫn chạy, nhưng daemon chưa thể xử lý lệnh timer hoặc chạy callback khác. Timer hết hạn trong khoảng này phải chờ daemon hoạt động lại.
## 4. Long-press
Thời gian giữ thực tế đo được so với `LONGPRESS_MS`: khớp không?
LONGPRESS_MS = 1000 ms. Log ghi nhận PRESS = 27452, HELD đầu tiên tại 28437, chênh lệch 985 ms. Vì PRESS được ghi sau debounce khoảng 15 ms, thời điểm phát hiện long-press tính từ cạnh nhấn khoảng 1000 ms, phù hợp cấu hình. Các HELD tiếp theo cách nhau 1000 ms.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
Debounce cấu hình 15 ms, đo trung bình 14,620 ms. Sai lệch chủ yếu do lượng tử tick 1 ms và thời gian thực thi callback.
Khi bật callback chậm, chưa quan sát thấy HELD trễ ở độ phân giải tick. Khoảng block xảy ra khoảng 15–65 ms sau nhấn, không trùng hạn long-press 1000 ms.
PRESS → HELD là 985 ms, không phải 1000 ms, vì timer long-press bắt đầu từ cạnh nhấn, trước log PRESS khoảng 15 ms.
## 6. Câu hỏi đánh đổi
Với `DEBOUNCE_MS` của bạn, nếu giảm xuống còn một nửa thì tín hiệu sau debounce
còn sạch không? Đo lại trên board của bạn, không suy luận suông.
Khi giảm DEBOUNCE_MS từ 15 xuống 7 ms, tín hiệu sau debounce vẫn sạch trong bản thu thử nghiệm. Tín hiệu thô có 19 cạnh, sau lọc còn 15 cạnh; độ trễ trung bình giảm xuống 6,481 ms. Tuy nhiên, kết quả này chưa bảo đảm lọc được các lần nảy kéo dài hơn 7 ms.