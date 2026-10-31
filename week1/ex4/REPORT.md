# Report — Week 1 / Ex4

`student_id`: 2550517   Commit của PREDICTION.md: f931932

## 1. Ngân sách RAM từ `.map`
| Mục | Symbol / section trong `.map` | Bytes |
|---|---|---|
| `.data` |.data, kích thước 0x60 |96 |
| `.bss` (không kể heap) |0x3334 − 0x2800 |2868 |
| Heap FreeRTOS (`ucHeap`) |ucHeap, kích thước 0x2800 |10240 |
| Stack của main (MSP) |_Min_Stack_Size = 0x400|1024 |
| Còn lại / chưa dùng | 20480 − 14228 |6252 |
| **Tổng** | | **20480** |

`.map` commit tại: `D:\RTOS_course\intern-261-rtos-course-easy\week1\ex4\template.map`

## 2. Số lúc chạy (dán nguyên văn log UART)
```--- RAM REPORT ---
free_heap=7352 bytes
minimum_ever_free_heap=7352 bytes
target_free_heap=2048 bytes
heap_target_met=1
dyn_task: HWM=135 words, 540 bytes
stat_task: HWM=136 words, 544 bytes
ram_report_task: HWM=364 words, 1456 bytes
idle_task: HWM=110 words, 440 bytes
timer_task: HWM=84 words, 336 bytes
uart_error_count=0

```

| Đại lượng | Words | Bytes |
|---|---|---|
| `xPortGetFreeHeapSize()` | — |7352 |
| `xPortGetMinimumEverFreeHeapSize()` | — |7352
| HWM `dyn_task` |135 |540 |
| HWM `stat_task` |136 |544 |
| HWM `ram_report_task` |364 |1456 |
| HWM idle task |110 |440 |
| HWM timer task |84 |336 |

Heap còn trống >= `TARGET_FREE_HEAP_BYTES` (7348)> 2048

## 3. Đối chiếu `.map` vs runtime
Heap trong `.map` = 10240; `xPortGetFreeHeapSize()` = 7352 ; lệch = 2888 bytes.
Lệch này gồm những gì (liệt kê từng mục kèm số bytes): 
Stack dyn_task	 640
Stack ram_report_task	2048
Hai TCB	152
Header và căn chỉnh các block cấp phát	40
Block đánh dấu cuối heap	8
Tổng	2888

## 4. Dynamic vs static
HWM của `dyn_task` và `stat_task` lệch 1  words dù làm cùng một việc. Vì sao?
Chênh lệch nhỏ này có thể do vị trí căn chỉnh stack và đường thực thi có thể ảnh hưởng HWM.
`xTaskCreate` và `xTaskCreateStatic` khác nhau ở đâu về **nơi** TCB và stack nằm?
xTaskCreate() lấy bộ nhớ cho TCB và stack từ heap FreeRTOS. xTaskCreateStatic() sử dụng TCB và mảng stack do ứng dụng cung cấp; trong chương trình này chúng nằm trong .bss.

## 5. Tràn stack
Log/ảnh chứng minh hook đã chạy:
Ex1.4Hook.png
Tại sao trong `vApplicationStackOverflowHook` không được gọi hàm UART blocking
(hoặc: nếu bạn vẫn gọi được, giải thích tại sao nó không crash — và tại sao đó là may):
Vì khi hook chạy, hệ thống đã phát hiện Stack lỗi, nên cần có cách báo hiệu nhanh và đơn giản nhất:
Gọi UART blocking có hai rủi ro:
- Cần thêm stack và dùng trạng thái HAL, trong khi bộ nhớ có thể đã bị ghi hỏng.
- Có thể chờ mãi: hook của bạn tắt ngắt nên TIM4 không tăng tick HAL nữa. Nếu UART gặp lỗi, timeout trong HAL_UART_Transmit() có thể không hết.


## 6. Stack nhỏ nhất an toàn
| Lần thử | Stack (words) | HWM (words) | Kết quả |
|---|---|---|---|
|1 |160|135 |chạy được, chưa bật hog |
|2 |160| |không chạy được, đã bật hog |
|3 |64|39|chạy được |
|4 |56|31|chạy được, chưa bật hog
|5|57|33|chạy được

Giá trị cuối cùng c nhưng chưa đủ HWM 32 byte wordss. Biên an toàn: ____ words. Lý do chọn biên đó

Em chọn 57 words = 228 bytes, với HWM đo được 33 words = 132 bytes. Đây là cho công việc bình thường, không bật hog, trong cấu hình build hiện tại. Mức 56 words chỉ còn 31 words nên không đạt.
## 7. Lệch so với dự đoán
Mỗi dòng lệch + nguyên nhân.
Đại lượng	Dự đoán	Đo được	Lệch
Heap trống	7200 bytes	7352 bytes	+152 bytes
HWM dyn	100 words	135 words	+35 words
HWM stat	100 words	136 words	+36 words
HWM idle	80 words	110 words	+30 words
HWM timer	80 words	84 words	+4 words


Heap lệch vì ước tính ban đầu chưa tính chính xác TCB, căn chỉnh và vị trí queue timer. HWM lớn hơn dự đoán nghĩa là công việc thực tế dùng ít stack hơn mình ước lượng.
## 8. Câu hỏi đánh đổi
Với `HEAP_SIZE_BYTES` = ____ của bạn: nếu phải thêm một task nữa có stack 128 words,
**lấy RAM từ đâu**? Dựa 7760 bytes bảng ngân sách của bạn, đưa ra 2 phương án kèm chi phí
cụ thể (bytes), rồi chọn một và **thực hiện, đo lại để chứng minh**.

Phương án	RAM lấy từ đâu?	Chi phí dự kiến
Tạo động bằng xTaskCreate()	Heap FreeRTOS còn trống	608 bytes: stack 512 + TCB 76 + overhead 20
Tạo tĩnh bằng xTaskCreateStatic()	Vùng .bss ngoài heap FreeRTOS	588 bytes: stack 512 + TCB 76, chưa kể căn chỉnh
Em chọn tạo động vì heap còn đủ. Trước khi thêm task, với stack dyn_task đã giảm xuống 57 words, heap trống là 7760 bytes. Sau khi thêm task stack 128 words, heap trống đo được là 7152 bytes, giảm 608 bytes. Phần giảm gồm stack 512 bytes, TCB 76 bytes và overhead cấp phát/căn chỉnh 20 bytes. Heap còn trống vẫn đạt yêu cầu 7152 ≥ 2048 bytes.

-- RAM REPORT ---
free_heap=7152 bytes
minimum_ever_free_heap=7152 bytes
target_free_heap=2048 bytes
heap_target_met=1
dyn_task: HWM=33 words, 132 bytes
stat_task: HWM=136 words, 544 bytes
ram_report_task: HWM=360 words, 1440 bytes
idle_task: HWM=110 words, 440 bytes
timer_task: HWM=84 words, 336 bytes
uart_error_count=0


