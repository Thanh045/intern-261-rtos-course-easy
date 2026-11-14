#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"

volatile uint32_t fault_code = 0;
volatile int dong_assert = 0;
const char * volatile tep_assert = 0;

static void dung_khi_loi(uint32_t ma_loi)
{
    __disable_irq();
    fault_code = ma_loi;

    for (;;) {
        __NOP();
    }
}

void dung_khi_assert(const char *tep, int dong)
{
    __disable_irq();

    tep_assert = tep;
    dong_assert = dong;

    dung_khi_loi(1U);
}

void vApplicationStackOverflowHook(
    TaskHandle_t task,
    char *ten_task)
{
    (void)task;
    (void)ten_task;

    dung_khi_loi(2U);
}

void vApplicationMallocFailedHook(void)
{
    dung_khi_loi(3U);
}

#if configSUPPORT_STATIC_ALLOCATION == 1

void vApplicationGetIdleTaskMemory(
    StaticTask_t **tcb,
    StackType_t **stack,
    uint32_t *so_word)
{
    static StaticTask_t idle_tcb;
    static StackType_t idle_stack[configMINIMAL_STACK_SIZE];

    *tcb = &idle_tcb;
    *stack = idle_stack;
    *so_word = configMINIMAL_STACK_SIZE;
}

#endif