#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "commoner.h"
#include "processor_info.h"
#include "soft_processor_unit.h"
#include "stack.h"

static int spu_dump(const spu_t* spu, int error_mask) {
    if (error_mask == 0) return 0;

    LOG_YELLOW("____________________________________DUMP____________________________________\n\n");

    if (error_mask & ZERO_SPU) {
        LOG_FATAL("LOST PTR TO SPU\n");
        LOG_YELLOW("____________________________________________________________________________\n");
        return error_mask;
    }

    assert(spu);

    if (error_mask & ZERO_CODE) {
        LOG_FATAL("LOST PTR TO CODE\n");
    }
    else {
        LOG_CYAN("[ip]   : ");
        for(size_t i = 0; i < spu->code_size; ++i) {
            if (i != spu->ip) LOG_WHITE(          "%-4.4llx "           , i);
            else              LOG_RED  (UNDERLINE "%-4.4llx" DEFAULT " ", i);
        }
        putc('\n', stderr);

        LOG_CYAN("[code] : ");
        for(size_t i = 0; i < spu->code_size; ++i) {
            LOG_INFO("%-4.4d ", spu->code[i] / PRECISION);
        }
        putc('\n', stderr);

        char* spases = (char*)calloc(5 * spu->ip + 10, sizeof(char));
        memset(spases, ' ', spu->ip * 5 + 9);
        LOG_RED("%s^ ",spases);

        for(size_t index_cmd = 0; index_cmd < sizeof(CMD_LIST) / sizeof(CMD_LIST[0]); ++index_cmd) {
            if (CMD_LIST[index_cmd].id == UNDEF) {
                LOG_BLUE("%s\n", CMD_LIST[index_cmd].name);
                break;
            }
            if ((spu->code[spu->ip] & cmd_mask) == CMD_LIST[index_cmd].id) {
                LOG_BLUE("%s\n", CMD_LIST[index_cmd].name);
                break;
            }
        }

        memset(spases, '_', spu->ip * 5 + 9);
        LOG_RED("%s|", spases);
        LOG_RED("(ip = %llx)\n\n", spu->ip);

        free(spases);

        LOG_CYAN("code size = %llx\n\n", spu->code_size);

    }
    if (error_mask & STACK_PROBLEM) {
        LOG_FATAL("STACK BROKEN\n");
        stack_assert(spu->stk);
    }
    else {
        LOG_CYAN("stk size = %llx\n", spu->stk->size);
        LOG_CYAN("[stk]  : ");
        for(size_t i = 0; i < spu->stk->size; ++i) {
            LOG_RED("%d ", spu->stk->data[i] / PRECISION);
        }
        LOG_BLUE("<-- last\n");
    }
    if (error_mask & ZERO_REGS) {
        LOG_FATAL("LOST PTR TO REGISTERS\n");
    }
    else {
        for(int i = 0; i < REGS_SIZE; ++i) {
            LOG_PURPLE("%cx = %d\n", 'a' + i, spu->reg[i] / PRECISION);
        }
        putchar('\n');
    }
    if (error_mask & ZERO_CALL) {
        LOG_FATAL("LOST PTR TO CALL_HEAP \n");
    } else {
        LOG_CYAN("call deep = %llx\n", spu->call_deep);
        if (spu->call_deep > 0) {
            LOG_CYAN("[call ip] : ");
            for(size_t i = 0; i < spu->call_deep; ++i) {
                 LOG_BLUE("%2.2llx ", spu->call_heap[i]);
            }
            putchar('\n');
        }
    }
    LOG_YELLOW("____________________________________________________________________________\n\n\n");

    return error_mask & ~PRINT_SPU;
}

int spu_verify(const spu_t* spu) {
    assert(spu);
    int error_mask = 0;
    if (!spu)                     error_mask |= ZERO_SPU;
    if (!spu->code)               error_mask |= ZERO_CODE;
    if (!spu->ram)                error_mask |= ZERO_RAM;
    if (!spu->reg)                error_mask |= ZERO_REGS;
    if (!spu->call_heap)          error_mask |= ZERO_CALL;
    if (!stack_assert(spu->stk)) error_mask |= STACK_PROBLEM;

    return error_mask;
}

int spu_print(const spu_t* spu) {
    assert(!spu_dump(spu, spu_verify(spu) | PRINT_SPU));
    return 0;
}

int spu_assert(const spu_t* spu) {
    return !spu_dump(spu, spu_verify(spu));
}

spu_t* spu_ctor() {
    spu_t* spu = (spu_t*)calloc(1, sizeof(spu_t));
    if (!spu) return 0;

    if (!(spu->ram       = (int*)    calloc(RAM_SIZE     , sizeof(int   )))) return 0;
    if (!(spu->reg       = (int*)    calloc(REGS_SIZE    , sizeof(int   )))) return 0;
    if (!(spu->code      = (int*)    calloc(MAX_CODE_SIZE, sizeof(int   )))) return 0;
    if (!(spu->call_heap = (size_t*) calloc(MAX_DEEP     , sizeof(size_t)))) return 0;

    if (!(spu->stk = (stack_t*)calloc(1, sizeof(stack_t)))) return 0;;
    stack_ctor(spu->stk);

    ASSERT_SPU(spu);
    return spu;
}

int spu_dtor(spu_t* spu) {
    ASSERT_SPU(spu);

    stack_dtor(spu->stk);
    free(spu->stk);

    free(spu->call_heap);
    free(spu->ram      );
    free(spu->code     );
    free(spu->reg      );

    *spu = {};

    free(spu);

    return 0;
}

#undef ASSERT_SPU
