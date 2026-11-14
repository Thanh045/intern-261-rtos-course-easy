#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "exti_button.h"

#if EXTI_PIN != 1
#error "Code nay dung EXTI1 tren PB1"
#endif

#define CHAN_NUT ((uint16_t)(1U << EXTI_PIN))
#define CHAN_ACK ((uint16_t)(1U << ACK_PIN))

static TaskHandle_t s_ack_task_handle = NULL;

void exti_button_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE(); // AFIO giúp chọn pb1 là nguồn ngắt exti

    HAL_NVIC_DisableIRQ(EXTI1_IRQn); // Vô hiệu hóa ngắt EXTI1
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    GPIO_InitTypeDef cau_hinh = {0};

    HAL_GPIO_WritePin(GPIOA, CHAN_ACK, GPIO_PIN_RESET);

    cau_hinh.Pin = CHAN_ACK;
    cau_hinh.Mode = GPIO_MODE_OUTPUT_PP;
    cau_hinh.Pull = GPIO_NOPULL;
    cau_hinh.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &cau_hinh);

    cau_hinh.Pin = CHAN_NUT;
    cau_hinh.Mode = GPIO_MODE_IT_FALLING;// tạo ngắt khi chuyển từ 1 xuông 0 ( it_falling)
    cau_hinh.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &cau_hinh);

#if THU_PRIORITY_SAI
    HAL_NVIC_SetPriority(
        EXTI1_IRQn,
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 1U,
        0U
    );
#else
    HAL_NVIC_SetPriority(
        EXTI1_IRQn,
        ISR_NVIC_PRIO,
        0U
    );
#endif

    __HAL_GPIO_EXTI_CLEAR_IT(CHAN_NUT); // xóa cờ
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

void exti_button_isr_handler(void)
{
    BaseType_t task_uu_tien_cao_thuc_day = pdFALSE;// kiểm tra độ ưu tiên của task so với ngắt, luôn đặt pdFalse giá trị khởi tạo

    configASSERT(s_ack_task_handle != NULL); // kiểm tra con trỏ task phải được tạo

    vTaskNotifyGiveFromISR(
        s_ack_task_handle,
        &task_uu_tien_cao_thuc_day
    );

#if DUNG_YIELD_ISR
    portYIELD_FROM_ISR(task_uu_tien_cao_thuc_day); // nếu không có yield, phải đợi tới tick sau task mới chạy
#endif
}


void ack_task(void *argument)
{
    (void)argument;

    s_ack_task_handle = xTaskGetCurrentTaskHandle(); // task chạy mới lấy địa chỉ mình lưu vào rồi mới ngắt phần cứng

    __HAL_GPIO_EXTI_CLEAR_IT(CHAN_NUT);// xóa cờ ngắt
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);// xóa cờ pending trên nhân cortex
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        HAL_GPIO_TogglePin(GPIOA, CHAN_ACK);
    }
}