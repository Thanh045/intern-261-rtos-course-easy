#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "params.h"
#include "pipeline.h"

_Static_assert(FILTER_WINDOW > 0, "FILTER_WINDOW phai lon hon 0");

volatile uint32_t so_mau_loc_gui_that_bai = 0;

void filter_task(void *argument)
{
    (void)argument;

    int16_t cua_so_ax[FILTER_WINDOW] = {0};
    int16_t cua_so_ay[FILTER_WINDOW] = {0};
    int16_t cua_so_az[FILTER_WINDOW] = {0};

    int32_t tong_ax = 0;
    int32_t tong_ay = 0;
    int32_t tong_az = 0;

    uint32_t vi_tri = 0;
    uint32_t so_mau_trong_cua_so = 0;

    for (;;) {
        raw_sample_t mau_raw;

        BaseType_t da_nhan = xQueueReceive(
            queue_raw,
            &mau_raw,
            portMAX_DELAY
        );

        if (da_nhan != pdPASS) {
            continue;
        }
// ý tưởng: lấy tổng đầu, trừ giá trị bị thay đổi rồi cộng giá trị thay đổi
        tong_ax -= cua_so_ax[vi_tri];
        tong_ay -= cua_so_ay[vi_tri];
        tong_az -= cua_so_az[vi_tri];

        cua_so_ax[vi_tri] = mau_raw.ax;
        cua_so_ay[vi_tri] = mau_raw.ay;
        cua_so_az[vi_tri] = mau_raw.az;

        tong_ax += mau_raw.ax;
        tong_ay += mau_raw.ay;
        tong_az += mau_raw.az;

        if (so_mau_trong_cua_so < FILTER_WINDOW) {
            so_mau_trong_cua_so++;
        }

        vi_tri++;

        if (vi_tri >= FILTER_WINDOW) {
            vi_tri = 0;
        }

        filtered_sample_t mau_loc = {0};

        mau_loc.seq = mau_raw.seq;

        mau_loc.ax = (int16_t)(
            tong_ax / (int32_t)so_mau_trong_cua_so
        );

        mau_loc.ay = (int16_t)(
            tong_ay / (int32_t)so_mau_trong_cua_so
        );

        mau_loc.az = (int16_t)(
            tong_az / (int32_t)so_mau_trong_cua_so
        );

        BaseType_t da_gui = xQueueSend(
            queue_filtered,
            &mau_loc,
            THOI_GIAN_CHO_QUEUE
        );

        if (da_gui != pdPASS) {
            so_mau_loc_gui_that_bai++;
        }
    }
}