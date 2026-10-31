#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "sync_tasks.h"

static void dung_khi_loi(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

void app_main(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    sync_init();

    TaskHandle_t task_dong_bo = NULL;

    BaseType_t ket_qua = xTaskCreate(
        sync_task,
        "sync",
        512,
        NULL,
        3,
        &task_dong_bo
    );

    configASSERT(ket_qua == pdPASS);

    if (ket_qua != pdPASS) {
        dung_khi_loi();
    }

    sync_task_handle = (void *)task_dong_bo;

    for (uint32_t ma_task = 0;
         ma_task < EVENT_BIT_COUNT;
         ma_task++) {

        ket_qua = xTaskCreate(
            producer_task,
            "producer",
            192,
            (void *)(uintptr_t)ma_task,
            1,
            NULL
        );

        configASSERT(ket_qua == pdPASS);

        if (ket_qua != pdPASS) {
            dung_khi_loi();
        }
    }

    vTaskStartScheduler();

    dung_khi_loi();
}
