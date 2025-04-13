#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if(q->size < MAX_QUEUE_SIZE){
                q->proc[q->size++] = proc;
        }
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
	if(empty(q)){
                return NULL;
               }
               int highIndex = 0;
               for(int i = 1;i < q->size;i++){
                if(q->proc[i]->priority < q->proc[highIndex]->priority){
                        highIndex = i;
                }
        
               }
               struct pcb_t * process = q->proc[highIndex];
               for(int i = highIndex;i < q->size - 1;i++){
                q->proc[i] = q->proc[i+1];
               }
               q->size--;
               return process;
}

