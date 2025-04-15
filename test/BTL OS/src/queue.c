#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
        if (q == NULL)
        {
                fprintf(stderr, "Error: queue is NULL\n");
                return -1;
        }
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */

        // exception handling
        if (q == NULL)
        {
                fprintf(stderr, "Error: queue is NULL\n");
                return;
        }
        if (proc == NULL)
        {
                fprintf(stderr, "Error: process is NULL\n");
                return;
        }
        if (q->size >= MAX_QUEUE_SIZE)
                return;
        // main action
        q->proc[q->size++] = proc;
}

struct pcb_t *dequeue(struct queue_t *q)
{
        // exception handling
        if (empty(q))
        {
                fprintf(stderr, "Error: queue is empty\n");
                return NULL;
        }

        // shift the queue
        struct pcb_t *proc = q->proc[0];
        for (int i = 1; i < q->size; i++) q->proc[i - 1] = q->proc[i];
        q->size--;
        q->proc[q->size] = NULL; // clear the last element
        return proc;
}
