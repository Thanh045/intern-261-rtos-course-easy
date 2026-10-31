#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

static void creation_failed(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

void app_main(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_WritePin(GPIOA, gpio.Pin, GPIO_PIN_RESET);
    HAL_GPIO_Init(GPIOA, &gpio);

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    resource_init();

    BaseType_t result = xTaskCreate(
        task_low,
        "low",
        192,
        NULL,
        1,
        NULL
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        creation_failed();
    }

    result = xTaskCreate(
        task_mid,
        "mid",
        192,
        NULL,
        2,
        NULL
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        creation_failed();
    }

    result = xTaskCreate(
        task_high,
        "high",
        192,
        NULL,
        3,
        NULL
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        creation_failed();
    }

    vTaskStartScheduler();

    creation_failed();
}