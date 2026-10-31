#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

extern UART_HandleTypeDef huart1;

volatile uint32_t uart_busy_count = 0;
volatile uint32_t uart_error_count = 0;

#if UART_GUARD_MODE == 1

static SemaphoreHandle_t s_uart_mutex = NULL;

#elif UART_GUARD_MODE == 3

#define GATEKEEPER_QUEUE_LENGTH WRITER_COUNT

static QueueHandle_t s_gatekeeper_queue = NULL;

#endif

static void guard_failed(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static void uart_write_bytes(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        HAL_StatusTypeDef status;

        do {
            status = HAL_UART_Transmit(
                &huart1,
                (uint8_t *)&buf[i],
                1,
                100U
            );

            if (status == HAL_BUSY) {
                taskENTER_CRITICAL();
                uart_busy_count++;
                taskEXIT_CRITICAL();

#if UART_GUARD_MODE == 0
                taskYIELD();
#else
                guard_failed();
#endif
            }
        } while (status == HAL_BUSY);

        if (status != HAL_OK) {
            taskENTER_CRITICAL();
            uart_error_count++;
            taskEXIT_CRITICAL();

            guard_failed();
        }

#if UART_GUARD_MODE == 0
        taskYIELD();
#endif
    }
}

void uart_guard_init(void)
{
#if UART_GUARD_MODE == 1

    s_uart_mutex = xSemaphoreCreateMutex();

    configASSERT(s_uart_mutex != NULL);

    if (s_uart_mutex == NULL) {
        guard_failed();
    }

#elif UART_GUARD_MODE == 3

    s_gatekeeper_queue = xQueueCreate(
        GATEKEEPER_QUEUE_LENGTH,
        MSG_LEN
    );

    configASSERT(s_gatekeeper_queue != NULL);

    if (s_gatekeeper_queue == NULL) {
        guard_failed();
    }

    BaseType_t result = xTaskCreate(
        gatekeeper_task,
        "gatekeeper",
        256,
        NULL,
        GATEKEEPER_PRIO,
        NULL
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        guard_failed();
    }

#endif
}

void uart_guard_send(const char *buf, size_t len)
{
    configASSERT(buf != NULL);
    configASSERT(len == MSG_LEN);

    if (buf == NULL || len != MSG_LEN) {
        guard_failed();
    }

#if UART_GUARD_MODE == 0

    uart_write_bytes(buf, len);

#elif UART_GUARD_MODE == 1

    BaseType_t taken = xSemaphoreTake(
        s_uart_mutex,
        portMAX_DELAY
    );

    if (taken != pdTRUE) {
        guard_failed();
    }

    uart_write_bytes(buf, len);

    BaseType_t given = xSemaphoreGive(s_uart_mutex);

    configASSERT(given == pdTRUE);

    if (given != pdTRUE) {
        guard_failed();
    }

#elif UART_GUARD_MODE == 2

    taskENTER_CRITICAL();

    uart_write_bytes(buf, len);

    taskEXIT_CRITICAL();

#elif UART_GUARD_MODE == 3

    BaseType_t result = xQueueSend(
        s_gatekeeper_queue,
        buf,
        portMAX_DELAY
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        guard_failed();
    }

#endif
}

#if UART_GUARD_MODE == 3

void gatekeeper_task(void *argument)
{
    (void)argument;

    for (;;) {
        char message[MSG_LEN];

        BaseType_t result = xQueueReceive(
            s_gatekeeper_queue,
            message,
            portMAX_DELAY
        );

        configASSERT(result == pdPASS);

        if (result != pdPASS) {
            guard_failed();
        }

        uart_write_bytes(message, sizeof(message));
    }
}

#endif