#ifndef   CMD_PROC_H
#define   CMD_PROC_H

#include "soft_processor_unit.h"
#include "processor_info.h"

int proc_hlt  (spu_t* spu);
int proc_out  (spu_t* spu);
int proc_add  (spu_t* spu);
int proc_sub  (spu_t* spu);
int proc_mul  (spu_t* spu);
int proc_div  (spu_t* spu);
int proc_tdiv (spu_t* spu);
int proc_mod  (spu_t* spu);
int proc_sqrt (spu_t* spu);
int proc_push (spu_t* spu);
int proc_pop  (spu_t* spu);
int proc_jmp  (spu_t* spu);
int proc_jb   (spu_t* spu);
int proc_ja   (spu_t* spu);
int proc_je   (spu_t* spu);
int proc_jne  (spu_t* spu);
int proc_call (spu_t* spu);
int proc_draw (spu_t* spu);
int proc_break(spu_t* spu);
int proc_error(spu_t* spu);

#endif // CMD_PROC_H
