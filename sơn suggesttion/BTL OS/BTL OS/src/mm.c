// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

// TODO: struct pgn_t* global_fifo = NULL;
int init_pte(uint32_t *pte,
             int pre,    
             int fpn,    
             int drt,    
             int swp,    
             int swptyp, 
             int swpoff) 
{
  if (pre != 0)
  {
    if (swp == 0)
    { 
      if (fpn == 0)
        return -1; 


      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    }
    else
    { 
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
      SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
    }
  }

  return 0;
}


int pte_set_swap(uint32_t *pte, int swptyp, int swpoff)
{
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

  return 0;
}


int pte_set_fpn(uint32_t *pte, int fpn)
{
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

  return 0;
}


int vmap_page_range(struct pcb_t *caller,           
                    int addr,                       
                    int pgnum,                      
                    struct framephy_struct *frames, 
                    struct vm_rg_struct *ret_rg)    
{                                                   

  struct framephy_struct *fpit = frames;

  int pgit = 0;
  int pgn = PAGING_PGN(addr);


  ret_rg->rg_start = addr;
  ret_rg->rg_end = addr + pgnum * PAGING_PAGESZ - 1;
  ret_rg->rg_next = NULL;


  for (pgit = 0; pgit < pgnum && fpit != NULL; pgit++, fpit = fpit->fp_next)
  {
    int cur_pgn = pgn + pgit;
    uint32_t *pte = &caller->mm->pgd[cur_pgn];
    pte_set_fpn(pte, fpit->fpn);

    enlist_pgn_node(&caller->mm->fifo_pgn, cur_pgn);
  }

  return 0;
}

int alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
  int pgit, fpn;
  struct framephy_struct *newfp_str = NULL;



  for (pgit = 0; pgit < req_pgnum; pgit++)
  {

    if (MEMPHY_get_freefp(caller->mram, &fpn) == 0)
    {
      newfp_str = (struct framephy_struct *)malloc(sizeof(struct framephy_struct));
      newfp_str->fpn = fpn;
      newfp_str->owner = caller->mm;
      if (!*frm_lst)
        *frm_lst = newfp_str;
      else
      {
        newfp_str->fp_next = *frm_lst;
        *frm_lst = newfp_str;
      }
      newfp_str->fp_next = caller->mram->used_fp_list;
      caller->mram->used_fp_list = newfp_str;
    }

    else
    {
      int victim_fpn, victim_pgn, victim_pte;
      int swpfpn = -1;
      if (find_victim_page(caller->mm, &victim_pgn) < 0)
        return -1;
      victim_pte = caller->mm->pgd[victim_pgn];
      victim_fpn = PAGING_FPN(victim_pte);
      newfp_str = (struct framephy_struct *)malloc(sizeof(struct framephy_struct));
      newfp_str->fpn = victim_fpn;
      newfp_str->owner = caller->mm;
      if (!*frm_lst)
        *frm_lst = newfp_str;
      else
      {
        newfp_str->fp_next = *frm_lst;
        *frm_lst = newfp_str;
      }

      int i = 0;
      if (MEMPHY_get_freefp(caller->active_mswp, &swpfpn) == 0)
      {
        __swap_cp_page(caller->mram, victim_fpn, caller->active_mswp, swpfpn);
        struct memphy_struct *mswp = (struct memphy_struct *)caller->mswp;
        for (i = 0; i < PAGING_MAX_MMSWP; i++)
        {
          if (mswp + i == caller->active_mswp)
            break;
        }
      }
      else
      {
        struct memphy_struct *mswp = (struct memphy_struct *)caller->mswp;
        for (i = 0; i < PAGING_MAX_MMSWP; i++)
        {
          if (MEMPHY_get_freefp(mswp + i, &swpfpn) == 0)
          {
            __swap_cp_page(caller->mram, victim_fpn, mswp + i, swpfpn);
            break;
          }
        }
      }
      if (swpfpn == -1)
        return -3000;
      pte_set_swap(&caller->mm->pgd[victim_pgn], i, swpfpn);
    }
  }
  return 0;
}


int vm_map_ram(struct pcb_t *caller, int astart, int aend, int mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  struct framephy_struct *frm_lst = NULL;
  int ret_alloc;


  ret_alloc = alloc_pages_range(caller, incpgnum, &frm_lst);

  if (ret_alloc < 0 && ret_alloc != -3000)
    return -1;


  if (ret_alloc == -3000)
  {
#ifdef MMDBG
    printf("OOM: vm_map_ram out of memory \n");
#endif
    return -1;
  }


  vmap_page_range(caller, mapstart, incpgnum, frm_lst, ret_rg);

  return 0;
}


int __swap_cp_page(struct memphy_struct *mpsrc, int srcfpn,
                   struct memphy_struct *mpdst, int dstfpn)
{
  int cellidx;
  int addrsrc, addrdst;
  for (cellidx = 0; cellidx < PAGING_PAGESZ; cellidx++)
  {
    addrsrc = srcfpn * PAGING_PAGESZ + cellidx;
    addrdst = dstfpn * PAGING_PAGESZ + cellidx;

    BYTE data;
    MEMPHY_read(mpsrc, addrsrc, &data);
    MEMPHY_write(mpdst, addrdst, data);
  }

  return 0;
}

/*
 *Initialize a empty Memory Management instance
 * @mm:     self mm
 * @caller: mm owner
 */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));

  mm->pgd = malloc(PAGING_MAX_PGN * sizeof(uint32_t));

  /* By default the owner comes with at least one vma */
  vma0->vm_id = 0;
  vma0->vm_start = 0;
  vma0->vm_end = vma0->vm_start;
  vma0->sbrk = vma0->vm_start;
  struct vm_rg_struct *first_rg = init_vm_rg(vma0->vm_start, vma0->vm_end);
  enlist_vm_rg_node(&vma0->vm_freerg_list, first_rg);


  vma0->vm_next = NULL;
  vma0->vm_mm = mm;


  mm->mmap = vma0;

  return 0;
}

struct vm_rg_struct *init_vm_rg(int rg_start, int rg_end)
{
  struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));

  rgnode->rg_start = rg_start;
  rgnode->rg_end = rg_end;
  rgnode->rg_next = NULL;

  return rgnode;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
  rgnode->rg_next = *rglist;
  *rglist = rgnode;

  return 0;
}

int enlist_pgn_node(struct pgn_t **plist, int pgn)
{
  struct pgn_t *pnode = malloc(sizeof(struct pgn_t));

  pnode->pgn = pgn;
  pnode->pg_next = *plist;
  *plist = pnode;

  return 0;
}

int print_list_fp(struct framephy_struct *ifp)
{
  struct framephy_struct *fp = ifp;

  printf("print_list_fp: ");
  if (fp == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (fp != NULL)
  {
    printf("fp[%d]\n", fp->fpn);
    fp = fp->fp_next;
  }
  printf("\n");
  return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
  struct vm_rg_struct *rg = irg;

  printf("print_list_rg: ");
  if (rg == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (rg != NULL)
  {
    printf("rg[%ld->%ld]\n", rg->rg_start, rg->rg_end);
    rg = rg->rg_next;
  }
  printf("\n");
  return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
  struct vm_area_struct *vma = ivma;

  printf("print_list_vma: ");
  if (vma == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (vma != NULL)
  {
    printf("va[%ld->%ld]\n", vma->vm_start, vma->vm_end);
    vma = vma->vm_next;
  }
  printf("\n");
  return 0;
}

int print_list_pgn(struct pgn_t *ip)
{
  printf("print_list_pgn: ");
  if (ip == NULL)
  {
    printf("NULL list\n");
    return -1;
  }
  printf("\n");
  while (ip != NULL)
  {
    printf("va[%d]-\n", ip->pgn);
    ip = ip->pg_next;
  }
  printf("n");
  return 0;
}

int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end)
{
  int pgn_start, pgn_end;
  int pgit;

  if (end == -1)
  {
    pgn_start = 0;
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, 0);
    end = cur_vma->vm_end;
  }
  pgn_start = PAGING_PGN(start);
  pgn_end = PAGING_PGN(end);

  printf("print_pgtbl: %d - %d", start, end);
  if (caller == NULL)
  {
    printf("NULL caller\n");
    return -1;
  }
  printf("\n");

  for (pgit = pgn_start; pgit < pgn_end; pgit++)
  {
    printf("%08ld: %08x\n", pgit * sizeof(uint32_t), caller->mm->pgd[pgit]);
  }

  return 0;
}

// #endif
