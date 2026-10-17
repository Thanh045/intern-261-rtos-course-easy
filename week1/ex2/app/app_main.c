/* Week 1 / Ex2 — priorities, preemption, starvation.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"

void app_main(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    uint16_t pins = (uint16_t)(
        (1U << TASK_A_PIN) |
        (1U << TASK_B_PIN) |
        (1U << TASK_C_PIN)
    );

    HAL_GPIO_WritePin(GPIOA, pins, GPIO_PIN_RESET);

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = pins;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    dwt_init();

    BaseType_t result;

    result = xTaskCreate(
        task_a, "A", 128, NULL, TASK_A_PRIO, NULL
    );
    configASSERT(result == pdPASS);
    if (result != pdPASS) {
        for (;;) {
        }
    }

    result = xTaskCreate(
        task_b, "B", 128, NULL, TASK_B_PRIO, NULL
    );
    configASSERT(result == pdPASS);
    if (result != pdPASS) {
        for (;;) {
        }
    }

    result = xTaskCreate(
        task_c, "C", 128, NULL, TASK_C_PRIO, NULL
    );
    configASSERT(result == pdPASS);
    if (result != pdPASS) {
        for (;;) {
        }
    }

    vTaskStartScheduler();

    for (;;) {
    }
}

void vApplicationStackOverflowHook(
    TaskHandle_t task,
    char *task_name)
{
    (void)task;
    (void)task_name;

    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}