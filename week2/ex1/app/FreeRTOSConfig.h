#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"
#include "params.h"

#define configCPU_CLOCK_HZ                      SystemCoreClock
#define configTICK_RATE_HZ                      1000
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES                    5
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

#define configMINIMAL_STACK_SIZE               128
#define configTOTAL_HEAP_SIZE                  10240
#define configSUPPORT_DYNAMIC_ALLOCATION       1
#define configSUPPORT_STATIC_ALLOCATION        1
#define configMAX_TASK_NAME_LEN                16

#define configCHECK_FOR_STACK_OVERFLOW         2
#define configUSE_MALLOC_FAILED_HOOK           1

#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0
#define configUSE_CO_ROUTINES                  0
#define INCLUDE_vTaskSuspend                   1
#define configUSE_TIMERS                       1
#define configTIMER_TASK_PRIORITY              2
#define configTIMER_QUEUE_LENGTH               5
#define configTIMER_TASK_STACK_DEPTH           128

#define INCLUDE_vTaskDelay                     1
#define INCLUDE_xTaskDelayUntil                1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle         1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle 1

#define configPRIO_BITS                        __NVIC_PRIO_BITS
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configASSERT(condition)         \
    do {                                \
        if ((condition) == 0) {         \
            taskDISABLE_INTERRUPTS();   \
            for (;;) {                  \
            }                           \
        }                               \
    } while (0)

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif