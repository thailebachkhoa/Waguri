#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// Các hàng đợi và mutex dùng chung
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
static struct queue_t running_list;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];  // Mỗi mức ưu tiên có một hàng đợi
static int slot[MAX_PRIO];                        // Số lượt xử lý cho mỗi mức ưu tiên
#endif

// Kiểm tra xem toàn bộ hàng đợi có rỗng không
int queue_empty(void) {
#ifdef MLQ_SCHED
	for (int prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

// Khởi tạo bộ lập lịch và các hàng đợi
void init_scheduler(void) {
#ifdef MLQ_SCHED
	for (int i = 0; i < MAX_PRIO; i++) {
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i;  // Ưu tiên cao hơn thì có nhiều slot hơn
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED

// Lấy tiến trình có ưu tiên cao nhất từ các hàng đợi MLQ
struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	pthread_mutex_lock(&queue_lock);
	for (int i = 0; i < MAX_PRIO; i++) {
		if (!empty(&mlq_ready_queue[i])) {
			proc = dequeue(&mlq_ready_queue[i]);
			break;
		}
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;	
}

// Đưa tiến trình trở lại hàng đợi MLQ theo mức ưu tiên
void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

// Thêm tiến trình mới vào hàng đợi MLQ
void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);    
}

// Interface chung — thực chất gọi hàm MLQ
struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

// Đưa tiến trình về lại hệ thống (cập nhật cả trạng thái chạy và hàng đợi)
void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);  // Đánh dấu là đang chạy
	pthread_mutex_unlock(&queue_lock);

	return put_mlq_proc(proc);     // Đưa lại vào hàng đợi MLQ
}

// Thêm tiến trình mới vào hệ thống
void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);  // Đưa vào hàng đợi ready (dùng thống kê)
	pthread_mutex_unlock(&queue_lock);

	return add_mlq_proc(proc);    // Thêm vào hàng đợi theo priority
}

#else  // Nếu KHÔNG dùng MLQ (chế độ đơn giản)

// Lấy tiến trình từ ready_queue
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	pthread_mutex_lock(&queue_lock);
	proc = dequeue(&ready_queue);
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

// Đưa tiến trình đang chạy vào running_list và run_queue
void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

// Thêm tiến trình vào hệ thống (chỉ dùng ready_queue và running_list)
void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);    
}
#endif
