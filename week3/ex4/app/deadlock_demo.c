
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "deadlock_demo.h"
#include "supervisor.h"


#define EX4_GAY_DEADLOCK 0


static SemaphoreHandle_t mutex_a;
static SemaphoreHandle_t mutex_b;

// Theo doi trong debugger:
// 0: khong cho; 1: cho mutex A; 2: cho mutex B
volatile uint32_t alpha_dang_cho = 0;
volatile uint32_t beta_dang_cho = 0;

volatile uint32_t so_vong_alpha = 0;
volatile uint32_t so_vong_beta = 0;

void deadlock_demo_init(void)
{
    mutex_a = xSemaphoreCreateMutex();
    mutex_b = xSemaphoreCreateMutex();

    configASSERT(mutex_a != NULL);
    configASSERT(mutex_b != NULL);

    vQueueAddToRegistry(mutex_a, "mutex_A"); // gán tên các địa chỉ thành Mutex tương ứng dễ debug
    vQueueAddToRegistry(mutex_b, "mutex_B");
}

void task_alpha(void *tham_so)
{
    (void)tham_so;

    for (;;)
    {
        // Alpha luon lay A truoc
        alpha_dang_cho = 1;
        xSemaphoreTake(mutex_a, portMAX_DELAY);
        alpha_dang_cho = 0;

        vTaskDelay(pdMS_TO_TICKS(5));

        alpha_dang_cho = 2;
        xSemaphoreTake(mutex_b, portMAX_DELAY);
        alpha_dang_cho = 0;

        // Da lay du hai mutex
        so_vong_alpha++; // đếm số lần alpha lấy đủ 2 mutex

        xSemaphoreGive(mutex_b);
        xSemaphoreGive(mutex_a);

        // Chi bao song sau khi hoan thanh cong viec
        supervisor_checkin(SUPERVISOR_BIT_ALPHA);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void task_beta(void *tham_so)
{
    (void)tham_so;

    for (;;)
    {
#if EX4_GAY_DEADLOCK

        // Che do loi goc: Beta lay B roi A
        beta_dang_cho = 2;
        xSemaphoreTake(mutex_b, portMAX_DELAY);
        beta_dang_cho = 0;

        vTaskDelay(pdMS_TO_TICKS(5));

        beta_dang_cho = 1;
        xSemaphoreTake(mutex_a, portMAX_DELAY);
        beta_dang_cho = 0;

        so_vong_beta++;

        xSemaphoreGive(mutex_a);
        xSemaphoreGive(mutex_b);

#else

        // Che do da sua: Beta cung lay A roi B
        beta_dang_cho = 1;
        xSemaphoreTake(mutex_a, portMAX_DELAY);
        beta_dang_cho = 0;

        vTaskDelay(pdMS_TO_TICKS(5));

        beta_dang_cho = 2;
        xSemaphoreTake(mutex_b, portMAX_DELAY);
        beta_dang_cho = 0;

        so_vong_beta++;

        xSemaphoreGive(mutex_b);
        xSemaphoreGive(mutex_a);

#endif

        supervisor_checkin(SUPERVISOR_BIT_BETA);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
