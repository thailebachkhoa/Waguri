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
#include "string.h"
#include <stdlib.h>
extern struct queue_t mlq_ready_queue[MAX_PRIO];

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while (data != -1) {
        libread(caller, memrg, i, &data);
        proc_name[i] = (char)data;
        i++;
    }
    proc_name[i - 1] = '\0';
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
      /* TODO: Traverse proclist to terminate the proc
     *       strcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queu
    int terminated_count = 0;
     // Duyệt qua từng mức ưu tiên
    for (int prio = 0; prio < MAX_PRIO; prio++) {
        struct queue_t *q = &mlq_ready_queue[prio];
        struct pcb_t *proc;
          // Giả định queue_t có con trỏ đầu tiên là proc, không cần init_queue hay is_empty
        while ((proc = dequeue(q)) != NULL) {
            if (strcmp(proc->path, proc_name) == 0) {
                printf("Terminating process PID %d with name \"%s\"\n", proc->pid, proc->path);
                terminated_count++;
                proc->pc = -1;  // Giải phóng tiến trình
            } else {
                enqueue(q, proc); // Giữ lại tiến trình không khớp
            }
        }
    }
    return terminated_count > 0 ? terminated_count : 0; 
        /* TODO Maching and terminating 
         *       all processes with given
         *        name in var proc_name
         */
}
 