/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// Thư viện hệ thống
#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Khóa để đồng bộ thao tác vùng nhớ ảo (virtual memory)
static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Hàm thêm một vùng nhớ tự do vào danh sách vùng nhớ trống
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/*
 * Trả về con trỏ đến vùng nhớ ứng với ID biến (dựa vào bảng symrgtbl)
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
  {
    return NULL;
  }

  return &mm->symrgtbl[rgid];
}

/*
 * Hàm cấp phát vùng nhớ cho một biến (tương ứng với reg_index) từ vm_area xác định
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  struct vm_rg_struct rgnode;

  pthread_mutex_lock(&mmvm_lock);

  // Thử cấp phát từ các vùng nhớ trống
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  // Nếu không còn vùng trống, cần mở rộng heap bằng syscall
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int old_sbrk = cur_vma->sbrk;

  struct sc_regs regs;
  regs.a1 = SYSMEM_INC_OP;
  regs.a2 = vmaid;
  regs.a3 = inc_sz;
  regs.orig_ax = 17;

  if (syscall(caller, regs.orig_ax, &regs) != 0)
  {
    regs.flags = -1;
    perror("failed to increase limit");
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  regs.flags = 0;

  // Cập nhật bảng vùng nhớ
  caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
  caller->mm->symrgtbl[rgid].rg_end = old_sbrk + inc_sz;
  *alloc_addr = old_sbrk;

  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/*
 * Hàm thu hồi vùng nhớ đã cấp phát cho một biến
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  struct vm_rg_struct rgnode;

  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
  {
    return -1;
  }

  // Lấy thông tin vùng nhớ đã cấp phát
  rgnode = *get_symrg_byid(caller->mm, rgid);

  // Đưa vùng nhớ đã thu hồi vào danh sách freerg_list
  enlist_vm_freerg_list(caller->mm, &rgnode);

  return 0;
}

/*
 * Hàm cấp phát bộ nhớ tại vm_area 0 (vùng mặc định)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;
  int val = __alloc(proc, 0, reg_index, size, &addr);

#ifdef IODUMP
  printf("===== PHYSICAL MEMORY AFTER ALLOCATION =====\n");
  printf("PID=%d - Region=%d - Address=%08ld - Size=%d byte\n", proc->pid, reg_index, addr * sizeof(uint32_t), size);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1);
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*
 * Hàm giải phóng bộ nhớ tại vm_area 0
 */
int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  int val = __free(proc, 0, reg_index);

#ifdef IODUMP
  printf("===== PHYSICAL MEMORY AFTER DEALLOCATION =====\n");
  printf("PID=%d - Region=%d\n", proc->pid, reg_index);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1);
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*
 * Kiểm tra page đã có mặt trong RAM chưa. Nếu chưa thì swap từ MEMSWP vào.
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  {
    int vicpgn, swpfpn, vicfpn;
    uint32_t vicpte;

    int tgtfpn = PAGING_PTE_SWP(pte);

    find_victim_page(caller->mm, &vicpgn);
    MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

    vicpte = mm->pgd[vicpgn];
    vicfpn = PAGING_PTE_FPN(vicpte);

    struct sc_regs regs;
    regs.a1 = SYSMEM_SWP_OP;
    regs.a2 = vicfpn;
    regs.a3 = swpfpn;
    regs.orig_ax = 17;

    if (syscall(caller, regs.orig_ax, &regs) != 0)
    {
      regs.flags = -1;
      return -1;
    }

    regs.a2 = tgtfpn;
    regs.a3 = vicfpn;

    if (syscall(caller, regs.orig_ax, &regs) != 0)
    {
      regs.flags = -1;
      return -1;
    }

    regs.flags = 0;

    pte_set_swap(&vicpte, caller->active_mswp_id, swpfpn);
    mm->pgd[vicpgn] = vicpte;

    pte_set_fpn(&pte, vicfpn);
    PAGING_PTE_SET_PRESENT(pte);
    mm->pgd[pgn] = pte;

    enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
  }

  *fpn = PAGING_FPN(pte);

  return 0;
}

/*
 * Đọc giá trị tại địa chỉ ảo, sau khi đảm bảo page đã nằm trong RAM
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int offst = PAGING_OFFST(addr);
  int fpn;

  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
  {
    return -1;
  }

  int phyaddr = PAGING_PHYADDR(fpn, offst);

  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_READ;
  regs.a2 = phyaddr;
  regs.orig_ax = 17;

  if (syscall(caller, regs.orig_ax, &regs) != 0)
  {
    regs.flags = -1;
    return -1;
  }

  *data = (BYTE)(regs.a3);
  regs.flags = 0;

  return 0;
}

/*
 * Ghi giá trị vào địa chỉ ảo
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int offst = PAGING_OFFST(addr);
  int fpn;

  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
  {
    return -1;
  }

  int phyaddr = PAGING_PHYADDR(fpn, offst);

  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_WRITE;
  regs.a2 = phyaddr;
  regs.a3 = value;
  regs.orig_ax = 17;

  if (syscall(caller, regs.orig_ax, &regs) != 0)
  {
    regs.flags = -1;
    return -1;
  }

  regs.flags = 0;

  return 0;
}

/*
 * Đọc giá trị tại vùng nhớ cụ thể theo offset
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL)
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*
 * Đọc giá trị tại vùng nhớ ảo được xác định bởi index và offset
 */
int libread(struct pcb_t *proc, uint32_t source, uint32_t offset, uint32_t *destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  if (val == 0)
  {
    *destination = (uint32_t)data;
  }

#ifdef IODUMP
  printf("================================================================\n");
  printf("===== PHYSICAL MEMORY AFTER READING =====\n");
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1);
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*
 * Ghi giá trị vào vùng nhớ theo offset trong region
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL)
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*
 * Ghi dữ liệu vào vùng nhớ ảo qua chỉ số và offset
 */
int libwrite(struct pcb_t *proc, BYTE data, uint32_t destination, uint32_t offset)
{
#ifdef IODUMP
  printf("================================================================\n");
  printf("===== PHYSICAL MEMORY AFTER WRITING =====\n");
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1);
#endif
  MEMPHY_dump(proc->mram);
#endif

  return __write(proc, 0, destination, offset, data);
}
