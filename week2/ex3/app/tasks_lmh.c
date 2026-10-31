#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

#if RESOURCE_MODE != 0 && RESOURCE_MODE != 1
#error "RESOURCE_MODE must be 0 or 1"
#endif

#define MID_WORK_MULTIPLIER 1U

static SemaphoreHandle_t resource_lock = NULL;
static volatile uint32_t shared_resource_value = 0;

volatile UBaseType_t low_priority_when_locked = 0;
volatile UBaseType_t low_priority_before_unlock = 0;

volatile uint32_t high_measurement_count = 0;
volatile uint32_t high_last_wait_cycles = 0;
volatile uint32_t high_max_wait_cycles = 0;
volatile uint32_t high_last_completion_cycles = 0;
volatile uint32_t high_max_completion_cycles = 0;
volatile uint32_t high_deadline_miss_count = 0;

static void stop_program_on_error(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static void busy_wait_microseconds(uint32_t wait_microseconds)
{
    uint32_t required_cpu_cycles = (uint32_t)(
        ((uint64_t)SystemCoreClock * wait_microseconds)
        / 1000000U
    );

    uint32_t start_cpu_cycles = DWT->CYCCNT;

    while (
        (uint32_t)(DWT->CYCCNT - start_cpu_cycles)
        < required_cpu_cycles
    ) {
        __NOP();
    }
}

void resource_init(void)
{
#if RESOURCE_MODE == 0
    resource_lock = xSemaphoreCreateBinary();
#else
    resource_lock = xSemaphoreCreateMutex();
#endif

    configASSERT(resource_lock != NULL);

    if (resource_lock == NULL) {
        stop_program_on_error();
    }

#if RESOURCE_MODE == 0
    BaseType_t give_succeeded = xSemaphoreGive(resource_lock);

    configASSERT(give_succeeded == pdTRUE);

    if (give_succeeded != pdTRUE) {
        stop_program_on_error();
    }
#endif
}

void task_low(void *argument)
{
    (void)argument;

    TickType_t previous_wake_tick = xTaskGetTickCount();

    const TickType_t repeat_period_ticks =
        pdMS_TO_TICKS(HIGH_PERIOD_MS);

    configASSERT(repeat_period_ticks > 0);

    for (;;) {
        BaseType_t lock_acquired = xSemaphoreTake(
            resource_lock,
            portMAX_DELAY
        );

        if (lock_acquired != pdTRUE) {
            stop_program_on_error();
        }

        GPIOA->BSRR = GPIO_PIN_1;

        low_priority_when_locked = uxTaskPriorityGet(NULL);

        busy_wait_microseconds(LOW_HOLD_MS * 1000U);

        shared_resource_value++;

        low_priority_before_unlock = uxTaskPriorityGet(NULL);

        GPIOA->BRR = GPIO_PIN_1;

        BaseType_t unlock_succeeded = xSemaphoreGive(
            resource_lock
        );

        if (unlock_succeeded != pdTRUE) {
            stop_program_on_error();
        }

        BaseType_t task_was_delayed = xTaskDelayUntil(
            &previous_wake_tick,
            repeat_period_ticks
        );

        (void)task_was_delayed;
    }
}

void task_mid(void *argument)
{
    (void)argument;

    vTaskDelay(pdMS_TO_TICKS(3));

    TickType_t previous_wake_tick = xTaskGetTickCount();

    const TickType_t repeat_period_ticks =
        pdMS_TO_TICKS(HIGH_PERIOD_MS);

    configASSERT(repeat_period_ticks > 0);

    for (;;) {
        GPIOA->BSRR = GPIO_PIN_2;

        busy_wait_microseconds(
            MID_WORK_MS * MID_WORK_MULTIPLIER * 1000U
        );

        GPIOA->BRR = GPIO_PIN_2;

        BaseType_t task_was_delayed = xTaskDelayUntil(
            &previous_wake_tick,
            repeat_period_ticks
        );

        (void)task_was_delayed;
    }
}

void task_high(void *argument)
{
    (void)argument;

    vTaskDelay(pdMS_TO_TICKS(2));

    TickType_t previous_wake_tick = xTaskGetTickCount();

    const TickType_t repeat_period_ticks =
        pdMS_TO_TICKS(HIGH_PERIOD_MS);

    const uint32_t deadline_cpu_cycles = (uint32_t)(
        ((uint64_t)SystemCoreClock * DEADLINE_MS) / 1000U
    );

    configASSERT(repeat_period_ticks > 0);

    for (;;) {
        uint32_t request_start_cycles = DWT->CYCCNT;

        GPIOA->BSRR = GPIO_PIN_3;

        BaseType_t lock_acquired = xSemaphoreTake(
            resource_lock,
            portMAX_DELAY
        );

        uint32_t resource_acquired_cycles = DWT->CYCCNT;

        GPIOA->BRR = GPIO_PIN_3;

        if (lock_acquired != pdTRUE) {
            stop_program_on_error();
        }

        shared_resource_value++;

        busy_wait_microseconds(100U);

        BaseType_t unlock_succeeded = xSemaphoreGive(
            resource_lock
        );

        uint32_t work_completed_cycles = DWT->CYCCNT;

        if (unlock_succeeded != pdTRUE) {
            stop_program_on_error();
        }

        uint32_t resource_wait_cycles = (uint32_t)(
            resource_acquired_cycles - request_start_cycles
        );

        uint32_t request_to_completion_cycles = (uint32_t)(
            work_completed_cycles - request_start_cycles
        );

        high_last_wait_cycles = resource_wait_cycles;

        high_last_completion_cycles =
            request_to_completion_cycles;

        if (resource_wait_cycles > high_max_wait_cycles) {
            high_max_wait_cycles = resource_wait_cycles;
        }

        if (
            request_to_completion_cycles
            > high_max_completion_cycles
        ) {
            high_max_completion_cycles =
                request_to_completion_cycles;
        }

        if (
            request_to_completion_cycles
            > deadline_cpu_cycles
        ) {
            high_deadline_miss_count++;
        }

        high_measurement_count++;

        BaseType_t task_was_delayed = xTaskDelayUntil(
            &previous_wake_tick,
            repeat_period_ticks
        );

        (void)task_was_delayed;
    }
}