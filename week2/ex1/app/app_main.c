#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

QueueHandle_t queue_raw = NULL;
QueueHandle_t queue_filtered = NULL;


volatile uint32_t so_mau_do_day_da_ghi = 0;
volatile mau_do_day_t bang_do_day[SO_MAU_GHI_DO_DAY];

static void dung_khi_loi(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static void kiem_tra_tao_task(BaseType_t ket_qua)
{
    configASSERT(ket_qua == pdPASS);

    if (ket_qua != pdPASS) {
        dung_khi_loi();
    }
}

void depth_logger_task(void *argument)
{
    (void)argument;

    const TickType_t chu_ky = pdMS_TO_TICKS(CHU_KY_GHI_DO_DAY_MS); 

    configASSERT(chu_ky > 0);

    if (chu_ky == 0) {
        dung_khi_loi();
    }

    TickType_t lan_thuc_truoc = xTaskGetTickCount();

    for (;;) {
        uint32_t vi_tri = so_mau_do_day_da_ghi;

        if (vi_tri < SO_MAU_GHI_DO_DAY) {
            bang_do_day[vi_tri].thoi_gian_ms =
                (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

            bang_do_day[vi_tri].so_mau_queue_raw =
                (uint16_t)uxQueueMessagesWaiting(queue_raw);

            bang_do_day[vi_tri].so_mau_queue_filtered =
                (uint16_t)uxQueueMessagesWaiting(queue_filtered);

            so_mau_do_day_da_ghi = vi_tri + 1U;
        }

        vTaskDelayUntil(&lan_thuc_truoc, chu_ky);
    }
}

void app_main(void)
{
    queue_raw = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(raw_sample_t)
    );

    configASSERT(queue_raw != NULL);

    if (queue_raw == NULL) {
        dung_khi_loi();
    }

    queue_filtered = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(filtered_sample_t)
    );

    configASSERT(queue_filtered != NULL);

    if (queue_filtered == NULL) {
        dung_khi_loi();
    }

    kiem_tra_tao_task(xTaskCreate(
        sensor_task,
        "sensor",
        256,
        NULL,
        3,
        NULL
    ));

    kiem_tra_tao_task(xTaskCreate(
        filter_task,
        "filter",
        256,
        NULL,
        2,
        NULL
    ));

    kiem_tra_tao_task(xTaskCreate(
        uart_task,
        "uart",
        512,
        NULL,
        1,
        NULL
    ));

    kiem_tra_tao_task(xTaskCreate(
        depth_logger_task,
        "depth_logger",
        192,
        NULL,
        1,
        NULL
    ));

    vTaskStartScheduler();

    dung_khi_loi();
}