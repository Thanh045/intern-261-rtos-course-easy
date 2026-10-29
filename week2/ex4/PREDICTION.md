# Dự đoán — Week 2 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2550517  `EVENT_BIT_COUNT` = 4`SYNC_TIMEOUT_MS` = 80

1. RAM event group chiếm (bytes) dự đoán:  28 bytes. Dựa vào đâu (cấu trúc dữ
   liệu nào trong FreeRTOS)?  EventGroup_t trong event_groups.c: giá trị các bit, danh sách task đang chờ và thông tin cấp phát, tính cả header và căn chỉnh heap dự kiến khoảng 40 bytes.”
2. RAM thêm khi dùng task notification cho 1 điểm đồng bộ: dự đoán = 0 
   bytes. Vì sao? 
Vì notification đã có sẵn trong TCB khi bật configUSE_TASK_NOTIFICATIONS. Tuy nhiên, chương trình vẫn cần event group cho ba producer còn lại, nên không loại bỏ được bộ nhớ event group.
3. Dự đoán thời gian đánh thức (producer gọi → sync_task chạy) của notification
   so với event-group-bit: nhanh hơn / chậm hơn / xấp xỉ? Vì sao (nghĩ đến số
   lớp hàm bên dưới mỗi API)?
Em dự đoán notification nhanh hơn, vì báo trực tiếp đến một task cụ thể. Event group cần xử lý mặt nạ bit và kiểm tra điều kiện của các task đang chờ. 
4. Nếu `sync_task` bị timeout (`SYNC_TIMEOUT_MS` hết hạn) vì một producer
   không set bit kịp, `xEventGroupWaitBits` trả về gì? Làm sao phân biệt được
   "đủ bit" với "timeout"?
xEventGroupWaitBits() trả về giá trị các bit tại thời điểm kết thúc chờ, không nhất thiết bằng 0. Phải kiểm tra có đủ toàn bộ bit yêu cầu hay không: Ta có thể check mặt nạ bit bằng phép AND với mặt nạ 0x0F
(cac_bit_nhan_duoc & 0x0F) == 0x0F
5. Nêu trước một tình huống bạn nghĩ **không thể** thay toàn bộ event group
   bằng notification — sẽ kiểm chứng lại ở REPORT.md.
Khi nhiều task cùng chờ một sự kiện chung, một notification không thể trực tiếp đánh thức tất cả chúng vì notification thuộc về từng task riêng. Event group hỗ trợ nhiều task cùng chờ một bit, còn dùng notification phải thiết kế thêm việc báo riêng cho từng task