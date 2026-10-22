# Report — Week 1 / Ex2

`student_id`: 2550517    Commit của PREDICTION.md:eb2987c

## 1. CPU share đo được
| Cấu hình | A (%) | B (%) | C (%) | Time slice đo được | Ảnh |
|---|---|---|---|---|---|
| B2: preempt=1, ưu tiên khác nhau |0% |0% |100% | — | |
| B3: preempt=0, ưu tiên khác nhau |0% |0% |100% | — | |
| B4: preempt=1, cùng ưu tiên |33.3% |33.3% |33.3% |1,000043 ms | |
| B5: sau khi sửa starvation |16.63% |42.37% |24.35% | | |

Cách tính CPU share từ ảnh PulseView (mô tả ngắn):Em ước tính CPU share từ tín hiệu GPIO. Khi các mức cao chồng nhau, em quy khoảng thời gian đó cho task có ưu tiên cao nhất. Phần cả ba chân thấp được ghi riêng. Phép đo chưa tách được thời gian ngắt và chi phí ngoài vùng GPIO đánh dấu.

## 2. Preemption nhìn thấy ở đâu
Trên ảnh bước 2, chỉ ra ít nhất **2 thời điểm** preemption xảy ra. Với mỗi thời điểm:
dấu thời gian, task bị cắt, task chiếm CPU, và vì sao.
Ở B2, task C có ưu tiên cao nhất và luôn Ready vì không block hay yield. C được chạy ngay khi scheduler khởi động; A và B không được chạy. Vì vậy, bản thu B2 không có hai thời điểm A/B đang chạy rồi bị C giành CPU. Em ghi nhận yêu cầu này chưa phù hợp với cấu hình B2, không tự điền các thời điểm không quan sát được.

| t (us) | Task bị preempt | Task chiếm CPU | Lý do |
|---|---|---|---|
| | | | |

## 3. Time slice
Time slice đo được ở bước 4: 1.000043 ms. `configTICK_RATE_HZ` = 1000.
Sai số: 0.0043% %. Giải thích sai số: Độ lệch có thể chịu ảnh hưởng của xử lý ngắt, chuyển ngữ cảnh, thời gian từ lúc task được chạy đến cạnh GPIO đầu tiên và độ phân giải lấy mẫu 0,125 µs


## 4. DWT wrap-around
Tại sao so sánh `DWT->CYCCNT > target` bị sai sau ~59 giây, và cách viết đúng:
Bộ đếm DWT có giới hạn và sẽ quay về 0. Vì thế, so trực tiếp số hiện tại với mốc kết thúc có thể nhầm khi bộ đếm quay vòng. Em dùng phép trừ unsigned giữa số hiện tại và số bắt đầu để tính số chu kỳ đã trôi qua, kể cả khi bộ đếm đi qua 0.: 
Cách viết đúng: while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
}

## 5. Sửa starvation
Cách tôi chọn: thêm vTaskDelay(1) sau mỗi vòng của cả ba task và giữ nguyên priority A=1, B=2, C=3
Vì sao nó hoạt động (dựa vào trạng thái task trong scheduler): Khi các task ưu tiên cao chuyển sang Blocked, task thấp hơn có cơ hội chạy
Đánh đổi: task phải chờ, làm giảm tốc độ lặp và thay đổi thời điểm thực hiện công việc.

## 6. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.
Ở B2 và B3, kết quả phù hợp với dự đoán: C chạy liên tục, A/B bị starvation. Ở B4, CPU share ước tính từ dữ liệu GPIO của A/B/C lần lượt là 33,3306%; 33,3349%; 33,3346%, gần bằng nhau. Time slice trung bình ước tính là 1,000043 ms, lệch khoảng 0,043 µs so với dự đoán 1 ms. Chênh lệch nhỏ có thể chịu ảnh hưởng của thời gian chuyển ngữ cảnh, ngắt và độ phân giải lấy mẫu.
Ở B5, cả ba task đều được chạy sau khi thêm vTaskDelay(1), phù hợp với dự đoán sửa được starvation. Các tỷ lệ suy ra từ GPIO chỉ là ước tính

## 7. Câu hỏi đánh đổi
Với **bộ ưu tiên của bạn**, nếu `TASK_C_WORK_US` tăng gấp 3 lần thì bảng CPU share ở
bước 2 thay đổi thế nào? Trả lời bằng số suy ra từ dữ liệu của bạn, rồi **đo lại để
kiểm chứng**. Nếu lệch, giải thích.
Khi tăng thời gian busy-wait của C từ 240 µs lên 720 µs trong cấu hình B2, C vẫn có ưu tiên cao nhất và không block. Vì vậy, dự kiến A/B vẫn chiếm khoảng 0%, C gần 100% thời gian chạy các task ứng dụng. C không tăng CPU share gấp ba vì trước đó đã chiếm gần hết phần thời gian này; thay vào đó, số vòng C thực hiện mỗi giây dự kiến giảm còn khoảng một phần ba, nếu bỏ qua chi phí khác.
Sau khi đo: Khi tăng thời gian busy-wait của C từ 240 µs lên 720 µs ở cấu hình B2, A và B vẫn không có chuyển mức trong 125 ms quan sát. Kết quả phù hợp với dự đoán A/B vẫn bị starvation, còn C chiếm gần hết thời gian chạy các task ứng dụng. Độ rộng mức cao của C trung bình là 723,786 µs; khoảng giữa hai cạnh lên trung bình là 724,402 µs, tương ứng khoảng 1380 vòng/giây. Phần thời gian vượt 720 µs có thể gồm chi phí gọi hàm, điều khiển GPIO và xử lý ngắt.