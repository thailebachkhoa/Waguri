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

#include "string.h"
#include "queue.h"
#include <stdlib.h>

static pthread_mutex_t queue_lock; // add
                                   // role:  giải phóng các vùng nhớ của process
void terminate_process(struct pcb_t *pcb)
{
    for (int i = 0; i < 10; i++)
    {
        if (pcb->regs[i] != 0)
        {
            libfree(pcb, i);
            pcb->regs[i] = 0;
        }
    }

    if (pcb->code)
    {
        if (pcb->code->text)
            free(pcb->code->text);
        free(pcb->code);
        pcb->code = NULL;
    }

#ifdef MM_PAGING
    if (pcb->mm)
    {

        if (pcb->mm->pgd)
        {
            free(pcb->mm->pgd);
            pcb->mm->pgd = NULL;
        }

        struct vm_area_struct *vma = pcb->mm->mmap;
        while (vma)
        {
            struct vm_area_struct *next = vma->vm_next;

            struct vm_rg_struct *rg = vma->vm_freerg_list;
            while (rg)
            {
                struct vm_rg_struct *next_rg = rg->rg_next;
                free(rg);
                rg = next_rg;
            }
            free(vma);
            vma = next;
        }

        struct pgn_t *pgn = pcb->mm->fifo_pgn;
        while (pgn)
        {
            struct pgn_t *next = pgn->pg_next;
            free(pgn);
            pgn = next;
        }
        free(pcb->mm);
        pcb->mm = NULL;
    }
    if (pcb->mram)
    {

        free(pcb->mram);
        pcb->mram = NULL;
    }
    if (pcb->mswp)
    {

        free(pcb->mswp);
        pcb->mswp = NULL;
    }
    if (pcb->active_mswp)
    {
        pcb->active_mswp = NULL;
    }
#endif

    if (pcb->page_table)
    {
        for (int i = 0; i < pcb->page_table->size; i++)
        {
            if (pcb->page_table->table[i].next_lv)
            {
                free(pcb->page_table->table[i].next_lv);
                pcb->page_table->table[i].next_lv = NULL;
            }
        }
        free(pcb->page_table);
        pcb->page_table = NULL;
    }
}

// role:  xóa process khỏi queue theo index
void remove_from_queue(struct queue_t *queue, int index)
{
    for (int k = index; k < (queue->size - 1); k++)
    {
        queue->proc[k] = queue->proc[k + 1];
    }
    queue->size--;
}

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

    //  /* TODO: Traverse proclist to terminate the proc
    //   *       stcmp to check the process match proc_name
    //   */
    //  //caller->running_list
    //  //caller->mlq_ready_queu

    //  for(int prio = 0; prio < MAX_PRIO; prio++){
    //      struct queue_t *queue = &caller->mlq_ready_queue[prio];
    //      if(queue->size == 0) continue;
    //      int j = 0;
    //      while(j < queue->size){
    //          struct pcb_t *proc = queue->proc[j];
    //          char *proc_name_in_path = strrchr(proc->path, '/');
    //          if(proc_name_in_path) proc_name_in_path++;
    //          else proc_name_in_path = proc->path;

    //          if(strcmp(proc_name_in_path, proc_name) == 0 && proc != caller ){
    //              printf("Terminating process %d with name %s from mlq_ready_queue[%d]\n",
    //                      proc->pid, proc->path, prio);
    //              terminate_process(proc);
    //              remove_from_queue(queue, j);
    //              free(proc);
    //          } else{
    //              j++;
    //          }
    //      }
    //  }
    //  struct queue_t *running_queue = caller->running_list;
    //  int j = 0;
    //  while( j < running_queue->size){
    //      struct pcb_t *proc = running_queue->proc[j];
    //      char *proc_name_in_path = strrchr(proc->path, '/');
    //      if(proc_name_in_path) proc_name_in_path++;
    //      else proc_name_in_path = proc->path;

    //      if(strcmp(proc_name_in_path, proc_name) == 0 && proc != caller ){
    //          printf("Terminating process %d with name %s from running_list\n",
    //                  proc->pid, proc->path);
    //          terminate_process(proc);
    //          remove_from_queue(running_queue, j);
    //          free(proc);
    //      } else{
    //          j++;
    //      }
    //  }

    //  /* TODO Maching and terminating
    //   *       all processes with given
    //   *        name in var proc_name
    //   */

    //  return 0;

    /* TODO: Traverse process list to terminate the process
     * Use strcmp to check if the process matches proc_name
     */
    struct pcb_t *proc = NULL;

    // Traverse the running list
    pthread_mutex_lock(&queue_lock);
    for (int i = 0; i < caller->running_list->size; i++)
    {
        proc = caller->running_list->proc[i];
        char *last_slash = strrchr(proc->path, '/');
        char *current_name = last_slash ? last_slash + 1 : proc->path;
        if (strcmp(current_name, proc_name) == 0)
        {
            printf("Terminating process %d with name \"%s\"\n", proc->pid, proc->path);
            proc->pc = proc->code->size;
        }
    }
    pthread_mutex_unlock(&queue_lock);

#ifdef MLQ_SCHED
    // Traverse the MLQ ready queue for each priority level
    for (int prio = 0; prio < MAX_PRIO; prio++)
    {
        pthread_mutex_lock(&queue_lock);
        for (int i = 0; i < caller->mlq_ready_queue[prio].size; i++)
        {
            proc = caller->mlq_ready_queue[prio].proc[i];
            char *last_slash = strrchr(proc->path, '/');
            char *current_name = last_slash ? last_slash + 1 : proc->path;
            if (strcmp(current_name, proc_name) == 0)
            {
                printf("Terminating process %d with name \"%s\" from priority %d queue\n",
                       proc->pid, proc->path, prio);
                proc->pc = proc->code->size;
            }
        }
        pthread_mutex_unlock(&queue_lock);
    }
#endif
}
