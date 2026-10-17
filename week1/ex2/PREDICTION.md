# Dự đoán — Week 1 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517   Ưu tiên của tôi: A=1 B=2 C=3

1. Bước 2 (`configUSE_PREEMPTION`=1, ưu tiên khác nhau): task nào chạy? task nào
   **không bao giờ** chạy? CPU share dự đoán:
Task C chạy liên tục vì có ưu tiên cao nhất và không bị blockl A và B sẽ bị starvataion

   | Task | CPU share dự đoán (%) |
   |---|---|
   | A |0% |
   | B |0% |
   | C |100% |

2. Bước 3 (`configUSE_PREEMPTION`=0): dự đoán thay đổi gì so với bước 2? Vì sao?
Dù đã tắt preemption nhưng task C vẫn được ưu tiên chạy trước và không bị blocked hay yield nên kết quả y hệt
3. Bước 4 (3 task cùng ưu tiên): time slice dự đoán = 1ms. Dựa vào đâu?
Do 3 task có cùng độ ưu tiên và còn bật time slicing với chu kì 1000Hz nên mỗi task 1 chu kì
4. Bước 5: tôi sẽ sửa starvation bằng cách: ______ . Cách này đánh đổi gì?
dự kiến thêm vTaskDelay(1) sau mỗi vòng ở cả ba task, giữ nguyên priority. Khi task ưu tiên cao chờ, task thấp hơn có cơ hội chạy. Đánh đổi là giảm tốc độ lặp và thay đổi thời điểm thực hiện.
5. Chân nào sẽ không bao giờ đổi trạng thái trên logic analyzer ở bước 2? ______
Chân không đổi mức ở: PA1 và PA5.
