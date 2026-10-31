#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

void ram_report_task(void *argument);

TaskHandle_t dyn_task_handle = NULL;
TaskHandle_t stat_task_handle = NULL;
TaskHandle_t ram_report_task_handle = NULL;

static StaticTask_t stat_task_tcb;
static StackType_t stat_task_stack[STATIC_TASK_STACK_WORDS];

static volatile uint32_t work_count = 0;

static void shared_work(void *argument)
{
    (void)argument;

    for (;;) {
        taskENTER_CRITICAL();
        work_count++;
        taskEXIT_CRITICAL();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
static void dyn_work(void *argument)
{
    volatile uint8_t hog[HOG_DEPTH_BYTES];

    for (uint32_t i = 0; i < HOG_DEPTH_BYTES; i++) {
        hog[i] = (uint8_t)i;
    }

    shared_work(argument);
}
static void creation_failed(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static void extra_task(void *argument)
{
    (void)argument;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    BaseType_t result;

    result = xTaskCreate(
        shared_work,
        "dyn_task",
       57,
        NULL,
        tskIDLE_PRIORITY + 1,
        &dyn_task_handle
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        creation_failed();
    }

    stat_task_handle = xTaskCreateStatic(
        shared_work,
        "stat_task",
        STATIC_TASK_STACK_WORDS,
        NULL,
        tskIDLE_PRIORITY + 1,
        stat_task_stack,
        &stat_task_tcb
    );

    configASSERT(stat_task_handle != NULL);

    if (stat_task_handle == NULL) {
        creation_failed();
    }

    result = xTaskCreate(
        ram_report_task,
        "ram_report",
        512,
        NULL,
        tskIDLE_PRIORITY + 1,
        &ram_report_task_handle
    );

    configASSERT(result == pdPASS);

    if (result != pdPASS) {
        creation_failed();
    }
result = xTaskCreate(
    extra_task,
    "extra_task",
    128,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
);

configASSERT(result == pdPASS);

if (result != pdPASS) {
    creation_failed();
}
    vTaskStartScheduler();

    creation_failed();
}