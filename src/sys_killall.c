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
    // caller->mlq_ready_queu

    /* TODO Maching and terminating
     *       all processes with given
     *        name in var proc_name
     */
    int count = 0;
    struct queue_t *running_list = caller->running_list;
    struct queue_t *mlq_ready_queue = caller->mlq_ready_queue;
    struct queue_t *ready_queue = caller->ready_queue;
    struct pcb_t *temp_proc;
    
    // Check process in running_list
    printf("Checking process in running_list\n");
    for (int i = 0; i < running_list->size; i++)
    {
        temp_proc = running_list->proc[i];
        if ( temp_proc != NULL &&strcmp(temp_proc->path, proc_name) == 0)
        {
            printf("Terminate process %s\n", running_list->proc[i]->path);
            free(running_list->proc[i]);
            running_list->size--;
            for (int j = i; j < running_list->size; j++)
            {
                running_list->proc[j] = running_list->proc[j + 1];
            }
            running_list->proc[running_list->size] = NULL;
            count++;
        }
    }
    // Check process in mlq_ready_queue
    printf("Checking process in mlq_ready_queue\n");
    for (int i = 0; i < mlq_ready_queue->size; i++)
    {
        if (strcmp(mlq_ready_queue->proc[i]->path, proc_name) == 0)
        {
            printf("Terminate process %s\n", mlq_ready_queue->proc[i]->path);
            free(mlq_ready_queue->proc[i]);
            mlq_ready_queue->size--;
            for (int j = i; j < mlq_ready_queue->size; j++)
            {
                mlq_ready_queue->proc[j] = mlq_ready_queue->proc[j + 1];
            }
            mlq_ready_queue->proc[mlq_ready_queue->size] = NULL;
            count++;
        }
    }
    // Check process in ready_queue ???
    printf("Checking process in ready_queue\n");
    for (int i = 0; i < ready_queue->size; i++)
    {
        if (strcmp(ready_queue->proc[i]->path, proc_name) == 0)
        {
            printf("Terminate process %s\n", ready_queue->proc[i]->path);
            free(ready_queue->proc[i]);
            ready_queue->size--;
            for (int j = i; j < ready_queue->size; j++)
            {
                ready_queue->proc[j] = ready_queue->proc[j + 1];
            }
            ready_queue->proc[ready_queue->size] = NULL;
            count ++;
        }
    }
    printf("Total %d processes terminated\n", count);
    return count;
}
