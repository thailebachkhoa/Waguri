# Introduction:

Ba khái niệm quan trọng: lập lịch - Scheduling, đồng bộ hóa - Synchronization, quản lí bộ nhớ - mem management
Tài nguyên OS: cpu và Ram
Mục tiêu: thực hiện bộ lập lịch và hoàn thành cơ chế bộ nhớ ảo

# giải thích file common.h:



# Thông tin của một process -> commmon.h -> pcb

# Scheduler:
keyword: multilevel queue, multiprocessors
thuật toán multilevel queue: 

## Flowchart:

new: 
một program mới -> process mới 
lắp pcb ( lật include commmon.h/pcb.t, đây là struct thông tin của process ): copy nội dung từ program vào pcb, quan sát lại struct pcb / code pointer
mô tả quá trình: disk (program in code) - loader.c lắp pcb > program in excution (processs)

ready: add_dequeue () -> ready queue

running: get_proc() -> multi processors -> chạy process

redo: xong quay lại bước ready


## nhiệm vụ: chú ý 2 file

queue.c: enqueue and dequeue, xóa process 
get_proc: lấy process từ hàng đợi

