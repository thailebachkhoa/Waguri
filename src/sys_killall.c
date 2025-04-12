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
#include "queue.h" // Add queue lib to traverse the process list

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
    while (data != -1)
    {
        libread(caller, memrg, i, &data);
        proc_name[i] = data;
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
    int killed = 0;
    struct queue_t *running_list = caller->running_list;
    struct queue_t *mlq_ready_queue = caller->mlq_ready_queue;
    struct queue_t *ready_queue = caller->ready_queue;
    // running list
    for (int i = 0; i < running_list->size; ++i)
    {
        struct pcb_t *pcb = dequeue(running_list);
        if (strcmp(pcb->path, proc_name) == 0)
        {
            // printf("Terminating running process: %s (PID: %d)\n", pcb->path, pcb->pid);
            free_pcb_memph(pcb);  
            killed++; // ???
        }
        else
        {
            enqueue(caller->running_list, pcb);
        }
    }

    // mlq ready queue
    for (int i = 0; i < mlq_ready_queue->size; ++i)
    {
        struct pcb_t *pcb = dequeue(caller->mlq_ready_queue);
        // if (strcmp(mlq_ready_queue->proc[i]->path, proc_name) == 0)
        if (strcmp(pcb->path, proc_name) == 0)
        {
            // printf("Terminating queued process: %s (PID: %d)\n", pcb->path, pcb->pid);
            free_pcb_memph(pcb);
            killed++;
        }
        else
        {
            enqueue(caller->mlq_ready_queue, pcb);
        }
    }

    // printf("Killed %d processes with name \"%s\"\n", killed, proc_name);
    //
    return killed > 0 ? 0 : -1;
}
