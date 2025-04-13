// From src/sys_xxxhandler.c
#include "common.h"
#include "syscall.h"
#include "stdio.h"

int sys_xxxhandler(struct pcbt *caller, struct sc_regs *reg) {
	/* TODO: implement syscall job */
	printf("The first system call parameter %d\n", reg->a1);
	return 0;
}