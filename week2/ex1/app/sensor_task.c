#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

extern I2C_HandleTypeDef hi2c1;

#define DIA_CHI_MPU6050         (0x68U << 1)
#define THANH_GHI_WHO_AM_I      0x75U
#define THANH_GHI_NGUON         0x6BU
#define THANH_GHI_ACCEL_CONFIG  0x1CU
#define THANH_GHI_ACCEL_XOUT    0x3BU

volatile uint32_t so_lan_doc_cam_bien_loi = 0;
volatile uint32_t so_mau_raw_gui_that_bai = 0;
volatile uint32_t ma_cam_bien = 0;

static void dung_khi_loi_cam_bien(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static int16_t ghep_hai_byte(uint8_t byte_cao, uint8_t byte_thap)
{
    uint16_t gia_tri =
        ((uint16_t)byte_cao << 8) | (uint16_t)byte_thap;

    return (int16_t)gia_tri;
}

static void khoi_tao_cam_bien(void)
{
    uint8_t ma = 0;

    vTaskDelay(pdMS_TO_TICKS(100U));

    HAL_StatusTypeDef ket_qua = HAL_I2C_Mem_Read(
        &hi2c1,
        DIA_CHI_MPU6050,
        THANH_GHI_WHO_AM_I,
        I2C_MEMADD_SIZE_8BIT,
        &ma,
        1U,
        100U
    );

    ma_cam_bien = ma;

    if (ket_qua != HAL_OK || (ma != 0x68U && ma != 0x70U)) {
        so_lan_doc_cam_bien_loi++;
        dung_khi_loi_cam_bien();
    }

    uint8_t cau_hinh = 0x00U;

    ket_qua = HAL_I2C_Mem_Write(
        &hi2c1,
        DIA_CHI_MPU6050,
        THANH_GHI_NGUON,
        I2C_MEMADD_SIZE_8BIT,
        &cau_hinh,
        1U,
        100U
    );

    if (ket_qua != HAL_OK) {
        so_lan_doc_cam_bien_loi++;
        dung_khi_loi_cam_bien();
    }

    vTaskDelay(pdMS_TO_TICKS(100U));

    ket_qua = HAL_I2C_Mem_Write(
        &hi2c1,
        DIA_CHI_MPU6050,
        THANH_GHI_ACCEL_CONFIG,
        I2C_MEMADD_SIZE_8BIT,
        &cau_hinh,
        1U,
        100U
    );

    if (ket_qua != HAL_OK) {
        so_lan_doc_cam_bien_loi++;
        dung_khi_loi_cam_bien();
    }
}

void sensor_task(void *argument)
{
    (void)argument;

    khoi_tao_cam_bien();

    uint32_t so_thu_tu = 0;
    TickType_t lan_thuc_truoc = xTaskGetTickCount();

    const TickType_t chu_ky = pdMS_TO_TICKS(SAMPLE_PERIOD_MS);

    configASSERT(chu_ky > 0);

    if (chu_ky == 0) {
        dung_khi_loi_cam_bien();
    }

    for (;;) {
        uint8_t du_lieu[6];
        raw_sample_t mau = {0};

        mau.seq = so_thu_tu++;
        

        HAL_StatusTypeDef ket_qua = HAL_I2C_Mem_Read(
            &hi2c1,
            DIA_CHI_MPU6050,
            THANH_GHI_ACCEL_XOUT,
            I2C_MEMADD_SIZE_8BIT,
            du_lieu,
            sizeof(du_lieu),
            5U
        );

        if (ket_qua == HAL_OK) {
            mau.ax = ghep_hai_byte(du_lieu[0], du_lieu[1]);
            mau.ay = ghep_hai_byte(du_lieu[2], du_lieu[3]);
            mau.az = ghep_hai_byte(du_lieu[4], du_lieu[5]);

            BaseType_t da_gui = xQueueSend(
                queue_raw,
                &mau,
                THOI_GIAN_CHO_QUEUE
            );

            if (da_gui != pdPASS) {
                so_mau_raw_gui_that_bai++;
            }
        } else {
            so_lan_doc_cam_bien_loi++;
        }

        vTaskDelayUntil(&lan_thuc_truoc, chu_ky);
    }
}