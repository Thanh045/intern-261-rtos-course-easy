#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#define CHE_DO_CHO_QUEUE        0 // 0: ko chờ, 1: chờ 4 giây, 2: chờ tới hàng trống
#define THOI_GIAN_CHO_QUEUE_MS    4U
#define CHU_KY_GHI_DO_DAY_MS      2U
#define SO_MAU_GHI_DO_DAY         256U

#if CHE_DO_CHO_QUEUE == 0
#define THOI_GIAN_CHO_QUEUE ((TickType_t)0)
#elif CHE_DO_CHO_QUEUE == 1
#define THOI_GIAN_CHO_QUEUE pdMS_TO_TICKS(THOI_GIAN_CHO_QUEUE_MS)
#elif CHE_DO_CHO_QUEUE == 2
#define THOI_GIAN_CHO_QUEUE portMAX_DELAY
#else
#error "CHE_DO_CHO_QUEUE phai la 0, 1 hoac 2"
#endif

typedef struct {
    uint32_t seq; // số thứ tự
    int16_t ax;
    int16_t ay;
    int16_t az;
} raw_sample_t;

typedef struct {
    uint32_t seq;
    int16_t ax;
    int16_t ay;
    int16_t az;
} filtered_sample_t;

typedef struct {
    uint32_t thoi_gian_ms;
    uint16_t so_mau_queue_raw;
    uint16_t so_mau_queue_filtered;
} mau_do_day_t;

extern QueueHandle_t queue_raw;
extern QueueHandle_t queue_filtered;

extern volatile uint32_t ma_cam_bien;
extern volatile uint32_t so_lan_doc_cam_bien_loi;
extern volatile uint32_t so_mau_raw_gui_that_bai;
extern volatile uint32_t so_mau_loc_gui_that_bai;
extern volatile uint32_t so_mau_uart_nhan;
extern volatile uint32_t so_mau_thieu_tai_uart;
extern volatile uint32_t so_lan_uart_loi;

extern volatile uint32_t so_mau_do_day_da_ghi;
extern volatile mau_do_day_t bang_do_day[SO_MAU_GHI_DO_DAY];

void app_main(void);
void sensor_task(void *argument);
void filter_task(void *argument);
void uart_task(void *argument);

#endif