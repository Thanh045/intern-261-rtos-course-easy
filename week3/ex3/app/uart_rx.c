#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"

extern UART_HandleTypeDef huart1;

StreamBufferHandle_t rx_stream;

static uint8_t s_rx_byte;

/* Xem các biến này trong debugger để kiểm tra mất byte. */
volatile uint32_t so_byte_nhan = 0;
volatile uint32_t so_byte_mat = 0;
volatile uint32_t so_loi_nhan = 0;

void uart_rx_init(void)
{
    rx_stream = xStreamBufferCreate(
        STREAM_BUFFER_BYTES,
        TRIGGER_LEVEL
    );

    if (rx_stream == NULL) {
        taskDISABLE_INTERRUPTS();
        for (;;) {
        }
    }

    /*
     * huart1 phải được khởi tạo trước ở RX_BAUD.
     * Đặt mức ưu tiên ngắt được phép gọi API FromISR.
     */
    HAL_NVIC_DisableIRQ(USART1_IRQn);

    HAL_NVIC_SetPriority(
        USART1_IRQn,
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
        0U
    );

    if (HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1) != HAL_OK) {
        taskDISABLE_INTERRUPTS();
        for (;;) {
        }
    }

    /*
     * parser_task sẽ bật ngắt sau khi scheduler chạy.
     * Tránh gọi API FromISR trước khi hệ thống sẵn sàng.
     */
}

void uart_rx_isr_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    so_byte_nhan++;

    if (xStreamBufferSendFromISR(
            rx_stream,
            &s_rx_byte,
            1,
            &xHigherPriorityTaskWoken) != 1) {
        /* Buffer đầy: bỏ byte mới và ghi nhận số byte mất. */
        so_byte_mat++;
    }

    if (HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1) != HAL_OK) {
        so_loi_nhan++;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_rx_isr_handler();
    }
}