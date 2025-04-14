#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
    if (q == NULL || proc == NULL) return; 

    if (q->size < MAX_QUEUE_SIZE) {
        q->proc[q->size] = proc; 
        q->size++;
    }
}

    

struct pcb_t * dequeue(struct queue_t * q) {
    if (q == NULL || q->size == 0) return NULL;

    struct pcb_t *highest_proc = q->proc[0];

    for (int i = 0; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i + 1]; 
    }
    
    q->proc[q->size - 1] = NULL; 
    q->size--;
    
    return highest_proc;
}

    


