# Report — Week 2 / Ex2

`student_id`: 2550517  Commit của PREDICTION.md: e3ec2a6 

## 1. Lỗi tái hiện (UART_GUARD_MODE = 0)
Log UART dán nguyên văn, đánh dấu vị trí ký tự lẫn lộn:


WWWW1230    ####00000000000000001111333377771111    CDABCDABCDABCDABCDABCDABCDABCDABCDAB



WWWW2301    ####00000000000000001111333377772222    CDABCDABCDABCDABCDABCDABCDABCDABCDAB



WWWW2301    ####00000000000000001111333377773333    CDABCDABCDABCDABCDABCDABCDABCDABCDAB



WWWW2301    ####00000000000000001111333377774444    CDABCDABCDABCDABCDABCDABCDABCDABCDAB



WWWW2301    ####00000000000000001111333377775555    CDABCDABCDABCDABCDABCDABCDABCDABCDAB



WWWW2301    ####00000000000000001111333377776666    DABCDABCDABCDABCDABCDABCDABCDABCDABC



WWWW3012    ####00000000000000001111333377777777    DABCDABCDABCDABCDABCDABCDABCDABCDABC



WWWW3012    ####00000000000000001111333377778888    DABCDABCDABCDABCDABCDABCDABCDABCDABC



WWWW3012    ####00000000000000001111333377779999    DABCDABCDABCDABCDABCDABCDABCDABCDABC



WWWW3012    ####00000000000000001111333388880000    DABCDABCDABCDABCDABCDABCDABCDABCDABC



WWWW3012    ####00000000000000001111333388881111    ABCDABCDABCDABCDABCDABCDABCDABCDABCD



WWWW0123    ####00000000000000001111333388882222    ABCDABCDABCDABCDABCDABCDABCDABCDABCD



WWWW0123    ####00000000000000001111333388883333    ABCDABCDABCDABCDABCDABCDABCDABCDABCD



WWWW0123    ####00000000000000001111333388884444    ABCDABCDABCDABCDABCDABCDABCDABCDABCD



WWWW0123    ####00000000000000001111333388885555    ABCDABCDABCDABCDABCDABCDABCDABCDABCD



WWWW0123    ####00000000000000001111333388886666    BCDABCDABCDABCDABCDABCDABCDABCDABCDA


Tái hiện lại bao nhiêu lần / bao nhiêu lần thấy lỗi: 3/ 3.

## 2. Bảng so sánh 3 cách
| Cách | Độ trễ in trung bình (us) | Độ trễ in max (us) | Ảnh hưởng task khác | RAM thêm (bytes) |
|---|---|---|---|---|
| Mutex |2890,43 |3340 |1005,708 | 80|
| Critical section |5853,22	 |9367|2402,569|0 |
| Gatekeeper |2326,64 |2338,69 |1012,944 |1320 |

Cách đo độ trễ in (mô tả ngắn, kèm ảnh logic analyzer nếu dùng pin đánh dấu):
Đại lượng	Cách đo
Latency từ lúc writer muốn gửi đến lúc UART gửi xong	Đánh dấu GPIO hoặc đo bằng DWT
Critical section làm task/ngắt khác trễ bao lâu	Thêm một task kiểm tra nhịp và so sánh
RAM tăng thêm	Đối chiếu .map và heap trống của từng mode

## 3. Vì sao critical section nguy hiểm nhất
Giải thích bằng số đo của bạn: đoạn in dài bao nhiêu us khi ở trong critical
section, và điều đó chặn ngắt nào trong hệ thống bao lâu.
Đoạn truyền trong critical section kéo dài khoảng 2,3 ms, làm SysTick và các ngắt bị che phải chờ. Task probe cần chạy mỗi 1 ms nhưng có khoảng chạy lên tới 2,403 ms, cho thấy hệ thống phản hồi chậm hơn
## 4. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.
Critical section có latency toàn hàm lớn vì sau khi mở khóa, writer có thể bị chuyển CPU trước khi kéo GPIO xuống. Gatekeeper ưu tiên 2 cao hơn writer ưu tiên 1 nên thường được chạy ngay khi có message.
## 5. Câu hỏi đánh đổi
Với `WRITER_COUNT` của **bạn**, nếu tăng gấp đôi thì cách nào bị ảnh hưởng độ trễ
nhiều nhất? Trả lời bằng số từ dữ liệu của bạn, rồi đo lại để kiểm chứng.
Cách	Trung bình 4 writer (µs)	Trung bình 8 writer (µs)	Mức tăng (µs)
Mutex	2890,43	12602,15	9711,72
Critical section	5853,22	10617,99	4764,77
Gatekeeper	2326,64	2340,56	13,92


Khi tăng từ 4 lên 8 writer, mutex có latency trung bình tăng nhiều nhất trong các lần đo, khoảng 9,71 ms, do nhiều writer phải chờ quyền sử dụng UART. Critical section tăng khoảng 4,76 ms. Gatekeeper tăng khoảng 13,92 µs; task gatekeeper có ưu tiên cao hơn writer nên thường truyền ngay khi nhận message, đồng thời trì hoãn các writer tạo yêu cầu tiếp theo.
