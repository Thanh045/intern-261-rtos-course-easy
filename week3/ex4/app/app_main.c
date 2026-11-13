
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "deadlock_demo.h"
#include "supervisor.h"

// Hai task phu, khai bao ngay trong file C
void task_phu_1(void *tham_so);
void task_phu_2(void *tham_so);

void app_main(void)
{
    runtime_stats_timer_init();
    deadlock_demo_init();
    supervisor_init();

    BaseType_t ket_qua;

    ket_qua = xTaskCreate(
        task_alpha, "alpha", 256, NULL, 2, NULL);
    configASSERT(ket_qua == pdPASS);

    ket_qua = xTaskCreate(
        task_beta, "beta", 256, NULL, 2, NULL);
    configASSERT(ket_qua == pdPASS);

    ket_qua = xTaskCreate(
        task_phu_1, "phu_1", 128, NULL, 1, NULL);
    configASSERT(ket_qua == pdPASS);

    ket_qua = xTaskCreate(
        task_phu_2, "phu_2", 128, NULL, 1, NULL);
    configASSERT(ket_qua == pdPASS);

    ket_qua = xTaskCreate(
        supervisor_task, "supervisor",
        256, NULL, 3, NULL);
    configASSERT(ket_qua == pdPASS);

    ket_qua = xTaskCreate(
        runtime_stats_task, "cpu_stats",
        384, NULL, 1, NULL);
    configASSERT(ket_qua == pdPASS);

    vTaskStartScheduler();

    for (;;) {
    }
}
