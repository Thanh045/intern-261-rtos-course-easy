#ifndef UART_GUARD_H
#define UART_GUARD_H

#include <stddef.h>

#ifndef UART_GUARD_MODE
#define UART_GUARD_MODE 3
#endif

#if UART_GUARD_MODE < 0 || UART_GUARD_MODE > 3
#error "UART_GUARD_MODE must be 0, 1, 2 or 3"
#endif

void uart_guard_init(void);
void uart_guard_send(const char *buf, size_t len);

#if UART_GUARD_MODE == 3
void gatekeeper_task(void *argument);
#endif

#endif