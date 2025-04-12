#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
        if (q == NULL)
        {
                printf("Queue is NULL\n");
                return -1;
        }
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */
        if (q->size < MAX_QUEUE_SIZE)
                q->proc[q->size++] = proc;
}

struct pcb_t *dequeue(struct queue_t *q)
{
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        // Kiểm tra ngoại lệ
        if (empty(q))
                return NULL;

        // Tìm priority cao nhất trong queue [q]
        int highestIndex = 0;
        for (int i = 1; i < q->size; i++)
        {
                if (q->proc[i]->priority < q->proc[highestIndex]->priority)
                {
                        highestIndex = i;
                }
        }
        // Xóa process có priority cao nhất
        struct pcb_t *process = q->proc[highestIndex];
        for (int i = highestIndex; i < q->size - 1; i++)
        {
                // Di chuyển các process còn lại về trước
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return process;
}
