/* DWT cycle counter — the only honest way to burn a known number of
 * microseconds of CPU inside a task without blocking.
 */
#include "stm32f1xx.h"
#include "tasks.h"

void dwt_init(void)
{
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void dwt_busy_wait_us(uint32_t microseconds)
{
  uint32_t cycles =
        (uint32_t)(((uint64_t)SystemCoreClock * microseconds) / 1000000U);

    uint32_t start = DWT->CYCCNT;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
    }
   
}
