#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "exti_button.h"

void app_main(void)
{
    exti_button_init();

    BaseType_t ket_qua = xTaskCreate(
        ack_task,
        "ack",
        256U,
        NULL,
        3U,
        NULL
    );

    configASSERT(ket_qua == pdPASS);

    if (ket_qua != pdPASS) {
        __disable_irq();

        for (;;) {
        }
    }

    vTaskStartScheduler();

    __disable_irq();

    for (;;) {
    }
}