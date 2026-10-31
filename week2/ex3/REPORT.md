# Report — Week 2 / Ex3

`student_id`: 2550517   Commit của PREDICTION.md: f82ecbc

## 1. Ảnh đo
Binary semaphore: `![](inversion_binsem.png)`
Mutex: `![](inversion_mutex.png)`

Trên mỗi ảnh, đánh dấu: thời điểm `task_high` sẵn sàng chạy (t0), thời điểm nó
thực sự chạy (t1). Thời gian block = t1 − t0.

## 2. Bảng thời gian block
| Cấu hình | t0 (us) | t1 (us) | Block (us) | Đạt `DEADLINE_MS`? |
|---|---|---|---|---|
| Binary semaphore |33045|54081 |21036 |không |
| Mutex |41745 |59778|18033 |có |

## 3. Vì sao mutex ngắn hơn
Giải thích bằng priority inheritance: `task_low` được nâng lên ưu tiên nào, khi
nào, và việc đó ngăn `task_mid` chen vào như thế nào.
Khi High chờ mutex, Low được nâng ưu tiên từ 1 lên 3, cao hơn Mid ưu tiên 2. Low tiếp tục chạy và trả tài nguyên trước khi Mid được chạy. Vì vậy, High chờ khoảng 18,033 ms thay vì 21,036 ms.
## 4. task_mid trong lúc inversion
Mô tả trạng thái kênh `task_mid` trên ảnh binary semaphore trong khoảng
inversion — chạy bao nhiêu lần, mỗi lần bao lâu.
Trong lần đo đã chọn, Mid chạy một đoạn từ 34044 µs đến 54049 µs, kéo dài 20005 µs ≈ 20,005 ms. Trong khoảng này, Low vẫn giữ tài nguyên nhưng bị Mid chiếm CPU, khiến High tiếp tục phải chờ.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.
Cấu hình	Dự đoán	Đo được	Lệch
Binary semaphore	21 ms	21,036 ms	+36 µs
Mutex	                18 ms	18,033 ms	+33 µs
Dự đoán chưa tính thời gian thực thi lệnh GPIO, thao tác semaphore/mutex, xử lý ngắt và chuyển ngữ cảnh. Các mốc khởi động task cũng phụ thuộc tick nên không chính xác tuyệt đối theo mili giây.
## 6. Câu hỏi đánh đổi
Với bộ tham số của **bạn**, nếu `MID_WORK_MS` tăng gấp đôi thì thời gian block ở
cấu hình binary semaphore tăng thêm bao nhiêu? Trả lời bằng số từ dữ liệu của
bạn, rồi đo lại để kiểm chứng. `task_high` còn đạt `DEADLINE_MS` không?
Công việc Mid	High chờ trung bình
20 ms	21,03621 ms
40 ms	41,037 ms
Tăng thêm	20,00079 ms


Khi tăng công việc Mid từ 20 lên 40 ms, thời gian High chờ tăng khoảng 20 ms, từ 21,036 ms lên 41,037 ms. Vì binary semaphore không có priority inheritance, Mid tiếp tục chiếm CPU khiến Low chưa trả tài nguyên được. High không đạt deadline 20 ms, vì riêng thời gian chờ đã vượt deadline.