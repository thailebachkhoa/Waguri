/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"

#include "queue.h"
#include <string.h>

int __sys_killall(struct pcb_t *caller, struct sc_regs *regs)
{
    char proc_name[100];
    uint32_t data;

    // hardcode for demo only
    uint32_t memrg = regs->a1;

    /* TODO: Get name of the target proc */
    // proc_name = libread..
    int i = 0;
    data = 0;
    while (data != (uint32_t)(-1))
    {
        libread(caller, memrg, i, &data);
        proc_name[i] = (char)data;
        if (data == -1)
            proc_name[i] = '\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    // caller->running_list
    // caller->mlq_ready_queue

    /* TODO Maching and terminating
     *       all processes with given
     *        name in var proc_name
     */

    int count = 0;
    // execption handling
    if (caller->running_list == NULL)
    {
        printf("No running list found.\n");
        return -1; // Error: No running list
    }
    struct queue_t *run_list = caller->running_list;
    if (run_list->size == 0)
    {
        printf("No processes in the running list.\n");
        return -1; // Error: No processes in the running list
    }

    // kill run-list
    for (i = 0; i < run_list->size; i++)
    {
        // pick process from the list
        struct pcb_t *proc = run_list->proc[i];
        if (proc == NULL)
            continue;
        if (strcmp(proc->path, proc_name) == 0)
        {

#ifdef MM_PAGING
            if (proc->mm != NULL)
                free_pcb_memph(proc);
#endif
            count++;
            // free memory resources
            proc = NULL;

            /* Shift run list */
            for (int j = i; j < run_list->size - 1; j++)
            {
                run_list->proc[j] = run_list->proc[j + 1];
            }
            run_list->size--;
            i--;
        }
    }

#ifdef MLQ_SCHED
    /* MLQ kill */
    if (caller->mlq_ready_queue == NULL)
    {
        printf("No MLQ ready queue found.\n");
        return -1; // Error: No MLQ ready queue
    }
    
    // kill mlq
    for (int prio = 0; prio < MAX_PRIO; prio++)
    {
        struct queue_t *queue = &caller->mlq_ready_queue[prio]; if (queue == NULL) continue;
        for (i = 0; i < queue->size; i++)
        {
            struct pcb_t *proc = queue->proc[i]; if (proc == NULL) continue;
            if (strcmp(proc->path, proc_name) == 0)
            {
/* Free memory resources */
#ifdef MM_PAGING
                if (proc->mm != NULL) free_pcb_memph(proc);
#endif

                /* Mark process as terminated */
                proc = NULL;
                count++;
                /* Remove terminated process from the queue */
                for (int j = i; j < queue->size - 1; j++)
                {
                    queue->proc[j] = queue->proc[j + 1];
                }
                queue->size--;
                i--;
            }
        }
    }

#else
    /* Find and terminate processes in the ready queue */
    if (caller->ready_queue != NULL)
    {
        struct queue_t *ready_q = caller->ready_queue; if (ready_q == NULL) return -1; // Error: No ready queue
        for (i = 0; i < ready_q->size; i++)
        {
            struct pcb_t *proc = ready_q->proc[i]; if (proc == NULL) continue;
            if (strcmp(proc->path, proc_name) == 0)
            {
#ifdef MM_PAGING
                if (proc->mm != NULL) free_pcb_memph(proc);
#endif

                proc = NULL;
                count++;

                /* Shift */
                for (int j = i; j < ready_q->size - 1; j++)
                {
                    ready_q->proc[j] = ready_q->proc[j + 1];
                }
                ready_q->size--;
                i--;
            }
        }
    }
#endif

    printf("Total %d processes with name \"%s\" terminated.\n", count, proc_name);
    return count; 
}
