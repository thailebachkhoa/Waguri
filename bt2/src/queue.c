#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE) {
                return; // Hàng đợi đầy hoặc tham số không hợp lệ
        }
        
        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
	if (q == NULL || q->size == 0) {
                return NULL; // Hàng đợi rỗng
        }
        
        // Tìm tiến trình có độ ưu tiên cao nhất (prio nhỏ nhất)
        int highest_prio_idx = 0;
        for (int i = 1; i < q->size; i++) {
                if (q->proc[i]->prio < q->proc[highest_prio_idx]->prio) {
                    highest_prio_idx = i;
                }
        }
        
        // Lưu tiến trình cần trả về
        struct pcb_t *proc = q->proc[highest_prio_idx];
        
        // Dịch chuyển các phần tử phía sau để xóa tiến trình
        for (int i = highest_prio_idx; i < q->size - 1; i++) {
                q->proc[i] = q->proc[i + 1];
        }
        q->proc[q->size - 1] = NULL; // Đặt phần tử cuối thành NULL
        q->size--;
        
        return proc;
}

