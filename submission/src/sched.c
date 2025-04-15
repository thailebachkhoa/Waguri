#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// Các hàng đợi tiến trình toàn cục
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
static struct queue_t running_list;  // Danh sách tiến trình đang chạy

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];  // Mỗi mức ưu tiên có một hàng đợi riêng
static int slot[MAX_PRIO];  // Dùng để hỗ trợ kỹ thuật "slot" trong MLQ
#endif

// Hàm kiểm tra toàn bộ hàng đợi có rỗng không
int queue_empty(void) {
#ifdef MLQ_SCHED
    unsigned long prio;
    for (prio = 0; prio < MAX_PRIO; prio++)
        if (!empty(&mlq_ready_queue[prio])) 
            return -1;  // Có ít nhất 1 hàng đợi còn tiến trình
#endif
    return (empty(&ready_queue) && empty(&run_queue));
}

// Hàm khởi tạo bộ lập lịch
void init_scheduler(void) {
#ifdef MLQ_SCHED
    for (int i = 0; i < MAX_PRIO; i++) {
        mlq_ready_queue[i].size = 0;
        slot[i] = MAX_PRIO - i;  // Ưu tiên cao được cấp nhiều slot hơn
    }
#endif
    ready_queue.size = 0;
    run_queue.size = 0;
    pthread_mutex_init(&queue_lock, NULL);  // Khởi tạo mutex
}

#ifdef MLQ_SCHED
/*
 * Lấy tiến trình tiếp theo từ hàng đợi MLQ (ưu tiên cao nhất trước)
 */
struct pcb_t * get_mlq_proc(void) {
    struct pcb_t * proc = NULL;
    pthread_mutex_lock(&queue_lock);
    for (int i = 0; i < MAX_PRIO; i++) {
        if (!empty(&mlq_ready_queue[i])) {
            proc = dequeue(&mlq_ready_queue[i]);  // Lấy tiến trình đầu tiên có ưu tiên cao
            break;
        }
    }
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

// Đưa tiến trình trở lại hàng đợi MLQ (ví dụ: khi bị preempt)
void put_mlq_proc(struct pcb_t * proc) {
    pthread_mutex_lock(&queue_lock);
    enqueue(&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock(&queue_lock);
}

// Thêm tiến trình mới vào hàng đợi MLQ theo mức ưu tiên
void add_mlq_proc(struct pcb_t * proc) {
    pthread_mutex_lock(&queue_lock);
    enqueue(&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock(&queue_lock);    
}

// Interface dùng chung trong code: thực chất gọi hàm bên MLQ
struct pcb_t * get_proc(void) {
    return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
    // Gán các hàng đợi liên quan cho tiến trình
    proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    enqueue(&running_list, proc);  // Cho biết tiến trình đang chạy
    pthread_mutex_unlock(&queue_lock);

    return put_mlq_proc(proc);  // Đưa vào lại hàng đợi theo mức ưu tiên
}

void add_proc(struct pcb_t * proc) {
    proc->ready_queue = &ready_queue;
    proc->mlq_ready_queue = mlq_ready_queue;
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    enqueue(&ready_queue, proc);  // Có thể chỉ dùng cho mục đích thống kê
    pthread_mutex_unlock(&queue_lock);

    return add_mlq_proc(proc);
}
#else

// Khi không bật MLQ — chỉ có 1 hàng đợi ready_queue
struct pcb_t * get_proc(void) {
    struct pcb_t * proc = NULL;
    pthread_mutex_lock(&queue_lock);
    proc = dequeue(&ready_queue);  // Lấy tiến trình đầu tiên trong hàng đợi
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_proc(struct pcb_t * proc) {
    proc->ready_queue = &ready_queue;
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    enqueue(&running_list, proc);  // Ghi nhận đang chạy
    pthread_mutex_unlock(&queue_lock);

    pthread_mutex_lock(&queue_lock);
    enqueue(&run_queue, proc);  // Đưa lại vào hàng đợi đang chạy
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
    proc->ready_queue = &ready_queue;
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    enqueue(&running_list, proc);  // Cho biết đã bắt đầu chạy
    pthread_mutex_unlock(&queue_lock);

    pthread_mutex_lock(&queue_lock);
    enqueue(&ready_queue, proc);  // Đưa tiến trình vào hàng đợi sẵn sàng
    pthread_mutex_unlock(&queue_lock);    
}
#endif
