#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

// Hàm kiểm tra xem tất cả hàng đợi có rỗng hay không
int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

// Hàm khởi tạo hàng đợi và mutex
void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i;
	for (i = 0; i < MAX_PRIO; i++) {
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i; 
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED

// Lấy tiến trình từ hàng đợi MLQ có độ ưu tiên cao nhất
struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	pthread_mutex_lock(&queue_lock);
	unsigned long prio = 0;
	while (prio < MAX_PRIO) {
		if (!empty(&mlq_ready_queue[prio])) {
			proc = dequeue(&mlq_ready_queue[prio]);
			break;
		}
		prio++;
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;	
}

// Đưa tiến trình vào hàng đợi MLQ theo độ ưu tiên
void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

// Thêm tiến trình mới vào hàng đợi MLQ theo độ ưu tiên
void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

// Hàm chung để lấy tiến trình (dành cho MLQ)
struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

// Hàm chung để đưa tiến trình đang chạy vào running_list và hàng đợi MLQ
void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	return put_mlq_proc(proc);
}

// Hàm chung để thêm tiến trình mới vào running_list và hàng đợi MLQ
void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	return add_mlq_proc(proc);
}

#else

// Lấy tiến trình từ hàng đợi ready_queue (non-MLQ)
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	pthread_mutex_lock(&queue_lock);
	if (!empty(&ready_queue)) {
		proc = dequeue(&ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

// Đưa tiến trình đang chạy vào running_list (non-MLQ)
void put_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	pthread_mutex_unlock(&queue_lock);
}

// Thêm tiến trình mới vào running_list (non-MLQ)
void add_proc(struct pcb_t * proc) {
	proc->ready_queue = &ready_queue;
	proc->running_list = &running_list;

	pthread_mutex_lock(&queue_lock);
	enqueue(&running_list, proc);
	pthread_mutex_unlock(&queue_lock);	
}

#endif
