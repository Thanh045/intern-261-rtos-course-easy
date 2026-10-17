/* Week 1 / Ex3 — periodic task: vTaskDelay vs xTaskDelayUntil.
 *
 * Build this file twice over the exercise, switching DELAY_MODE, and twice more
 * with configTICK_RATE_HZ changed in FreeRTOSConfig.h. Four runs total.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "tasks.h"

#include "main.h"
#include "params.h"

/* 0 = vTaskDelay, 1 = xTaskDelayUntil */
#ifndef DELAY_MODE
#define DELAY_MODE 0
#endif

/* Fixed seed: the variable load must be reproducible across all four runs,
 * otherwise the four configurations are not comparable. */
static uint32_t s_rand_state = 0x12345678u;

volatile uint32_t periodic_no_block_count = 0;
volatile uint32_t periodic_sample_count = 0;

static uint32_t next_work_us(void)
{
    uint32_t x = s_rand_state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    s_rand_state = x;

    return x % (WORK_JITTER_US + 1U);
}

void periodic_task(void *argument)
{
    (void)argument;

    const TickType_t period_ticks = pdMS_TO_TICKS(PERIOD_MS);
    const uint16_t pin = (uint16_t)(1U << PERIODIC_PIN);

    configASSERT(period_ticks > 0);

#if DELAY_MODE == 1
    TickType_t previous_wake_time = xTaskGetTickCount();
#endif

    for (;;) {
        HAL_GPIO_TogglePin(GPIOA, pin);
        dwt_busy_wait_us(next_work_us());

#if DELAY_MODE == 0
        vTaskDelay(period_ticks);
#else
        BaseType_t did_block = xTaskDelayUntil(
            &previous_wake_time,
            period_ticks
        );

        if (periodic_sample_count < SAMPLE_COUNT) {
            if (did_block == pdFALSE) {
                periodic_no_block_count++;
            }
        }
#endif

        if (periodic_sample_count < SAMPLE_COUNT) {
            periodic_sample_count++;
        }
    }
}
