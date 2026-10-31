#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "params.h"
#include "sync_tasks.h"

extern UART_HandleTypeDef huart1;
#define MAT_NA_TAT_CA_BIT \
    ((EventBits_t)((1UL << EVENT_BIT_COUNT) - 1UL))

#define SO_MAU_CAN_DO 1000U
#define TASK_CO_TIN_HIEU_CHAM 1U
#define HE_SO_TRE_THU_TIMEOUT 0U

static EventGroupHandle_t nhom_su_kien = NULL;

void *sync_task_handle = NULL;

volatile size_t heap_truoc_tao = 0;
volatile size_t heap_sau_tao = 0;
volatile size_t ram_nhom_su_kien = 0;
volatile size_t kich_thuoc_bo_nho_tinh = 0;

volatile uint32_t so_lan_dong_bo = 0;
volatile uint32_t so_lan_timeout = 0;
volatile uint32_t so_loi_uart = 0;

volatile uint32_t gia_tri_bit_tra_ve = 0;
volatile uint32_t so_thong_bao_nhan_duoc = 0;

volatile uint32_t thoi_diem_dong_bo_ms = 0;
volatile uint32_t chu_ky_dong_bo_ms = 0;

volatile uint32_t thoi_diem_bao_hieu = 0;
volatile uint32_t so_mau_da_do = 0;
volatile uint64_t tong_chu_ky_danh_thuc = 0;
volatile uint32_t chu_ky_danh_thuc_lon_nhat = 0;

static void dung_khi_loi(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;) {
    }
}

static uint32_t tao_so_ngau_nhien(uint32_t *trang_thai)
{
    uint32_t gia_tri = *trang_thai;

    gia_tri ^= gia_tri << 13;
    gia_tri ^= gia_tri >> 17;
    gia_tri ^= gia_tri << 5;

    *trang_thai = gia_tri;

    return gia_tri;
}

static void gui_log(
    uint32_t da_du_bit,
    EventBits_t cac_bit_nhan_duoc)
{
    char noi_dung[192];

    int do_dai = snprintf(
        noi_dung,
        sizeof(noi_dung),
        "ok=%lu time_ms=%lu period_ms=%lu " 
        "bits=0x%lx notify=%lu timeout=%lu\r\n",
        (unsigned long)da_du_bit,
        (unsigned long)thoi_diem_dong_bo_ms,
        (unsigned long)chu_ky_dong_bo_ms,
        (unsigned long)cac_bit_nhan_duoc,
        (unsigned long)so_thong_bao_nhan_duoc,
        (unsigned long)so_lan_timeout
    );

    if (do_dai <= 0 ||
        (size_t)do_dai >= sizeof(noi_dung)) {
        so_loi_uart++;
        return;
    }

    HAL_StatusTypeDef ket_qua = HAL_UART_Transmit(
        &huart1,
        (uint8_t *)noi_dung,
        (uint16_t)do_dai,
        1000U
    );

    if (ket_qua != HAL_OK) {
        so_loi_uart++;
    }
}

void sync_init(void)
{
    void *bo_nho_thu = pvPortMalloc(1);

    configASSERT(bo_nho_thu != NULL);

    if (bo_nho_thu == NULL) {
        dung_khi_loi();
    }

    vPortFree(bo_nho_thu);

    heap_truoc_tao = xPortGetFreeHeapSize();

    nhom_su_kien = xEventGroupCreate();

    configASSERT(nhom_su_kien != NULL);

    if (nhom_su_kien == NULL) {
        dung_khi_loi();
    }

    heap_sau_tao = xPortGetFreeHeapSize();

    ram_nhom_su_kien = heap_truoc_tao - heap_sau_tao;
    kich_thuoc_bo_nho_tinh = sizeof(StaticEventGroup_t);
}

void producer_task(void *argument)
{
    uint32_t ma_task = (uint32_t)(uintptr_t)argument;

    uint32_t trang_thai_ngau_nhien =
        0x12345678U ^ ((ma_task + 1U) * 0x9E3779B9U);

    configASSERT(ma_task < EVENT_BIT_COUNT);

    for (;;) {
#if CHE_DO_DO_TOC_DO

        if (ma_task != 0U) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(NOTIFY_PERIOD_MS));

        thoi_diem_bao_hieu = DWT->CYCCNT;

#if PRODUCER0_USE_NOTIFY

        BaseType_t ket_qua = xTaskNotifyGive(
            (TaskHandle_t)sync_task_handle
        );

        configASSERT(ket_qua == pdPASS);

        if (ket_qua != pdPASS) {
            dung_khi_loi();
        }

#else

        (void)xEventGroupSetBits(
            nhom_su_kien,
            (EventBits_t)1U
        );

#endif

#else

        uint32_t thoi_gian_nghi_ms =
            NOTIFY_PERIOD_MS + tao_so_ngau_nhien(&trang_thai_ngau_nhien) % 5U;

        if (ma_task == TASK_CO_TIN_HIEU_CHAM &&
            HE_SO_TRE_THU_TIMEOUT != 0U) {
            thoi_gian_nghi_ms =
                SYNC_TIMEOUT_MS * HE_SO_TRE_THU_TIMEOUT; // để tạo kịch bản một task vô tình đến chậm
        }

        vTaskDelay(pdMS_TO_TICKS(thoi_gian_nghi_ms));

#if PRODUCER0_USE_NOTIFY

        if (ma_task == 0U) {
            BaseType_t ket_qua = xTaskNotifyGive( 
                (TaskHandle_t)sync_task_handle
            );

            configASSERT(ket_qua == pdPASS);

            if (ket_qua != pdPASS) {
                dung_khi_loi();
            }
        } else {
            (void)xEventGroupSetBits(
                nhom_su_kien,
                (EventBits_t)(1UL << ma_task)
            );
        }

#else

        (void)xEventGroupSetBits(
            nhom_su_kien,
            (EventBits_t)(1UL << ma_task)
        );

#endif
#endif
    }
}

void sync_task(void *argument)
{
    (void)argument;

    const TickType_t gioi_han_cho =
        pdMS_TO_TICKS(SYNC_TIMEOUT_MS);

#if !CHE_DO_DO_TOC_DO
    uint32_t lan_dong_bo_truoc_ms = HAL_GetTick();
    BaseType_t da_co_moc_dong_bo = pdFALSE;
#endif

    for (;;) { // cài dặt lại mỗi vòng các bit
        EventBits_t cac_bit_nhan_duoc = 0;
        BaseType_t da_du_tin_hieu = pdFALSE;

        so_thong_bao_nhan_duoc = 0;

#if CHE_DO_DO_TOC_DO

#if PRODUCER0_USE_NOTIFY

        so_thong_bao_nhan_duoc = ulTaskNotifyTake(
            pdTRUE,
            gioi_han_cho
        );

        uint32_t thoi_diem_thuc_day = DWT->CYCCNT;

        da_du_tin_hieu = // kiểm tra xem task thức dậy do timeout hay vì đủ sự kiện
            so_thong_bao_nhan_duoc > 0U ? pdTRUE : pdFALSE;

#else

        cac_bit_nhan_duoc = xEventGroupWaitBits(
            nhom_su_kien,
            (EventBits_t)1U,
            pdTRUE, // tự động xóa các bit về 0 sau khi thoát hàm
            pdTRUE, // yêu cầu thỏa mãn các bit trong mặt nạ
            gioi_han_cho
        );

        uint32_t thoi_diem_thuc_day = DWT->CYCCNT;

        da_du_tin_hieu =
            (cac_bit_nhan_duoc & 1U) != 0U
            ? pdTRUE : pdFALSE;

#endif

        if (da_du_tin_hieu == pdTRUE) {
            uint32_t so_chu_ky_da_cho = (uint32_t)(
                thoi_diem_thuc_day - thoi_diem_bao_hieu
            );

            if (so_mau_da_do < SO_MAU_CAN_DO) {
                tong_chu_ky_danh_thuc += so_chu_ky_da_cho;

                if (so_chu_ky_da_cho >
                    chu_ky_danh_thuc_lon_nhat) {
                    chu_ky_danh_thuc_lon_nhat =
                        so_chu_ky_da_cho;
                }

                so_mau_da_do++;
            }
        } else {
            so_lan_timeout++;
        }

#else

#if PRODUCER0_USE_NOTIFY

        TickType_t bat_dau_cho = xTaskGetTickCount();

        so_thong_bao_nhan_duoc = ulTaskNotifyTake(
            pdTRUE,
            gioi_han_cho
        );

        if (so_thong_bao_nhan_duoc > 0U) {
            TickType_t da_cho =
                xTaskGetTickCount() - bat_dau_cho;

            TickType_t con_duoc_cho =
                da_cho < gioi_han_cho
                ? gioi_han_cho - da_cho : 0;

            EventBits_t mat_na_con_lai =
                MAT_NA_TAT_CA_BIT & ~(EventBits_t)1U;

            cac_bit_nhan_duoc = xEventGroupWaitBits(
                nhom_su_kien,
                mat_na_con_lai,
                pdTRUE,
                pdTRUE,
                con_duoc_cho
            );

            da_du_tin_hieu =
                (cac_bit_nhan_duoc & mat_na_con_lai)
                == mat_na_con_lai ? pdTRUE : pdFALSE;
        }

#else

        cac_bit_nhan_duoc = xEventGroupWaitBits(
            nhom_su_kien,
            MAT_NA_TAT_CA_BIT,
            pdTRUE,
            pdTRUE,
            gioi_han_cho
        );

        da_du_tin_hieu =
            (cac_bit_nhan_duoc & MAT_NA_TAT_CA_BIT)
            == MAT_NA_TAT_CA_BIT ? pdTRUE : pdFALSE;

#endif

        if (da_du_tin_hieu == pdTRUE) {
            uint32_t hien_tai_ms = HAL_GetTick();

            thoi_diem_dong_bo_ms = hien_tai_ms;

            chu_ky_dong_bo_ms =
                da_co_moc_dong_bo == pdTRUE
                ? hien_tai_ms - lan_dong_bo_truoc_ms
                : 0U;

            lan_dong_bo_truoc_ms = hien_tai_ms;
            da_co_moc_dong_bo = pdTRUE;

            so_lan_dong_bo++;
        } else {
            so_lan_timeout++;
        }

        gui_log(
            da_du_tin_hieu == pdTRUE ? 1U : 0U,
            cac_bit_nhan_duoc
        );

#endif

        gia_tri_bit_tra_ve = (uint32_t)cac_bit_nhan_duoc;
    }
}