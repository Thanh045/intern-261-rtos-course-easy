#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

extern UART_HandleTypeDef huart1;

volatile uint32_t so_mau_uart_nhan = 0;
volatile uint32_t so_mau_thieu_tai_uart = 0;
volatile uint32_t so_lan_uart_loi = 0;

void uart_task(void *argument)
{
    (void)argument;

    uint32_t so_thu_tu_mong_doi = 0;

    for (;;) {
        filtered_sample_t mau;

        BaseType_t da_nhan = xQueueReceive(
            queue_filtered,
            &mau,
            portMAX_DELAY
        );

        if (da_nhan != pdPASS) {
            continue;
        }

        so_mau_uart_nhan++;

        so_mau_thieu_tai_uart +=
            (uint32_t)(mau.seq - so_thu_tu_mong_doi);

        so_thu_tu_mong_doi = mau.seq + 1U;

        char dong_tin[64];

        int do_dai = snprintf(
            dong_tin,
            sizeof(dong_tin),
            "%lu,%d,%d,%d\r\n",
            (unsigned long)mau.seq,
            (int)mau.ax,
            (int)mau.ay,
            (int)mau.az
        );

        if (do_dai <= 0 || (size_t)do_dai >= sizeof(dong_tin)) {
            so_lan_uart_loi++;
            continue;
        }

        HAL_StatusTypeDef ket_qua = HAL_UART_Transmit(
            &huart1, // con trỏ trỏ tới bộ điều khiển ngoại vi uart
            (uint8_t *)dong_tin,
            (uint16_t)do_dai,
            1000U
        );

        if (ket_qua != HAL_OK) {
            so_lan_uart_loi++;
        }
    }
}