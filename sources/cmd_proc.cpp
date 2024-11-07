#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commoner.h"
#include "colors.h"
#include "cmd_proc.h"
#include "processor_info.h"
#include "soft_processor_unit.h"
#include "stack.h"

int PROC_HLT  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    spu->ip = END_PROC;

    return 0;
}
int PROC_OUT  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    int result = 0;
    if (spu->stk.size == 0) {
        return -1;
    }
    if (stack_pop(&spu->stk, &result)) return 1;
    printf("OUT: %d\n", result);
    spu->ip++;

    return 0;
}
int PROC_ADD  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    int a = 0, b = 0;
    if (stack_pop( &spu->stk, &b)) return 1;
    if (stack_pop( &spu->stk, &a)) return 1;
    stack_push(&spu->stk, a + b);
    spu->ip++;

    return 0;
}
int PROC_SUB  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    int a = 0, b = 0;
    if (stack_pop( &spu->stk, &b)) return 1;
    if (stack_pop( &spu->stk, &a)) return 1;
    stack_push(&spu->stk, a - b);
    spu->ip++;

    return 0;
}
int PROC_MUL  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    int a = 0, b = 0;
    if (stack_pop( &spu->stk, &b)) return 1;
    if (stack_pop( &spu->stk, &a)) return 1;
    stack_push(&spu->stk, a * b);
    spu->ip++;

    return 0;
}
int PROC_DIV  (spu_t* spu) {
    spu_assert(spu);

    int a = 0, b = 0;
    if (stack_pop( &spu->stk, &b)) return 1;
    if (stack_pop( &spu->stk, &a)) return 1;
    stack_push(&spu->stk, a / b);
    spu->ip++;

    return 0;
}
int PROC_PUSH (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    int result = 0;
    int arg_type = spu->code[spu->ip++];
    if (arg_type & NUM_T) result += spu->code[spu->ip++];
    if (arg_type & REG_T) result += spu->reg[spu->code[spu->ip++]];
    if (arg_type & RAM_T) result  = spu->ram[result];

    stack_push(&spu->stk, result);

    return 0;
}
int PROC_POP  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    int  arg_type = spu->code[spu->ip++];
    int* place = 0;
    int  result = 0;

    if (stack_pop(&spu->stk, &result)) return 1;

    // if (arg_type & RAM_T)
    if (arg_type & NUM_T) {
        place = &spu->code[spu->ip++];
        if (!(arg_type & RAM_T)) return 1;
    }
    if (arg_type & REG_T) place = spu->reg + spu->code[spu->ip++];
    if (arg_type & RAM_T) place = spu->ram + *place;

    *place = result;

    return 0;
}
int PROC_JMP  (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    spu->ip++;
    spu->ip = spu->code[spu->ip];

    return 0;
}
int PROC_JB   (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    spu->ip++;
    int a = 0, b = 0;
    if (stack_pop( &spu->stk, &b)) return 1;
    if (stack_pop( &spu->stk, &a)) return 1;
    if (a < b) spu->ip = spu->code[spu->ip];
    else       spu->ip++;

    return 0;
}
int PROC_JA   (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    spu->ip++;
    int a = 0, b = 0;
    if (stack_pop( &spu->stk, &b)) return 1;
    if (stack_pop( &spu->stk, &a)) return 1;
    if (a > b) spu->ip = spu->code[spu->ip];
    else       spu->ip++;

    return 0;
}
int PROC_CALL (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    spu->ip++;
    spu->call_heap[spu->call_deep++] = spu->ip + 1;
    spu->ip = spu->code[spu->ip];

    return 0;
}
int PROC_DRAW (spu_t* spu) {
    if (spu_assert(spu)) return 1;

    for(size_t i = 0; i < RAM_SIZE; ++i) {
        printf(spu->ram[i] ? BLUE  "*" : RED ".");
        if (!((i + 1) % RAM_SIDE)) putchar('\n');
    }
    printf(WHITE);

    spu->ip++;

    return 0;
}
int PROC_BREAK(spu_t* spu) {
    if (spu_assert(spu)) return 1;

    if (spu->call_deep == 0) return 1;
    spu->ip = spu->call_heap[--spu->call_deep];

    return 0;
}
int PROC_ERROR(spu_t* spu) {
    if (spu_assert(spu)) return 1;
    LOG_FATAL("NO FUNCTION");
    return 1;
}
