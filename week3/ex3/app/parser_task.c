#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
extern UART_HandleTypeDef huart1;
static TaskHandle_t s_demo_task_handle;
static volatile uint32_t s_demo_period_ms = 200;

/* Các biến theo dõi trong debugger. */
volatile uint32_t so_lenh_dung = 0;
volatile uint32_t so_lenh_sai = 0;
volatile uint32_t so_lenh_qua_dai = 0;
volatile uint32_t so_lenh_co_thu_tu = 0;
volatile uint32_t so_thu_tu_cuoi = 0;

/* Đọc số nguyên không dấu và kiểm tra tràn số. */
static BaseType_t doc_so(
    const char *chuoi, // trỏ đến vị trí chuỗi cần đọc
    size_t do_dai,
    uint32_t *ket_qua)
{
    uint32_t gia_tri = 0;

    if (do_dai == 0) {
        return pdFALSE;
    }

    for (size_t vi_tri = 0; vi_tri < do_dai; vi_tri++) {
        if (chuoi[vi_tri] < '0' || chuoi[vi_tri] > '9') {
            return pdFALSE;
        }

        uint32_t chu_so = (uint32_t)(chuoi[vi_tri] - '0'); // trừ đi mã ASCII của 0 để lấy số nguyên

        if (gia_tri > (UINT32_MAX - chu_so) / 10U) { // Báo lỗi ngay lập tức, KHÔNG thực hiện phép tính gây tràn
            return pdFALSE;
        }

        gia_tri = gia_tri * 10U + chu_so;
    }

    *ket_qua = gia_tri;
    return pdTRUE;
}

static void apply_command(const char *cmd, size_t len)
{
    size_t vi_tri_dau_thang = len;
    uint32_t gia_tri = 0;
    uint32_t so_thu_tu = 0;
    BaseType_t co_so_thu_tu = pdFALSE;

    if (len < 2) {
        so_lenh_sai++;
        return;
    }

    /* Tìm phần số thứ tự, ví dụ P10#42. */
    for (size_t vi_tri = 1; vi_tri < len; vi_tri++) {
        if (cmd[vi_tri] == '#') {
            vi_tri_dau_thang = vi_tri;
            co_so_thu_tu = pdTRUE;
            break;
        }
    }

    if (doc_so(
            &cmd[1],
            vi_tri_dau_thang - 1,
            &gia_tri) != pdTRUE) {
        so_lenh_sai++;
        return;
    }

    if (co_so_thu_tu) {
        if (doc_so(
                &cmd[vi_tri_dau_thang + 1],
                len - vi_tri_dau_thang - 1,
                &so_thu_tu) != pdTRUE) {
            so_lenh_sai++;
            return;
        }
    }

    switch (cmd[0]) {
    case 'P': {
        /*
         * Chỉ chấp nhận chu kỳ chuyển được thành ít nhất 1 tick.
         * Tính bằng uint64_t để kiểm tra tràn trước khi ép kiểu.
         */
        uint64_t so_tick =
            ((uint64_t)gia_tri * configTICK_RATE_HZ) / 1000U;

        if (gia_tri == 0 ||
            so_tick == 0 ||
            so_tick >= (uint64_t)portMAX_DELAY) {
            so_lenh_sai++;
            return;
        }

        s_demo_period_ms = gia_tri;
        break;
    }

    case 'S':
        /*
         * Giữ parser ở ưu tiên cao nhất.
         * demo được phép dùng mức 0 ... configMAX_PRIORITIES - 2.
         */
        if (gia_tri >= (uint32_t)(configMAX_PRIORITIES - 1)) {
            so_lenh_sai++;
            return;
        }

        vTaskPrioritySet(
            s_demo_task_handle,
            (UBaseType_t)gia_tri
        ); // lệnh đổi giá trị ưu tiên
        break;

    case 'T':
        /* Chỉ nhận đúng T0 hoặc T1, có thể kèm #số_thứ_tự. */
        if (vi_tri_dau_thang != 2 || gia_tri > 1) { 
            so_lenh_sai++;
            return;
        }

        if (gia_tri == 0) {
            vTaskSuspend(s_demo_task_handle);
        } else {
            vTaskResume(s_demo_task_handle);
        }
        break;

    default:
        so_lenh_sai++;
        return;
    }

    so_lenh_dung++;

    if (co_so_thu_tu) {
        so_lenh_co_thu_tu++;
        so_thu_tu_cuoi = so_thu_tu;
    }
        char thong_bao[MAX_CMD_LEN + 8];

    int do_dai = snprintf(
        thong_bao,
        sizeof(thong_bao),
        "OK %.*s\r\n",
        (int)len,
        cmd
    );

    if (do_dai > 0 && do_dai < (int)sizeof(thong_bao)) {
        HAL_UART_Transmit(
            &huart1,
            (uint8_t *)thong_bao,
            (uint16_t)do_dai,
            100U
        );
    }
}

void parser_task(void *argument)
{
    (void)argument;

    char cmd[MAX_CMD_LEN];
    size_t idx = 0;
    BaseType_t bo_lenh = pdFALSE; // bật true khi phát hiện sai

    /* Chờ demo_task đăng ký handle trước khi nhận lệnh. */
    while (s_demo_task_handle == NULL) {
        vTaskDelay(1);
    }

    HAL_NVIC_EnableIRQ(USART1_IRQn);
const char thong_bao[] = "SAN SANG\r\n";

HAL_UART_Transmit(
    &huart1,
    (uint8_t *)thong_bao,
    sizeof(thong_bao) - 1,
    100U
);
    for (;;) {
        uint8_t byte;

        if (xStreamBufferReceive( // đọc dữ liệu từ bộ đệm rx_stream
                rx_stream,
                &byte,
                1,
                portMAX_DELAY) != 1) {
            continue;
        }

        if (byte == '\r') {
            if (!bo_lenh && idx > 0) {
                cmd[idx] = '\0';
                apply_command(cmd, idx); // chuyển chuỗi đã hoàn thiện sang hàm xử lí chính
            }

            idx = 0;
            bo_lenh = pdFALSE;
        } else if (byte == '\n') {
            /* Cho phép terminal gửi CRLF. */
            continue;
        } else if (!bo_lenh) { // Chỉ ghi nhận ký tự nếu chuỗi hiện tại chưa bị lỗi độ dài.
            if (idx < MAX_CMD_LEN - 1) {
                cmd[idx++] = (char)byte;
            } else {
                idx = 0;
                bo_lenh = pdTRUE;
                so_lenh_qua_dai++;
            }
        }
    }
}

void demo_task(void *argument)
{
    (void)argument;

    s_demo_task_handle = xTaskGetCurrentTaskHandle();

    for (;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

        /*
         * Đọc chu kỳ mới mỗi vòng.
         * Phép nhân 64 bit tránh tràn khi đổi ms sang tick.
         */
        TickType_t so_tick = (TickType_t)(
            ((uint64_t)s_demo_period_ms * configTICK_RATE_HZ)
            / 1000U
        );

        vTaskDelay(so_tick);
    }
}