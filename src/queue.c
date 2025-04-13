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
        /* TODO: put a new process to queue [q] */
        if ( q != 0 || proc != 0 ||q->size < MAX_QUEUE_SIZE) q->proc[q->size++] = proc;
        else printf("Fail to enqueue process %s\n", proc->path);
}

struct pcb_t *dequeue(struct queue_t *q)
{
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        // Kiểm tra ngoại lệ
        if (empty(q)) return NULL;

        // Tìm index trong pcb có priority cao nhất trong queue [q]
        int toFindIndex = 0;
        for (int i = 1; i < q->size; i++)
        {
                int highestPriority = q->proc[toFindIndex]->priority;
                int currentPriority = q->proc[i]->priority;
                if (currentPriority < highestPriority) toFindIndex = i; 
        }
        // Xóa process có priority cao nhất
        struct pcb_t *process = q->proc[toFindIndex];
        for (int i = toFindIndex; i < q->size - 1; i++)
        {
                // Di chuyển các process còn lại về trước
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return process;
}
