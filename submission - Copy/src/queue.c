#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Kiểm tra hàng đợi có rỗng không
int empty(struct queue_t * q) {
    if (q == NULL) return 1;
    return (q->size == 0);
}

// Thêm tiến trình vào cuối hàng đợi nếu còn chỗ
void enqueue(struct queue_t * q, struct pcb_t * proc) {
    if(q->size < MAX_QUEUE_SIZE){
        q->proc[q->size++] = proc;
    }
}

// Lấy ra tiến trình có độ ưu tiên cao nhất (priority nhỏ nhất)
struct pcb_t * dequeue(struct queue_t * q) {
    if(empty(q)) return NULL;

    int highIndex = 0;
    // Tìm vị trí tiến trình có priority thấp nhất
    for(int i = 1; i < q->size; i++){
        if(q->proc[i]->priority < q->proc[highIndex]->priority){
            highIndex = i;
        }
    }

    struct pcb_t * process = q->proc[highIndex];

    // Dịch các phần tử để xóa tiến trình khỏi hàng đợi
    for(int i = highIndex; i < q->size - 1; i++){
        q->proc[i] = q->proc[i + 1];
    }

    q->size--;
    return process;
}
