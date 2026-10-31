#include <stdio.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "main.h"
#include "params.h"

extern UART_HandleTypeDef huart1;

extern TaskHandle_t dyn_task_handle;
extern TaskHandle_t stat_task_handle;
extern TaskHandle_t ram_report_task_handle;

volatile uint32_t uart_error_count = 0;

static void uart_send(const char *text, uint16_t length)
{
    HAL_StatusTypeDef result = HAL_UART_Transmit(
        &huart1,
        (uint8_t *)text,
        length,
        1000U
    );

    if (result != HAL_OK) {
        uart_error_count++;
    }
}

static void print_value(
    const char *name,
    unsigned long value,
    const char *unit)
{
    char buffer[112];

    int length = snprintf(
        buffer,
        sizeof(buffer),
        "%s=%lu %s\r\n",
        name,
        value,
        unit
    );

    if (length <= 0 || (size_t)length >= sizeof(buffer)) {
        uart_error_count++;
        return;
    }

    uart_send(buffer, (uint16_t)length);
}

static void print_hwm(
    const char *name,
    TaskHandle_t handle)
{
    if (handle == NULL) {
        static const char message[] = "ERROR: null task handle\r\n";
        uart_send(message, sizeof(message) - 1U);
        return;
    }

    UBaseType_t words = uxTaskGetStackHighWaterMark(handle);

    char buffer[112];

    int length = snprintf(
        buffer,
        sizeof(buffer),
        "%s: HWM=%lu words, %lu bytes\r\n",
        name,
        (unsigned long)words,
        (unsigned long)(words * sizeof(StackType_t))
    );

    if (length <= 0 || (size_t)length >= sizeof(buffer)) {
        uart_error_count++;
        return;
    }

    uart_send(buffer, (uint16_t)length);
}

void ram_report_task(void *argument)
{
    (void)argument;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));

        size_t free_heap = xPortGetFreeHeapSize();
        size_t minimum_heap = xPortGetMinimumEverFreeHeapSize();

        static const char heading[] = "\r\n--- RAM REPORT ---\r\n";
        uart_send(heading, sizeof(heading) - 1U);

        print_value(
            "free_heap",
            (unsigned long)free_heap,
            "bytes"
        );

        print_value(
            "minimum_ever_free_heap",
            (unsigned long)minimum_heap,
            "bytes"
        );

        print_value(
            "target_free_heap",
            TARGET_FREE_HEAP_BYTES,
            "bytes"
        );

        print_value(
            "heap_target_met",
            free_heap >= TARGET_FREE_HEAP_BYTES ? 1UL : 0UL,
            ""
        );

        print_hwm("dyn_task", dyn_task_handle);
        print_hwm("stat_task", stat_task_handle);
        print_hwm("ram_report_task", ram_report_task_handle);
        print_hwm("idle_task", xTaskGetIdleTaskHandle());
        print_hwm("timer_task", xTimerGetTimerDaemonTaskHandle());

        print_value(
            "uart_error_count",
            uart_error_count,
            ""
        );
    }
}