# Dự đoán — Week 1 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517   `HEAP_SIZE_BYTES` = 10240   `DYN_TASK_STACK_WORDS` = 160
`STATIC_TASK_STACK_WORDS` = 160   `HOG_DEPTH_BYTES` = 512    `TARGET_FREE_HEAP_BYTES` = 2048

1. Ngân sách RAM dự đoán (tổng phải = 20480 bytes):

| Mục | Bytes dự đoán |
|---|---|
| `.data` |256 |
| `.bss` (không kể heap FreeRTOS) |2200 |
| Heap FreeRTOS (`ucHeap`) |10240 |
| Stack của main (MSP) |1024 |
| Còn lại / chưa dùng |6760 |
| **Tổng** | **20480** |

2. `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy: 7200 bytes.
   `HEAP_SIZE_BYTES` − số này = 3040. Số đó là của những gì?
Phần này gồm stack và TCB của các task tạo động, queue của timer và chi phí quản lý bộ cấp phát.
3. High water mark dự đoán (words): dyn 100, stat 100 , idle 80 , timer 80 .
4. `dyn_task` có stack  160 words = 640 bytes. Mảng `HOG_DEPTH_BYTES` = 512 bytes.
   Hook tràn stack **có** chạy không? Vì sao?
Em dự đoán stack có nguy cơ thiếu, nhưng chưa chắc hook sẽ chạy vì còn phụ thuộc lượng stack thực tế sử dụng và cách compiler sinh mã
5. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì:
6.  .map cho biết toàn bộ vùng ucHeap dành cho FreeRTOS, bằng 10240 bytes. xPortGetFreeHeapSize() chỉ cho biết phần còn trống sau khi cấp phát. Chênh lệch gồm các vùng đã cấp phát, phần quản lý block, căn chỉnh và phần quản lý của heap_4
6. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: 96 words. Dựa vào đâu? Nếu task dùng khoảng 60 words như dự đoán HWM ban đầu, stack 96 words sẽ còn khoảng 36 words, lớn hơn biên yêu cầu 32 words, Giá trị này dự đoán cho task chạy bình thường, không bật mảng thử tràn 512 bytes.
