#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
    if (q == NULL)
    {
        printf("Queue is NULL\n");
        return 1;
    }
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
    if (q == NULL || proc == NULL)
    {
        printf("Queue or process is NULL\n");
        return;
    }

    if (q->size == 0)
    {
        q->proc[0] = proc;
        q->size++;
        return;
    }
    if (q->size < MAX_QUEUE_SIZE)
    {
        q->proc[q->size] = proc;
        q->size++;
    }
    else
        printf("Fail to enqueue\n");
}

struct pcb_t *dequeue(struct queue_t *q)
{
    /* TODO: return a pcb whose priority is the highest
     * in the queue [q] and remember to remove it from q
     * */
    // exception 
    if (empty(q))
    {
        printf("Queue is empty or NULL\n");
        return NULL;
    }

    #ifdef MLQ_SCHED

    struct pcb_t *proc = NULL;
    //get highest priority index
    int highest_prio_index = 0;
    for (int i = 1; i < MAX_PRIO; i++)
    {
        if (q->proc[i] != NULL && q->proc[i]->prio < q->proc[highest_prio_index]->prio)
        {
            highest_prio_index = i;
        }
    }
    proc = q->proc[highest_prio_index];
    // shift the rest of the queue
    for (int i = highest_prio_index; i < q->size - 1; i++)
    {
        q->proc[i] = q->proc[i + 1];
    }
    // free the last element
    q->proc[q->size - 1] = NULL;
    // decrease the size of the queue
    q->size--;
    return proc;
    #endif

    struct pcb_t *proc = NULL;
    if (q->proc[0] != NULL)
    {
        proc = q->proc[0];
        for (int i = 0; i < q->size - 1; i++)
        {
            q->proc[i] = q->proc[i + 1];
        }
        q->size--;
    }
    return proc;
}