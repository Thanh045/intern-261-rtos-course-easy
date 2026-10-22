/* Week 1 / Ex3 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

void periodic_task(void *argument);
void dwt_init(void);


void app_main(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    const uint16_t pin = (uint16_t)(1U << PERIODIC_PIN);

    HAL_GPIO_WritePin(GPIOA, pin, GPIO_PIN_RESET);

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    dwt_init();

    BaseType_t result = xTaskCreate(
        periodic_task,
        "periodic",
        128,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL
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
