
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"

extern UART_HandleTypeDef huart1;

extern volatile uint32_t alpha_dang_cho;
extern volatile uint32_t beta_dang_cho;
extern volatile uint32_t so_vong_alpha;
extern volatile uint32_t so_vong_beta;

void runtime_stats_timer_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CYCCNT = 0;

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t runtime_stats_timer_value(void)
{
    return DWT->CYCCNT;
}

static void gui_uart(const char *noi_dung)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)noi_dung,
        (uint16_t)strlen(noi_dung),
        300
    );
}

static const char *doc_trang_thai(eTaskState trang_thai) // tham số đầu vào kiểu enum
{
    switch (trang_thai)
    {
        case eRunning:   return "Running";
        case eReady:     return "Ready";
        case eBlocked:   return "Blocked";
        case eSuspended: return "Suspended";
        case eDeleted:   return "Deleted";
        default:         return "Invalid";
    }
}

static const char *ten_mutex(uint32_t dang_cho)
{
    if (dang_cho == 1) return "mutex_A";
    if (dang_cho == 2) return "mutex_B";

    return "khong";
}

void runtime_stats_task(void *tham_so)
{
    (void)tham_so;

    static char bang_cpu[768];
    static TaskStatus_t danh_sach[12];

    char dong[160];

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(STATS_WINDOW_MS));

        memset(bang_cpu, 0, sizeof(bang_cpu)); // Đặt toàn bộ 768 byte trong bang_cpu về 0, làm sạch dữ liệu cũ.

        vTaskGetRunTimeStats(bang_cpu); // yêu cầu rtos tạo bảng thống kê sử dụng của từng task 

        gui_uart("\r\n=== CPU STATS ===\r\n");
        gui_uart(bang_cpu);

        gui_uart("\r\n=== TASK STATES ===\r\n");

        UBaseType_t so_task = uxTaskGetSystemState(
            danh_sach,
            12,
            NULL
        ); // hàm lấy trạng thái từng task
    
//in trạng thái task
        for (UBaseType_t i = 0; i < so_task; i++)
        {
            snprintf(
                dong,
                sizeof(dong),
                "%s: %s\r\n",
                danh_sach[i].pcTaskName,
                doc_trang_thai(
                    danh_sach[i].eCurrentState
                )
            );

            gui_uart(dong);
        }
// in mutex A,B chờ
        snprintf(
            dong,
            sizeof(dong),
            "Alpha cho: %s | Beta cho: %s\r\n",
            ten_mutex(alpha_dang_cho),
            ten_mutex(beta_dang_cho)
        );

        gui_uart(dong);
// in số vòng xử lí thành công
        snprintf(
            dong,
            sizeof(dong),
            "So vong: alpha=%lu, beta=%lu\r\n",
            (unsigned long)so_vong_alpha,
            (unsigned long)so_vong_beta
        );

        gui_uart(dong);
    }
}
