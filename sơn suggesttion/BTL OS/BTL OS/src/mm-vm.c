// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>



struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
{
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    return 0;
}


struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct * newrg;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);  

  newrg = malloc(sizeof(struct vm_rg_struct));
  
  newrg->rg_start = cur_vma->sbrk; 
  newrg->rg_end = newrg->rg_start + size;



  return newrg;
}


int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{

  struct vm_area_struct *vma = caller->mm->mmap;

  while (!vma)
  {
    if((vma->vm_id != vmaid) && (vma->vm_start != vma->vm_end))
    {
      if((int)((vmaend - 1 - vma->vm_start) * (vma->vm_end - 1 - vmastart))>=0)
      return -1;
      if((int)((vmastart - vma->vm_start) * (vma->vm_end - vmaend)) >= 0)
      return -1;
    }
    vma = vma->vm_next;
  }
  return 0;
}


int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage =  inc_amt / PAGING_PAGESZ;
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  int old_end = cur_vma->vm_end;

  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    return -1; 


  cur_vma->vm_end += inc_sz;
  cur_vma->sbrk += inc_sz;

  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                    old_end, incnumpage , newrg) < 0)
    return -1; 

  return 0;
}

// #endif
