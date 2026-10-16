# Dự đoán — Week 1 / Ex1

> **Commit file này TRƯỚC khi nạp chương trình lần đầu.** Report sẽ phải giải thích
> mọi chỗ dự đoán lệch so đo. Dự đoán sai không bị trừ điểm; dự đoán sau khi đo thì bị.

`student_id`: 2550517   Ngày/giờ commit: 10:15 16/10/2026
1. Chu kỳ LED tôi sẽ đo được: 166 ms (`BLINK_PERIOD_MS` = 166 ms)
2. Sai số giữa chu kỳ đo được và `BLINK_PERIOD_MS`: 1000 us. Vì sao?
Vì FreeRTOS dùng một đồng hồ chung là tiếng tick, nên sẽ có độ lệch phụ thuộc vào thời điểm gọi vTaskDelay so với tick
3. Nếu để HAL timebase ở lại SysTick, điều gì sẽ xảy ra khi `vTaskDelay()` được gọi?
   (chọn và giải thích: chạy bình thường / treo / tick sai nhịp / hard fault)
em dự đoán những task gọi hàm vTaskDelay() sẽ bị treo mãi vì systick chỉ làm tăng việc đếm tick của HAL_Inctick() hoặc ngược lại, những task có HAL_Delay sẽ bị treo so systick chỉ làm tăng tick đếm của FreeRTOS
4. `BRINGUP_STACK_WORDS` = 128 words = 512 bytes. Task này có đủ không? Vì sao?
Theo em, với lượng words là 128 ( bằng 512 bytes) là đủ cho task này vì chỉ cần đảo chân GPIO, gọi hàm vTaskDelay(), không xử lí dữ liệu và mảng cục bộ lớn
5. Trước khi map handler, linker sẽ báo lỗi gì (đoạn nguyên văn thông báo):
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
