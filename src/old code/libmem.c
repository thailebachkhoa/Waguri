// /*
//  * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
//  */

// /* Sierra release
//  * Source Code License Grant: The authors hereby grant to Licensee
//  * personal permission to use and modify the Licensed Source Code
//  * for the sole purpose of studying while attending the course CO2018.
//  */

// // #ifdef MM_PAGING
// /*
//  * System Library
//  * Memory Module Library libmem.c
//  */

// #include "string.h"
// #include "mm.h"
// #include "syscall.h"
// #include "libmem.h"
// #include <stdlib.h>
// #include <stdio.h>
// #include <pthread.h>

// static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

// /*enlist_vm_freerg_list - add new rg to freerg_list
//  *@mm: memory region
//  *@rg_elmt: new region
//  *
//  */
// int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
// {
//   struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

//   if (rg_elmt->rg_start >= rg_elmt->rg_end)
//     return -1;

//   if (rg_node != NULL)
//     rg_elmt->rg_next = rg_node;

//   /* Enlist the new region */
//   mm->mmap->vm_freerg_list = rg_elmt;

//   return 0;
// }

// /*get_symrg_byid - get mem region by region ID
//  *@mm: memory region
//  *@rgid: region ID act as symbol index of variable
//  *
//  */
// struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
// {
//   if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
//     return NULL;

//   return &mm->symrgtbl[rgid];
// }

// /*__alloc - allocate a region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *@alloc_addr: address of allocated memory region
//  *
//  */
// int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
// {
//   /*Allocate at the toproof */
//   struct vm_rg_struct rgnode;

//   /* TODO: commit the vmaid */
//   // rgnode.vmaid

//   if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
//   {
//     caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
//     caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

//     *alloc_addr = rgnode.rg_start;

//     pthread_mutex_unlock(&mmvm_lock);
//     return 0;
//   }

//   /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

//   /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
//   /*Attempt to increate limit to get space */
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   int inc_sz = PAGING_PAGE_ALIGNSZ(size);
//   int inc_limit_ret;

//   /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
//   int old_sbrk = cur_vma->sbrk;

//   /* TODO INCREASE THE LIMIT as inovking systemcall
//    * sys_memap with SYSMEM_INC_OP
//    */
//   struct sc_regs regs;
//   regs.a1 = SYSMEM_INC_OP;
//   regs.a2 = vmaid;
//   regs.a3 = inc_sz;

//   syscall(caller, 17, &regs);
//   // alloc_addr = &old_sbrk;
//   if (get_free_vmrg_area(caller, vmaid, size, &rgnode) != 0)
//   {
//     pthread_mutex_unlock(&mmvm_lock);
//     return -1;
//   }
//   caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
//   caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
//   *alloc_addr = caller->mm->symrgtbl[rgid].rg_start;

//   cur_vma->sbrk += inc_sz;
//   inc_limit_ret = cur_vma->sbrk;

//   /* TODO: commit the allocation address

//   // *alloc_addr = ...
//   */
//   // caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
//   // caller->mm->symrgtbl[rgid].rg_end = old_sbrk + inc_sz;

//   return 0;
// }

// /*__free - remove a region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *
//  */
// int __free(struct pcb_t *caller, int vmaid, int rgid)
// {
//   struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));

//   // Dummy initialization for avoding compiler dummay warning
//   // in incompleted TODO code rgnode will overwrite through implementing
//   // the manipulation of rgid later

//   if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
//     return -1;
//   struct vm_rg_struct *free_region = get_symrg_byid(caller->mm, rgid);
//   int free_start = free_region->rg_start;
//   int free_end = free_region->rg_end;

//   // freed free_region
//   free_region->rg_start = -1;
//   free_region->rg_end = -1;

//   struct vm_rg_struct *new_free_area = (struct vm_rg_struct *)malloc(sizeof(struct vm_area_struct));
//   new_free_area->rg_start = free_start;
//   new_free_area->rg_end = free_end;
//   // new_free_area->vmaid = vmaid;
//   /*enlist the obsoleted memory region */
//   enlist_vm_freerg_list(caller->mm, new_free_area);
//   return 0;
// }

// /*liballoc - PAGING-based allocate a region memory
//  *@proc:  Process executing the instruction
//  *@size: allocated size
//  *@reg_index: memory region ID (used to identify variable in symbole table)
//  */
// int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
// {
//   /* TODO Implement allocation on vm area 0 */

//   int addr;
//   int free_id = -1;

//   if (proc->mm->symrgtbl[reg_index].rg_start == -1 &&
//       proc->mm->symrgtbl[reg_index].rg_end == -1)
//   {
//     free_id = reg_index;
//   }
//   else
//   {
//     for (int i = 0; i < PAGING_MAX_SYMTBL_SZ; i++)
//     {
//       if (proc->mm->symrgtbl[i].rg_start == -1 &&
//           proc->mm->symrgtbl[i].rg_end == -1)
//       {
//         free_id = i;
//         break;
//       }
//     }
//   }

//   if (free_id == -1)
//     return -1;

//   /* By default using vmaid = 0 */
//   if (__alloc(proc, 0, free_id, size, &addr) == 0)
//   {
//     proc->regs[reg_index] = addr;
//   }
// }

// /*libfree - PAGING-based free a region memory
//  *@proc: Process executing the instruction
//  *@size: allocated size
//  *@reg_index: memory region ID (used to identify variable in symbole table)
//  */

// int libfree(struct pcb_t *proc, uint32_t reg_index)
// {
//   /* TODO Implement free region */

//   /* By default using vmaid = 0 */
//   uint32_t region_id = -1;

//   if (reg_index >= PAGING_MAX_SYMTBL_SZ)
//     return -1;

//   if (proc->mm->symrgtbl[reg_index].rg_start == proc->regs[reg_index])
//   {
//     region_id = reg_index;
//   }
//   else
//   {
//     for (size_t i = 0; i < PAGING_MAX_SYMTBL_SZ; i++)
//     {
//       if (proc->mm->symrgtbl[i].rg_start == proc->regs[reg_index] &&
//           proc->mm->symrgtbl[i].rg_end > proc->mm->symrgtbl[i].rg_start)
//       {
//         region_id = i;
//         break;
//       }
//     }
//   }

//   if (region_id == -1)
//     return -1;

//   int result = __free(proc, 0, region_id);

//   if (result == 0)
//   {
//     proc->regs[reg_index] = -1;
//   }
// }
// /*pg_getpage - get the page in ram
//  *@mm: memory region
//  *@pagenum: PGN
//  *@framenum: return FPN
//  *@caller: caller
//  *
//  */
// int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
// {
//   uint32_t pte = mm->pgd[pgn];

//   if (!PAGING_PAGE_PRESENT(pte))
//   { /* Page is not online, make it actively living */
//     int vicpgn, swpfpn;
//     int vicfpn;
//     uint32_t vicpte;

//     int tgtfpn = PAGING_PTE_SWP(pte); // the target frame storing our variable

//     /* TODO: Play with your paging theory here */
//     /* Find victim page */
//     find_victim_page(caller->mm, &vicpgn);
//     vicpte = mm->pgd[vicpgn];
//     vicfpn = PAGING_FPN(vicpte);
//     MEMPHY_get_freefp(caller->active_mswp, &swpfpn);
//     /* Get free frame in MEMSWP */

//     /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

//     /* TODO copy victim frame to swap
//      * SWP(vicfpn <--> swpfpn)
//      * SYSCALL 17 sys_memmap
//      * with operation SYSMEM_SWP_OP
//      */

//     struct sc_regs regs;
//     regs.a1 = SYSMEM_SWP_OP;
//     regs.a2 = vicfpn;
//     regs.a3 = swpfpn;
//     syscall(caller, 17, &regs);
//     /* SYSCALL 17 sys_memmap */

//     /* TODO copy target frame form swap to mem
//      * SWP(tgtfpn <--> vicfpn)
//      * SYSCALL 17 sys_memmap
//      * with operation SYSMEM_SWP_OP
//      */
//     regs.a1 = SYSMEM_SWP_OP; // Source (Swap)
//     regs.a2 = tgtfpn;        // Destination (RAM)
//     regs.a3 = vicfpn;
//     syscall(caller, 17, &regs);
//     /* TODO copy target frame form swap to mem
//     //regs.a1 =...
//     //regs.a2 =...
//     //regs.a3 =..
//     */

//     /* SYSCALL 17 sys_memmap */

//     /* Update page table */

//     // pte_set_fpn() &
//     // mm->pgd[pgn];
//     // pte_set_fpn();
//     CLRBIT(mm->pgd[vicpgn], PAGING_PTE_PRESENT_MASK);

//     // Set the victim page as swapped
//     pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);

//     // Set the target page as present with the victim's frame
//     pte_set_fpn(&mm->pgd[pgn], vicfpn);

//     enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
//   }

//   *fpn = PAGING_FPN(mm->pgd[pgn]);

//   return 0;
// }

// /*pg_getval - read value at given offset
//  *@mm: memory region
//  *@addr: virtual address to acess
//  *@value: value
//  *
//  */
// int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
// {
//   int pgn = PAGING_PGN(addr);
//   int off = PAGING_OFFST(addr);
//   int fpn;

//   /* Get the page to MEMRAM, swap from MEMSWAP if needed */
//   if (pg_getpage(mm, pgn, &fpn, caller) != 0)
//     return -1; /* invalid page access */

//   /* TODO
//    *  MEMPHY_read(caller->mram, phyaddr, data);
//    *  MEMPHY READ
//    *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
//    */
//   int phyaddr = fpn * PAGING_PAGESZ + off;
//   MEMPHY_read(caller->mram, phyaddr, data);
//   struct sc_regs regs;
//   regs.a1 = SYSMEM_IO_READ;
//   regs.a2 = phyaddr;
//   regs.a3 = *data;
//   syscall(caller, 17, &regs);
//   /* SYSCALL 17 sys_memmap */

//   return 0;
// }

// /*pg_setval - write value to given offset
//  *@mm: memory region
//  *@addr: virtual address to acess
//  *@value: value
//  *
//  */
// int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
// {
//   int pgn = PAGING_PGN(addr);
//   int off = PAGING_OFFST(addr);
//   int fpn;

//   /* Get the page to MEMRAM, swap from MEMSWAP if needed */
//   if (pg_getpage(mm, pgn, &fpn, caller) != 0)
//     return -1; /* invalid page access */

//   /* TODO
//    *  MEMPHY_write(caller->mram, phyaddr, value);
//    *  MEMPHY WRITE
//    *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
//    */
//   int phyaddr = fpn * PAGING_PAGESZ + off;
//   MEMPHY_write(caller->mram, phyaddr, value);
//   // int phyaddr
//   struct sc_regs regs;
//   regs.a1 = SYSMEM_IO_WRITE;
//   regs.a2 = phyaddr;
//   regs.a3 = value;
//   syscall(caller, 17, &regs);

//   /* SYSCALL 17 sys_memmap */

//   // Update data

//   return 0;
// }

// /*__read - read value in region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@offset: offset to acess in memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *
//  */
// int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
// {
//   struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
//     return -1;

//   pg_getval(caller->mm, currg->rg_start + offset, data, caller);

//   return 0;
// }

// /*libread - PAGING-based read a region memory */
// int libread(
//     struct pcb_t *proc, // Process executing the instruction
//     uint32_t source,    // Index of source register
//     uint32_t offset,    // Source address = [source] + [offset]
//     uint32_t *destination)
// {
//   BYTE data;
//   int val = __read(proc, 0, source, offset, &data);

//   /* TODO update result of reading action*/
//   if (val == 0)
//   {
//     *destination = (uint32_t)data;
//   }
//   else
//   {
//     *destination = 0;
//   }
//   // destination
// #ifdef IODUMP
//   printf("read region=%d offset=%d value=%d\n", source, offset, data);
// #ifdef PAGETBL_DUMP
//   print_pgtbl(proc, 0, -1); // print max TBL
// #endif
//   MEMPHY_dump(proc->mram);
// #endif

//   return val;
// }

// /*__write - write a region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@offset: offset to acess in memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *
//  */
// int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
// {
//   struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
//     return -1;

//   pg_setval(caller->mm, currg->rg_start + offset, value, caller);

//   return 0;
// }

// /*libwrite - PAGING-based write a region memory */
// int libwrite(
//     struct pcb_t *proc,   // Process executing the instruction
//     BYTE data,            // Data to be wrttien into memory
//     uint32_t destination, // Index of destination register
//     uint32_t offset)
// {
// #ifdef IODUMP
//   printf("write region=%d offset=%d value=%d\n", destination, offset, data);
// #ifdef PAGETBL_DUMP
//   print_pgtbl(proc, 0, -1); // print max TBL
// #endif
//   MEMPHY_dump(proc->mram);
// #endif

//   return __write(proc, 0, destination, offset, data);
// }

// /*free_pcb_memphy - collect all memphy of pcb
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@incpgnum: number of page
//  */
// int free_pcb_memph(struct pcb_t *caller)
// {
//   int pagenum, fpn;
//   uint32_t pte;

//   for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
//   {
//     pte = caller->mm->pgd[pagenum];

//     if (!PAGING_PAGE_PRESENT(pte))
//     {
//       fpn = PAGING_PTE_FPN(pte);
//       MEMPHY_put_freefp(caller->mram, fpn);
//     }
//     else
//     {
//       fpn = PAGING_PTE_SWP(pte);
//       MEMPHY_put_freefp(caller->active_mswp, fpn);
//     }
//   }

//   return 0;
// }

// /*find_victim_page - find victim page
//  *@caller: caller
//  *@pgn: return page number
//  *
//  */
// int find_victim_page(struct mm_struct *mm, int *retpgn)
// {
//   struct pgn_t *pg = mm->fifo_pgn;
//   if (!mm || !mm->fifo_pgn || !retpgn)
//     return -1;
//   *retpgn = pg->pgn;
//   mm->fifo_pgn = pg->pg_next;
//   /* TODO: Implement the theorical mechanism to find the victim page */

//   free(pg);

//   return 0;
// }

// /*get_free_vmrg_area - get a free vm region
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@size: allocated size
//  *
//  */
// int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
// {
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

//   if (rgit == NULL)
//     return -1;

//   /* Probe unintialized newrg */
//   newrg->rg_start = newrg->rg_end = -1;
//   while (rgit)
//   {
//     int rg_size = rgit->rg_end - rgit->rg_start;
//     if (rg_size >= size)
//     {
//       newrg->rg_start = rgit->rg_start;
//       newrg->rg_end = newrg->rg_start + size;

//       /* Shrink the free region */
//       rgit->rg_start += size;

//       return 0;
//     }
//     rgit = rgit->rg_next;
//   }
//   /* TODO Traverse on list of free vm region to find a fit space */
//   // while (...)
//   //  ..

//   return -1;
// }

// // #endif


#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Removed unused mmvm_lock
// static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt) {
    struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

    if (rg_elmt->rg_start >= rg_elmt->rg_end)
        return -1;

    if (rg_node != NULL)
        rg_elmt->rg_next = rg_node;

    mm->mmap->vm_freerg_list = rg_elmt;
    return 0;
}

struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid) {
    if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
        return NULL;
    return &mm->symrgtbl[rgid];
}

int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr) {
    struct vm_rg_struct rgnode;

    if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ) {
        printf("Process %d alloc error: Invalid region\n", caller->pid);
        return -1;
    } else if (caller->mm->symrgtbl[rgid].rg_start > caller->mm->symrgtbl[rgid].rg_end) {
        printf("Process %d alloc error: Region was alloc before\n", caller->pid);
        return -1;
    }

    if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0) {
        caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
        caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
        *alloc_addr = rgnode.rg_start;

        for (int i = rgnode.rg_start / 256; i <= rgnode.rg_end / 256; i++) {
            uint32_t current_pte = caller->mm->pgd[i];
            if (rgnode.rg_end % 256 == 0 && rgnode.rg_end - rgnode.rg_start > 0) {
                break;
            }

            if (!PAGING_PAGE_PRESENT(current_pte)) {
                printf("FREE REG in SWAP \n\n");
                int targetfpn = GETVAL(current_pte, GENMASK(20, 0), 5);
                uint32_t *vicpte = &caller->mm->pgd[find_victim_page(caller->mm, &targetfpn)]; // Fix: Use PTE pointer
                int vicfpn = GETVAL(*vicpte, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
                int swpfpn;

                if (MEMPHY_get_freefp(caller->active_mswp, &swpfpn) < 0) {
                    printf("Out of SWAP");
                    return -3000;
                }

                __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
                pte_set_swap(vicpte, 0, swpfpn);
                __swap_cp_page(caller->active_mswp, targetfpn, caller->mram, vicfpn);
                pte_set_fpn(&caller->mm->pgd[i], vicfpn);
                MEMPHY_put_freefp(caller->active_mswp, targetfpn);
            }
        }
        return 0;
    }

    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    int inc_sz = PAGING_PAGE_ALIGNSZ(size);
    int old_sbrk = cur_vma->sbrk;

    inc_vma_limit(caller, vmaid, inc_sz);

    caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
    caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
    *alloc_addr = old_sbrk;

    struct vm_rg_struct *rgnode_temp = malloc(sizeof(struct vm_rg_struct));
    rgnode_temp->rg_start = old_sbrk + size;
    rgnode_temp->rg_end = cur_vma->sbrk;
    enlist_vm_freerg_list(caller->mm, rgnode_temp);

    return 0;
}

int __free(struct pcb_t *caller, int vmaid, int rgid) {
    struct vm_rg_struct *rgnode;

    if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
        return -1;

    rgnode = get_symrg_byid(caller->mm, rgid);
    if (rgnode->rg_start == rgnode->rg_end) {
        printf("Process %d FREE Error: Region wasn't alloc or was freed before\n", caller->pid);
        return -1;
    }

    struct vm_rg_struct *rgnode_temp = malloc(sizeof(struct vm_rg_struct));
    rgnode_temp->rg_start = rgnode->rg_start;
    rgnode_temp->rg_end = rgnode->rg_end;
    rgnode->rg_start = rgnode->rg_end = 0;

    enlist_vm_freerg_list(caller->mm, rgnode_temp);
    return 0;
}

int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index) {
    int addr;
    return __alloc(proc, 0, reg_index, size, &addr);
}

int libfree(struct pcb_t *proc, uint32_t reg_index) {
    return __free(proc, 0, reg_index);
}

int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller) {
    uint32_t pte = mm->pgd[pgn];

    if (!PAGING_PAGE_PRESENT(pte)) {
        int fpn_temp = -1;
        if (MEMPHY_get_freefp(caller->mram, &fpn_temp) == 0) {
            int tgtfpn = GETVAL(pte, GENMASK(20, 0), 5); // Fix: Declare tgtfpn here
            __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, fpn_temp);
            pte_set_fpn(&mm->pgd[pgn], fpn_temp);
        } else {
            int swpfpn, tgtfpn = GETVAL(pte, GENMASK(20, 0), 5); // Fix: Declare tgtfpn here
            uint32_t *vicpte = &mm->pgd[find_victim_page(mm, fpn)]; // Fix: Use PTE pointer

            int vicfpn = GETVAL(*vicpte, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

            if (MEMPHY_get_freefp(caller->active_mswp, &swpfpn) < 0) {
                printf("Out of SWAP");
                return -3000;
            }

            __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
            __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicfpn);
            pte_set_swap(vicpte, 0, swpfpn);
            pte_set_fpn(&mm->pgd[pgn], vicfpn);

            MEMPHY_put_freefp(caller->active_mswp, tgtfpn);
        }
    }
    *fpn = GETVAL(mm->pgd[pgn], PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    return 0;
}

int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller) {
    int pgn = PAGING_PGN(addr);
    int off = PAGING_OFFST(addr);
    int fpn;

    if (pg_getpage(mm, pgn, &fpn, caller) != 0)
        return -1;

    int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;
    MEMPHY_write(caller->mram, phyaddr, value);
    return 0;
}
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO
   *  MEMPHY_read(caller->mram, phyaddr, data);
   *  MEMPHY READ
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram,phyaddr, data);
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...

  /* SYSCALL 17 sys_memmap */
  // syscall(caller, 17, &regs);
  // Update data
  // *data = (BYTE)regs.a3;

  return 0;
}

int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data) {
    struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

    if (currg == NULL || cur_vma == NULL)
        return -1;

    return pg_getval(caller->mm, currg->rg_start + offset, data, caller); // Fix: Call defined function
}

int libread(struct pcb_t *proc, uint32_t source, uint32_t offset, uint32_t *destination) {
    BYTE data;
    int val = __read(proc, 0, source, offset, &data);
    *destination = (uint32_t)data;
#ifdef IODUMP
    printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
    print_pgtbl(proc, 0, -1);
#endif
    MEMPHY_dump(proc->mram);
#endif
    return val;
}

int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value) {
    struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

    if (currg == NULL || cur_vma == NULL)
        return -1;

    return pg_setval(caller->mm, currg->rg_start + offset, value, caller);
}

int libwrite(struct pcb_t *proc, BYTE data, uint32_t destination, uint32_t offset) {
#ifdef IODUMP
    printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
    print_pgtbl(proc, 0, -1);
#endif
    MEMPHY_dump(proc->mram);
#endif
    return __write(proc, 0, destination, offset, data);
}

int free_pcb_memph(struct pcb_t *caller) {
    int pagenum, fpn;
    uint32_t pte;

    for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++) {
        pte = caller->mm->pgd[pagenum];

        if (!PAGING_PAGE_PRESENT(pte)) {
            fpn = PAGING_PTE_FPN(pte);
            MEMPHY_put_freefp(caller->mram, fpn);
        } else {
            fpn = PAGING_PTE_SWP(pte);
            MEMPHY_put_freefp(caller->active_mswp, fpn);
        }
    }
    return 0;
}

int find_victim_page(struct mm_struct *mm, int *retpgn) {
    struct pgn_t *pg = mm->fifo_pgn;

    // Placeholder: Should implement a proper victim selection mechanism
    if (pg != NULL) {
        *retpgn = pg->pgn;
        mm->fifo_pgn = pg->pg_next;
        free(pg);
        return *retpgn; // Return page number for now
    }
    return 0; // Default case, should be improved
}

int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg) {
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

    if (rgit == NULL)
        return -1;

    newrg->rg_start = newrg->rg_end = -1;

    while (rgit != NULL) {
        if (rgit->rg_start + size <= rgit->rg_end) {
            newrg->rg_start = rgit->rg_start;
            newrg->rg_end = rgit->rg_start + size;

            if (rgit->rg_start + size < rgit->rg_end) {
                rgit->rg_start = rgit->rg_start + size;
            } else {
                struct vm_rg_struct *nextrg = rgit->rg_next;
                if (nextrg != NULL) {
                    rgit->rg_start = nextrg->rg_start;
                    rgit->rg_end = nextrg->rg_end;
                    rgit->rg_next = nextrg->rg_next;
                    free(nextrg);
                } else {
                    rgit->rg_start = rgit->rg_end;
                    rgit->rg_next = NULL;
                }
            }
            break;
        }
        rgit = rgit->rg_next;
    }
    if (newrg->rg_start == -1)
        return -1;

    return 0;
}
