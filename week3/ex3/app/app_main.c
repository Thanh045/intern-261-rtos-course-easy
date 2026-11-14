#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"

void app_main(void)
{
    GPIO_InitTypeDef cau_hinh_gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    cau_hinh_gpio.Pin = GPIO_PIN_0;
    cau_hinh_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    cau_hinh_gpio.Pull = GPIO_NOPULL;
    cau_hinh_gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &cau_hinh_gpio);

    configASSERT(configMAX_PRIORITIES >= 3);

    uart_rx_init();

    if (xTaskCreate(
            demo_task,
            "demo",
            256,
            NULL,
            1,
            NULL) != pdPASS) {
        taskDISABLE_INTERRUPTS();
        for (;;) {
        }
    }

    if (xTaskCreate(
            parser_task,
            "parser",
            256,
            NULL,
            configMAX_PRIORITIES - 1,
            NULL) != pdPASS) {
        taskDISABLE_INTERRUPTS();
        for (;;) {
        }
    }

    vTaskStartScheduler();

    for (;;) {
    }
}