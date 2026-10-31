#include "FreeRTOS.h"
#include "task.h"

#include "main.h"

static StaticTask_t idle_tcb;
static StackType_t idle_stack[configMINIMAL_STACK_SIZE];

static StaticTask_t timer_tcb;
static StackType_t timer_stack[configTIMER_TASK_STACK_DEPTH];

volatile uint32_t fault_code = 0;

static void fault_wait(void)
{
    for (volatile uint32_t i = 0; i < 800000U; i++) {
        __NOP();
    }
}

static void fault_signal(uint32_t pulses)
{
    __disable_irq();

    fault_code = pulses;

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    (void)RCC->APB2ENR;

    GPIOA->CRL =
        (GPIOA->CRL & ~0xFU) | 0x2U;

    for (;;) {
        for (uint32_t i = 0; i < pulses; i++) {
            GPIOA->BSRR = GPIO_PIN_0;
            fault_wait();

            GPIOA->BRR = GPIO_PIN_0;
            fault_wait();
        }

        for (uint32_t i = 0; i < 6U; i++) {
            fault_wait();
        }
    }
}

void vApplicationStackOverflowHook(
    TaskHandle_t task,
    char *task_name)
{
    (void)task;
    (void)task_name;

    fault_signal(2U);
}

void vApplicationMallocFailedHook(void)
{
    fault_signal(3U);
}

void vApplicationGetIdleTaskMemory(
    StaticTask_t **tcb,
    StackType_t **stack,
    uint32_t *stack_size)
{
    *tcb = &idle_tcb;
    *stack = idle_stack;
    *stack_size = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(
    StaticTask_t **tcb,
    StackType_t **stack,
    uint32_t *stack_size)
{
    *tcb = &timer_tcb;
    *stack = timer_stack;
    *stack_size = configTIMER_TASK_STACK_DEPTH;
}