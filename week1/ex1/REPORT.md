# Report — Week 1 / Ex1

`student_id`: 2550517    Commit của PREDICTION.md: b78e52d

## 1. Số đo
| Đại lượng | Giá trị |
|---|---|
| `BLINK_PERIOD_MS` (từ params.h) | |
| Chu kỳ đo được — min |166.000ms |
| Chu kỳ đo được — max | 166.050ms|
| Chu kỳ đo được — trung bình | 166.017ms|
| Sai số trung bình vs `BLINK_PERIOD_MS` |17us |
| `uxTaskGetStackHighWaterMark()` của bringup_task | words |

Ảnh PulseView (>= 10 chu kỳ, đọc được giá trị): `![](do_ex1.png)`

## 2. Ba handler của kernel
| Tên trong port ARM_CM3 | Tên trong vector table | Nhiệm vụ |
|---|---|---|
| `vPortSVCHandler` |SVC_Handler | khôi phục ngữ cảnh để bắt đầu chạy task đầu tiên của RTOS |
| `xPortPendSVHandler` |PendSV_Handler | Chuyển ngữ cảnh giữa các task|
| `xPortSysTickHandler` |SysTick_Handler |tăng tick kernel, xử lí thời gian chờ và chuyển task khi cần |

Log lỗi link trước khi sửa (dán nguyên văn):
C:/PROGRA~2/ARMGNU~1/14EFD8~1.2RE/bin/../lib/gcc/arm-none-eabi/14.2.1/../../../../arm-none-eabi/bin/ld.exe: freertos-kernel/libfreertos_kernel.a(port.c.obj): in function `SVC_Handler':
D:/RTOS_course/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:230: multiple definition of `SVC_Handler'; CMakeFiles/template.dir/Core/Src/stm32f1xx_it.c.obj:D:/RTOS_course/intern-261-rtos-course-easy/template/Core/Src/stm32f1xx_it.c:146: first defined here
C:/PROGRA~2/ARMGNU~1/14EFD8~1.2RE/bin/../lib/gcc/arm-none-eabi/14.2.1/../../../../arm-none-eabi/bin/ld.exe: freertos-kernel/libfreertos_kernel.a(port.c.obj): in function `PendSV_Handler':
D:/RTOS_course/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:460: multiple definition of `PendSV_Handler'; CMakeFiles/template.dir/Core/Src/stm32f1xx_it.c.obj:D:/RTOS_course/intern-261-rtos-course-easy/template/Core/Src/stm32f1xx_it.c:172: first defined here
C:/PROGRA~2/ARMGNU~1/14EFD8~1.2RE/bin/../lib/gcc/arm-none-eabi/14.2.1/../../../../arm-none-eabi/bin/ld.exe: freertos-kernel/libfreertos_kernel.a(port.c.obj): in function `SysTick_Handler':
D:/RTOS_course/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:494: multiple definition of `SysTick_Handler'; 
CMakeFiles/template.dir/Core/Src/stm32f1xx_it.c.obj:D:/RTOS_course/intern-261-rtos-course-easy/template/Core/Src/stm32f1xx_it.c:185: first defined here
Memory region         Used Size  Region Size  %age Used
             RAM:        6008 B        20 KB     29.34%
           FLASH:       11680 B        64 KB     17.82%
collect2.exe: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.

```
```

Cách sửa:
Xóa các định nghĩa SVC_Handler, PendSV_Handler và SysTick_Handler
trong template/Core/Src/stm32f1xx_it.c, để sử dụng handler do
port FreeRTOS cung cấp thông qua mapping trong FreeRTOSConfig.h.

## 3. Tại sao HAL timebase phải sang TIM4
Phải trả lời được: `HAL_IncTick()` chạy ở đâu, `configKERNEL_INTERRUPT_PRIORITY`
ảnh hưởng gì đến nó, và `HAL_Delay()` / `vTaskDelay()` khác nhau chỗ nào.
Bởi vì mỗi khi TIM4 đếm đủ 1ms, HAL sẽ gọi hàm HAL_TIM_PeriodElapsedCallback() để xác nhận TIMER có phải TIM4 hay không, nếu đúng thì HAL_IncTIck() mới tăng bộ đếm thời gian của HAL thêm 1. configKERNEL_INTERRUPT_PRIORITY sẽ đặt mức ưu tiên ngắt của systick xuống thấp nhất, điều đó có nghĩa nó dễ bị ngắt bởi các sự kiện có độ ưu tiên cao hơn, Nếu HAL cũng dùng SysTick thì việc tăng tick HAL có thể bị trì hoãn theo. Vì vậy, chuyển HAL timebase sang TIM4 để HAL có nguồn tick và mức ưu tiên ngắt riêng, còn SysTick phục vụ FreeRTOS. HAL_Delay() liên tục kiểm tra tick HAL nên vẫn chiếm thời gian CPU; vTaskDelay() đưa task vào trạng thái Blocked, cho phép CPU chạy task khác hoặc Idle task. Vì vậy, task nháy LED của bài dùng vTaskDelay(


## 4. Lệch so với dự đoán
Với mỗi mục trong `PREDICTION.md` lệch so đo: lệch bao nhiêu, và **nguyên nhân vật lý**
(không được trả lời "do sai số").
Em dự đoán chu kỳ đầy đủ của tín hiệu PA0 khoảng 166 ms. Qua các lần đo liên tiếp chu kỳ trung bình là 166,017 ms, lớn hơn dự đoán khoảng 0,017 ms, tức 17 µs.
Tuy nhiên, logic analyzer đang lấy mẫu ở 20 kHz, tương ứng 50 µs giữa hai mẫu, và các con trỏ được đặt bằng tay. Vì vậy, độ lệch quan sát được có thể chịu ảnh hưởng của độ phân giải lấy mẫu và vị trí đặt con trỏ;. Cần đọc thời điểm cạnh từ dữ liệu thu để xác nhận. Các yếu tố có thể ảnh hưởng gồm thời gian thực thi lệnh, độ trễ xử lý ngắt và độ lệch tần số clock của board hoặc thiết bị đo.

## 5. Câu hỏi đánh đổi
`configTICK_RATE_HZ` = 1000 trong bài này. Dựa vào số đo của **bạn**, nếu hạ xuống
100 Hz thì chu kỳ LED và sai số thay đổi thế nào? Trả lời bằng con số, không bằng lý thuyết.
Khi hạ configTICK_RATE_HZ từ 1000 Hz xuống 100 Hz, kết quả đo chu kỳ đầy đủ của LED như sau:
Đại lượng	|1000 Hz	|100 Hz
Chu kỳ trung bình	|166,017 ms	|160,008 ms
Sai lệch so với mục tiêu 166 ms	|+0,017 ms	|−5,992 ms


Như vậy, chu kỳ LED giảm khoảng 6,009 ms. Độ lớn sai số so với mục tiêu 166 ms tăng từ khoảng 17 µs lên 5992 µs, và LED nhấp nháy nhanh hơn.

