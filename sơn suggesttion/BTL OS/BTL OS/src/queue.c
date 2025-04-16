#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Kiểm tra hàng đợi có rỗng không
int empty(struct queue_t * q) {
    if (q == NULL) return 1;
    return (q->size == 0);
}

// Thêm tiến trình vào hàng đợi
void enqueue(struct queue_t * q, struct pcb_t * proc) {
    if (q->size >= MAX_QUEUE_SIZE)     // Hàng đợi đầy
        return;
    q->proc[q->size] = proc;           // Thêm vào cuối hàng
    q->size++;                         // Tăng kích thước hàng
}

// Lấy tiến trình đầu tiên ra khỏi hàng đợi
struct pcb_t * dequeue(struct queue_t * q) {
    if (q->size == 0)                  // Hàng đợi rỗng
        return NULL;
    struct pcb_t *proc = q->proc[0];   // Lấy tiến trình đầu
    for (int i = 1; i < q->size; i++)  // Dời các tiến trình còn lại
        q->proc[i - 1] = q->proc[i];
    q->size--;                         // Giảm kích thước hàng
    return proc;
}
