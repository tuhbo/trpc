#ifndef TRPC_COROUTINE_COCTX_H
#define TRPC_COROUTINE_COCTX_H

namespace trpc {

enum {
  kRBP = 6, // rbp, bottom of stack
  kRDI = 7, // rdi, first para when all function
  kRSI = 8, // rsi, second param when all function
  kRETAddr = 9, // the next excute cmd address, it will be assigned to rip
  kRSP = 13, // rsp, top of stack
};

struct coctx {
  void *regs[14];
/*
  regs[0] ----> r15
  regs[1] ----> r14
  regs[2] ----> r13
  regs[3] ----> r12
  regs[4] ----> r9
  regs[5] ----> r8
  regs[6] ----> rbp
  regs[7] ----> rdi
  regs[8] ----> rsi
  regs[9] ----> rax
  regs[10] ----> rdx
  regs[11] ----> rcx
  regs[12] ----> rbx
  regs[13] ----> rsp
*/
};

extern "C" {

extern void coctx_swap(coctx *, coctx *) asm("coctx_swap");
};

}
#endif