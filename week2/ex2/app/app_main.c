#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

void writer_task(void *argument);

static void creation_failed(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

void app_main(void)
{
    uart_guard_init();

    for (uint32_t i = 0; i < WRITER_COUNT; i++) {
        BaseType_t result = xTaskCreate(
            writer_task,
            "writer",
            384,
            (void *)(uintptr_t)i,
            tskIDLE_PRIORITY + 1,
            NULL
        );

        configASSERT(result == pdPASS);

        if (result != pdPASS) {
            creation_failed();
        }
    }

    vTaskStartScheduler();

    creation_failed();
}