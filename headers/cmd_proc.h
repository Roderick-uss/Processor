#ifndef   CMD_PROC_H
#define   CMD_PROC_H

#include "soft_processor_unit.h"
#include "processor_info.h"

int PROC_HLT  (spu_t* spu);
int PROC_OUT  (spu_t* spu);
int PROC_ADD  (spu_t* spu);
int PROC_SUB  (spu_t* spu);
int PROC_MUL  (spu_t* spu);
int PROC_DIV  (spu_t* spu);
int PROC_TDIV (spu_t* spu);
int PROC_MOD  (spu_t* spu);
int PROC_SQRT (spu_t* spu);
int PROC_PUSH (spu_t* spu);
int PROC_POP  (spu_t* spu);
int PROC_JMP  (spu_t* spu);
int PROC_JB   (spu_t* spu);
int PROC_JA   (spu_t* spu);
int PROC_JE   (spu_t* spu);
int PROC_JNE  (spu_t* spu);
int PROC_CALL (spu_t* spu);
int PROC_DRAW (spu_t* spu);
int PROC_BREAK(spu_t* spu);
int PROC_ERROR(spu_t* spu);

#endif // CMD_PROC_H
