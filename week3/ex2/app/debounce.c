#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "debounce.h"

extern UART_HandleTypeDef huart1;

/* Chỉ bật khi làm thí nghiệm callback bị block. */
#define THU_CALLBACK_CHAM  0

typedef struct {
    const char *name;
    TickType_t tick;
} LogEvent;

static TimerHandle_t s_debounce_timer; // con trỏ chống rung
static TimerHandle_t s_longpress_timer; // con trỏ quản lí giữ phím
static QueueHandle_t s_log_queue;

static volatile uint8_t s_pin_high; // biến trạng thái chân vừa đọc
static volatile TickType_t s_last_edge_tick; // mốc tick khi xuất hiện sườn khung
static uint8_t s_stable_high;

static void fail(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static void send_log(const char *name)
{
    LogEvent event = {
        .name = name,
        .tick = xTaskGetTickCount()
    };

    /* Không chờ nếu queue đầy. */
    (void)xQueueSend(s_log_queue, &event, 0);
}

static void log_task(void *argument)
{
    (void)argument;
    LogEvent event;
    char buffer[64];

    for (;;) {
        if (xQueueReceive(
                s_log_queue, &event, portMAX_DELAY) == pdPASS) {

            int length = snprintf(
                buffer,
                sizeof(buffer),
                "%s tick=%lu\r\n",
                event.name,
                (unsigned long)event.tick
            );

            if (length > 0 && length < (int)sizeof(buffer)) {
                HAL_UART_Transmit(
                    &huart1,
                    (uint8_t *)buffer,
                    (uint16_t)length,
                    100U
                );
            }
        }
    }
}

static void debounce_callback(TimerHandle_t timer)
{
    (void)timer;

    TickType_t now;
    TickType_t last_edge;
    uint8_t pin_high;

    taskENTER_CRITICAL();
    now = xTaskGetTickCount();
    last_edge = s_last_edge_tick;
    pin_high = s_pin_high;
    taskEXIT_CRITICAL();

    /* Bỏ callback cũ nếu vừa xuất hiện cạnh mới. */
    if ((TickType_t)(now - last_edge)
            < pdMS_TO_TICKS(DEBOUNCE_MS)) {
        return; // nếu chưa đủ đứng yên so mốc thời gian, nên cập nhật lại
    }

    if (pin_high != s_stable_high) {
        s_stable_high = pin_high;

        HAL_GPIO_WritePin(
            GPIOA,
            GPIO_PIN_0,
            pin_high ? GPIO_PIN_SET : GPIO_PIN_RESET
        );

        if (!pin_high) {
            send_log("PRESS");
        }
    }

#if THU_CALLBACK_CHAM
    /* Chỉ dùng cho thí nghiệm, phải tắt trong bản nộp. */
    vTaskDelay(pdMS_TO_TICKS(50));
#endif
}

static void longpress_callback(TimerHandle_t timer)
{
    (void)timer;

    if (!s_pin_high && !s_stable_high) {
        send_log("HELD");
    }
}

void debounce_init(void)
{
    configASSERT(pdMS_TO_TICKS(DEBOUNCE_MS) > 0);
    configASSERT(pdMS_TO_TICKS(LONGPRESS_MS) > 0);
s_pin_high =
    (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET);

s_stable_high = s_pin_high;

HAL_GPIO_WritePin(
    GPIOA,
    GPIO_PIN_0,
    s_stable_high ? GPIO_PIN_SET : GPIO_PIN_RESET
);
    s_log_queue = xQueueCreate(16, sizeof(LogEvent));

    s_debounce_timer = xTimerCreate(
        "deb",
        pdMS_TO_TICKS(DEBOUNCE_MS),
        pdFALSE, // one-time reload
        NULL,
        debounce_callback // hàm thực thi khi hết giờ
    );

    s_longpress_timer = xTimerCreate(
        "held",
        pdMS_TO_TICKS(LONGPRESS_MS),
        pdTRUE, // auto-reload
        NULL,
        longpress_callback
    );

    if (s_log_queue == NULL ||
        s_debounce_timer == NULL ||
        s_longpress_timer == NULL) {
        fail();
    }

    if (xTaskCreate(
            log_task,
            "uart",
            256,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL) != pdPASS) {
        fail();
    }
}

void debounce_on_edge(uint8_t pin_is_high)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    BaseType_t debounce_result;
    BaseType_t longpress_result;

    s_pin_high = pin_is_high;
    s_last_edge_tick = xTaskGetTickCountFromISR();

    debounce_result = xTimerResetFromISR(
        s_debounce_timer, // mỗi khi có tín hiệu từ cạnh, buộc reset lại giá trị s_debounce_timer
        &higher_priority_task_woken
    );

    if (!pin_is_high) {
        longpress_result = xTimerStartFromISR(
            s_longpress_timer,
            &higher_priority_task_woken
        );
    } else {
        longpress_result = xTimerStopFromISR(
            s_longpress_timer,
            &higher_priority_task_woken
        );
    }

    if (debounce_result != pdPASS ||
        longpress_result != pdPASS) {
        fail();
    }

    portYIELD_FROM_ISR(higher_priority_task_woken);
}