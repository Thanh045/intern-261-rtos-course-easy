#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "debounce.h"

void app_main(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* PA0: tín hiệu sau debounce. */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW; // tốc độ thấp để ra tín hiệu sạch sau khi lọc bằng debounce
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA1: nút nhấn, bắt cả cạnh lên và xuống. */
    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING; // chế độ ngắt ngoài để bắt exti ( ngắt ) cả cạnh lên và xuống
    gpio.Pull = GPIO_PULLUP; 
    HAL_GPIO_Init(GPIOA, &gpio);

    debounce_init();

    HAL_NVIC_SetPriority(
        EXTI1_IRQn,
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
        0U
    );

    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    vTaskStartScheduler();

    for (;;) {
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_1) {
        debounce_on_edge(
            HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET
        );
    }
}
#if (configSUPPORT_STATIC_ALLOCATION == 1) && (configUSE_TIMERS == 1)

void vApplicationGetTimerTaskMemory(
    StaticTask_t **tcb,
    StackType_t **stack,
    uint32_t *so_word
)
{
    static StaticTask_t timer_tcb;
    static StackType_t timer_stack[configTIMER_TASK_STACK_DEPTH];

    *tcb = &timer_tcb;
    *stack = timer_stack;
    *so_word = configTIMER_TASK_STACK_DEPTH;
}

#endif