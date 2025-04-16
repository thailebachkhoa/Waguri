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
 
 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
 {
     char proc_name[100];
     uint32_t data;
 
     uint32_t memrg = regs->a1;
     
     /* TODO: Get name of the target proc */
     int i = 0;
     data = 0;
     while(data != (uint32_t)(-1)){
         libread(caller, memrg, i, &data);
         proc_name[i] = (char)data;
         if(data == -1) proc_name[i]='\0';
         i++;
     }
     printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
 

 
     int count = 0;
     

     if (caller->running_list != NULL) {
         struct queue_t *run_list = caller->running_list;
         for (i = 0; i < run_list->size; i++) {
             struct pcb_t *proc = run_list->proc[i];
             if (proc != NULL && strcmp(proc->path, proc_name) == 0) {
                 #ifdef MM_PAGING
                 if (proc->mm != NULL) {
                     free_pcb_memph(proc);
                 }
                 #endif

                 proc->pid = 0; 
                 count++;
                 
                 for (int j = i; j < run_list->size - 1; j++) {
                     run_list->proc[j] = run_list->proc[j + 1];
                 }
                 run_list->size--;
                 i--;
             }
         }
     }
     
     #ifdef MLQ_SCHED
     if (caller->mlq_ready_queue != NULL) {
         for (int prio = 0; prio < MAX_PRIO; prio++) {
             struct queue_t *queue = &caller->mlq_ready_queue[prio];
             for (i = 0; i < queue->size; i++) {
                 struct pcb_t *proc = queue->proc[i];
                 if (proc != NULL && strcmp(proc->path, proc_name) == 0) {
                     #ifdef MM_PAGING
                     if (proc->mm != NULL) {
                         free_pcb_memph(proc);
                     }
                     #endif
                     
                     proc->pid = 0;
                     count++;
                     
                     for (int j = i; j < queue->size - 1; j++) {
                         queue->proc[j] = queue->proc[j + 1];
                     }
                     queue->size--;
                     i--;
                 }
             }
         }
     }
     #else
     if (caller->ready_queue != NULL) {
         struct queue_t *ready_q = caller->ready_queue;
         for (i = 0; i < ready_q->size; i++) {
             struct pcb_t *proc = ready_q->proc[i];
             if (proc != NULL && strcmp(proc->path, proc_name) == 0) {
                 #ifdef MM_PAGING
                 if (proc->mm != NULL) {
                     free_pcb_memph(proc);
                 }
                 #endif
                 
                 proc->pid = 0;
                 count++;
                 
                 for (int j = i; j < ready_q->size - 1; j++) {
                     ready_q->proc[j] = ready_q->proc[j + 1];
                 }
                 ready_q->size--;
                 i--;
             }
         }
     }
     #endif
     
     printf("Terminated %d processes with name \"%s\"\n", count, proc_name);
      
     return count; 
 }
 