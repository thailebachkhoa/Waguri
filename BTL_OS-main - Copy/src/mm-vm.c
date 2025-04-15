/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*
 * get_vma_by_num - Retrieve the vm area corresponding to a given vmaid.
 * @mm:   Memory management structure.
 * @vmaid: The VM area ID.
 *
 * Returns the pointer to the matching vm_area_struct, or NULL if not found.
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
    struct vm_area_struct *pvma = mm->mmap;
    if (pvma == NULL)
        return NULL;
    // Iterate through the linked list until a matching vm_id is found
    while (pvma != NULL && pvma->vm_id != vmaid)
    {
        pvma = pvma->vm_next;
    }
    return pvma;
}

/*
 * __mm_swap_page - Swap copy a page from a victim frame to a swap frame.
 * @caller:  The caller's process control block.
 * @vicfpn:  Victim frame page number (in MEMRAM).
 * @swpfpn:  Swap frame page number (in the active swap device).
 *
 * Returns 0 on success.
 */
int __mm_swap_page(struct pcb_t *caller, int vicfpn, int swpfpn)
{
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    return 0;
}

/*
 * get_vm_area_node_at_brk - Obtain a new free region node based on the current break pointer.
 * @caller:     Caller process control block.
 * @vmaid:      ID of the VM area from which to allocate.
 * @size:       Requested increase size (in bytes; not directly used in the calculation).
 * @alignedsz:  The aligned size (using PAGING_PAGE_ALIGNSZ) that is a multiple of the page size.
 *
 * Returns a pointer to a newly allocated vm_rg_struct whose boundaries are set based on the current sbrk.
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
    // Retrieve the current VMA for the given vmaid.
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    if (!cur_vma)
        return NULL;
    
    // Allocate a new region node.
    struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
    if (!newrg)
        return NULL;
    
    // Update the new region boundaries based on the current sbrk.
    newrg->rg_start = cur_vma->sbrk;
    newrg->rg_end   = cur_vma->sbrk + alignedsz;
    newrg->rg_next  = NULL;
    
    return newrg;
}

/*
 * validate_overlap_vm_area - Validate that the planned region does not overlap with allocated areas.
 * @caller:    Caller process control block.
 * @vmaid:     ID of the VM area being extended.
 * @vmastart:  Start address of the planned region.
 * @vmaend:    End address of the planned region.
 *
 * This implementation assumes that the VM area contains a free region list (vm_freerg_list)
 * that defines the available areas. We iterate over this list and verify that the new region
 * is entirely contained within one free region.
 *
 * Returns 0 if the region is valid (no overlap), or -1 if an overlap is detected.
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
    struct vm_area_struct *vma = get_vma_by_num(caller->mm, vmaid);
    if (!vma)
        return -1;
    
    // Iterate through the free region list to check if the new region fits.
    struct vm_rg_struct *rg = vma->vm_freerg_list;
    int valid = 0;
    while (rg != NULL)
    {
        if (vmastart >= rg->rg_start && vmaend <= rg->rg_end)
        {
            valid = 1;
            break;
        }
        rg = rg->rg_next;
    }
    return valid ? 0 : -1;
}

/*
 * inc_vma_limit - Increase the VM area's limits to reserve space for new data.
 * @caller:  Caller process control block.
 * @vmaid:   VM area ID to be extended.
 * @inc_sz:  Requested increment size in bytes.
 *
 * This function performs the following steps:
 *   1. Align the requested size to the page boundary.
 *   2. Obtain a new candidate region (free region) based on the current sbrk.
 *   3. Validate that the new region does not overlap already allocated areas.
 *   4. Map the new region into RAM using vm_map_ram().
 *   5. Update the VM area's limit and break pointer (sbrk).
 *
 * Returns 0 on success, or -1 on failure.
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
    // Align the requested size.
    int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
    int incnumpage = inc_amt / PAGING_PAGESZ;
    
    // Get a candidate region from the current sbrk.
    struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
    if (!area)
        return -1;
    
    // Retrieve the current VMA.
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    if (!cur_vma)
    {
        free(area);
        return -1;
    }
    
    int old_end = cur_vma->vm_end;
    
    // Validate that the new region does not overlap any allocated area.
    if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    {
        free(area);
        return -1;  // Overlap detected: allocation fails.
    }
    
    // Allocate a new region structure that will be updated during mapping.
    struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
    if (!newrg)
    {
        free(area);
        return -1;
    }
    
    // Map the new region from physical memory (RAM) into the virtual memory space.
    if (vm_map_ram(caller, area->rg_start, area->rg_end, old_end, incnumpage, newrg) < 0)
    {
        free(area);
        free(newrg);
        return -1;  // Mapping failed.
    }
    
    // On success, update the VM area's end and the sbrk (break pointer).
    cur_vma->vm_end = newrg->rg_end;
    cur_vma->sbrk   = cur_vma->vm_end;
    
    free(area);
    free(newrg);
    return 0;
}


