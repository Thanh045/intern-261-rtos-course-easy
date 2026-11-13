
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "params.h"
#include "supervisor.h"

#ifndef EX4_BAT_WATCHDOG
#define EX4_BAT_WATCHDOG 0
#endif

#define BIT_PHU_1 (1u << 2)
#define BIT_PHU_2 (1u << 3)

#define TAT_CA_BIT ((1u << SUPERVISOR_BITS) - 1u) // có 4 bit, 16 -1 = 15 = 0x0F

EventGroupHandle_t supervisor_events = NULL;

#if EX4_BAT_WATCHDOG
static IWDG_HandleTypeDef watchdog;
#endif

volatile uint32_t so_lan_du_heartbeat = 0;
volatile uint32_t so_lan_thieu_heartbeat = 0;
volatile EventBits_t bit_thieu = 0;

void supervisor_init(void)
{
    configASSERT(SUPERVISOR_BITS == 4);

    supervisor_events = xEventGroupCreate();

    configASSERT(supervisor_events != NULL);
}

static void bat_watchdog(void)
{
#if EX4_BAT_WATCHDOG

    // LSI khoang 40000 Hz
    // Prescaler = 64
    // Reload = 1000 * (40000 / 64) / 1000 - 1 = 624

    watchdog.Instance = IWDG;
    watchdog.Init.Prescaler = IWDG_PRESCALER_64;

    uint32_t gia_tri_reload =
        ((uint32_t)IWDG_TIMEOUT_MS * 40000UL)
        / (64UL * 1000UL);

    configASSERT(gia_tri_reload >= 1);
    configASSERT(gia_tri_reload <= 4096);

    watchdog.Init.Reload = gia_tri_reload - 1UL;

    if (HAL_IWDG_Init(&watchdog) != HAL_OK)
    {
        Error_Handler();
    }

#endif
}

void task_phu_1(void *tham_so)
{
    (void)tham_so;

    for (;;)
    {
        supervisor_checkin(BIT_PHU_1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task_phu_2(void *tham_so)
{
    (void)tham_so;

    for (;;)
    {
        supervisor_checkin(BIT_PHU_2);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void supervisor_task(void *tham_so)
{
    (void)tham_so;

    // Bat IWDG sau khi scheduler da chay
    bat_watchdog();

    const TickType_t thoi_gian_cho =
        pdMS_TO_TICKS(300);

    for (;;)
    {
        EventBits_t bit_nhan = xEventGroupWaitBits(
            supervisor_events,
            TAT_CA_BIT,
            pdTRUE,
            pdTRUE,
            thoi_gian_cho
        );

        if ((bit_nhan & TAT_CA_BIT) == TAT_CA_BIT)
        {
            so_lan_du_heartbeat++;

#if EX4_BAT_WATCHDOG
            HAL_IWDG_Refresh(&watchdog);
#endif
        }
        else
        {
            so_lan_thieu_heartbeat++;

            bit_thieu = TAT_CA_BIT & ~bit_nhan;

            // Khong refresh IWDG khi thieu heartbeat.
            // Neu watchdog bat, phan cung se reset.
            // Neu watchdog tat, van tiep tuc thong ke.
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
