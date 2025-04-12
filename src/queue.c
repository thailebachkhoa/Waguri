#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) {
                printf("Queue is not initial \n");
                return -1;
        }
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if(q->size < MAX_QUEUE_SIZE) q->proc[q->size++] = proc;
        else printf("Failed to enqueue\n");
}
struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        if (empty(q)) {
                printf("Queue is empty\n");
                return NULL;
        }
        // Find the process with the highest priority
        int highestIndex = 0;
        for (int i = 1; i < q->size; i++) {
                if (q->proc[i]->priority > q->proc[highestIndex]->priority) {
                        highestIndex = i;
                }
        }
        // Store the process to return
        struct pcb_t * highestPriorityProc = q->proc[highestIndex];
        for (int i = highestIndex; i < q->size - 1; i++) {
                // Shift the processes to fill the gap
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return highestPriorityProc;
}

