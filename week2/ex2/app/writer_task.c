#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

_Static_assert(MSG_LEN == 24, "Message format requires MSG_LEN = 24");

void writer_task(void *argument)
{
    uint32_t id = (uint32_t)(uintptr_t)argument;
    uint32_t seq = 0;

    configASSERT(id < WRITER_COUNT);
    configASSERT(id < 10U);

    for (;;) {
        char message[MSG_LEN + 1];

        int length = snprintf(
            message,
            sizeof(message),
            "W%lu #%08lu ",
            (unsigned long)id,
            (unsigned long)seq
        );

        if (length != 13) {
            configASSERT(0);
            taskDISABLE_INTERRUPTS();

            for (;;) {
            }
        }

        for (uint32_t i = 13; i < MSG_LEN - 2U; i++) {
            message[i] = (char)('A' + id);
        }

        message[MSG_LEN - 2U] = '\r';
        message[MSG_LEN - 1U] = '\n';
        message[MSG_LEN] = '\0';

        uart_guard_send(message, MSG_LEN);

        seq = (seq + 1U) % 100000000U;

        vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS));
    }
}