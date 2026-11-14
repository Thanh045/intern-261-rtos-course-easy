#ifndef EXTI_BUTTON_H
#define EXTI_BUTTON_H

#include <stdint.h>

#define THU_PRIORITY_SAI  0
#define DUNG_YIELD_ISR    1

void exti_button_init(void);
void exti_button_isr_handler(void);
void ack_task(void *argument);

#endif