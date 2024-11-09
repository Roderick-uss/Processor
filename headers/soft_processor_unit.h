#ifndef SPU_H
#define SPU_H

#include "stack.h"

struct spu_t {
    size_t   ip;
    size_t   code_size;
    int*     code;
    int*     ram;
    int*     reg;
    stack_t* stk;
    size_t*  call_heap;
    size_t   call_deep;
};

enum SPU_ERRORS {
    PRINT_SPU     = 1<<0,
    ZERO_RAM      = 1<<1,
    ZERO_CODE     = 1<<2,
    ZERO_REGS     = 1<<3,
    ZERO_SPU      = 1<<4,
    ZERO_CALL     = 1<<5,
    STACK_PROBLEM = 1<<6,
};

#define ASSERT_SPU(spu) assert(spu_assert(spu))

spu_t* spu_ctor();
int    spu_dtor(spu_t* spu);

int spu_assert(const spu_t* spu);
int spu_print (const spu_t* spu);
int spu_verify(const spu_t* spu);

#endif // SPU_H
