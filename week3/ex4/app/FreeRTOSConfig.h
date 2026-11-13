
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"
#include "params.h"
#include <stdint.h>

/* 1. Cau hinh CPU va Scheduler */

#define configCPU_CLOCK_HZ                      SystemCoreClock
#define configTICK_RATE_HZ                      1000

#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#define configMAX_PRIORITIES                    5
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* 2. Cau hinh bo nho */

#define configMINIMAL_STACK_SIZE                128
#define configTOTAL_HEAP_SIZE                   10240

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         1
#define configMAX_TASK_NAME_LEN                 16

/* 3. Mutex va Event Groups */

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_EVENT_GROUPS                  1
#define configUSE_TASK_NOTIFICATIONS            1

#define configQUEUE_REGISTRY_SIZE               4

/* 4. Kiem tra loi */

#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_CO_ROUTINES                   0

/* 5. Software Timer */

#define configUSE_TIMERS                        0
#define configTIMER_TASK_PRIORITY               1
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

/* 6. Run-time Stats cho Ex4 */

#define configGENERATE_RUN_TIME_STATS           1
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* 7. Cau hinh DWT de dem chu ky CPU */

#ifndef __ASSEMBLER__

void runtime_stats_timer_init(void);
uint32_t runtime_stats_timer_value(void);

#endif

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() \
    runtime_stats_timer_init()

#define portGET_RUN_TIME_COUNTER_VALUE() \
    runtime_stats_timer_value()

/* 8. Bat cac API FreeRTOS can thiet */

#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1

#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTaskGetSchedulerState         1

/* 9. Cau hinh muc uu tien ngat */

#define configPRIO_BITS                         __NVIC_PRIO_BITS

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY \
    << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY \
    << (8 - configPRIO_BITS))

/* 10. Xu ly Assert */

#ifndef __ASSEMBLER__

void dung_khi_assert(const char *tep, int dong);

#endif

#define configASSERT(x) \
    do { \
        if ((x) == 0) { \
            dung_khi_assert(__FILE__, __LINE__); \
        } \
    } while (0)

/* 11. Anh xa ngat FreeRTOS cho STM32 */

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
