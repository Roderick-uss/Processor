#ifndef PROCESSOR_H
#define PROCESSOR_H

#ifdef    START_PROCESSOR
// #include "soft_processor_unit.h"
#include "cmd_proc.h"
typedef int (*proc_func_t)(spu_t* spu);
#endif // START_PROCESSOR

// NO NEGATIVE NUMBERS
enum COMMANDS_T {
    HLT   = 1 ,
    OUT   = 2 ,
    ADD   = 3 ,
    SUB   = 4 ,
    MUL   = 5 ,
    DIV   = 6 ,
    PUSH  = 7 ,
    POP   = 8 ,
    JMP   = 9 ,
    JB    = 10 ,
    JA    = 11,
    CALL  = 12,
    DRAW  = 13,
    BREAK = 14,
    UNDEF = -1,
};


const int MAX_LABELS    =  50;
const int RAM_SIZE      =  49;
const int REGS_SIZE     =   5;
const int MAX_CODE_SIZE = 500;
const int MAX_DEEP      =  50;
const int RAM_SIDE      =   7;

const size_t END_PROC = MAX_CODE_SIZE + 1;

const int cmd_mask = (1<<8) - 1;

// const int arg_mask = cmd_mask << 8;

enum ARGUMENT_TYPE {
    NO_ARG    = 0,
    NUM_ARG   = 1,
    LABEL_ARG = 2,
    POP_ARG   = 3,
    PUSH_ARG  = 4,
};

static const int arg_shift = 8;
enum ARG_MASK_T {
    ERR_T = 64<<arg_shift,
    NUM_T =  1<<arg_shift,
    REG_T =  2<<arg_shift,
    RAM_T =  4<<arg_shift,
};

struct COMMAND {
    const char* name;
    int n_args;
    COMMANDS_T id;
    ARGUMENT_TYPE agr_type;

    #ifdef    START_PROCESSOR
    proc_func_t process_cmd;
    #endif // START_PROCESSOR
};

#ifdef    START_PROCESSOR
const COMMAND CMD_LIST[] ={
{"HLT"  , 0, HLT  , NO_ARG   , PROC_HLT  },
{"OUT"  , 0, OUT  , NO_ARG   , PROC_OUT  },
{"ADD"  , 0, ADD  , NO_ARG   , PROC_ADD  },
{"SUB"  , 0, SUB  , NO_ARG   , PROC_SUB  },
{"MUL"  , 0, MUL  , NO_ARG   , PROC_MUL  },
{"DIV"  , 0, DIV  , NO_ARG   , PROC_DIV  },
{"PUSH" , 1, PUSH , PUSH_ARG , PROC_PUSH },
{"POP"  , 1, POP  , POP_ARG  , PROC_POP  },
{"JMP"  , 1, JMP  , LABEL_ARG, PROC_JMP  },
{"JA"   , 1, JA   , LABEL_ARG, PROC_JA   },
{"JB"   , 1, JB   , LABEL_ARG, PROC_JB   },
{"CALL" , 1, CALL , LABEL_ARG, PROC_CALL },
{"DRAW" , 0, DRAW , NO_ARG   , PROC_DRAW },
{"BREAK", 0, BREAK, NO_ARG   , PROC_BREAK},
{""     , 0, UNDEF, NO_ARG   , PROC_ERROR}}; // uind must be last element

#else //  START_PROCESSOR

const COMMAND CMD_LIST[] ={
{"HLT"  , 0, HLT  , NO_ARG   },
{"OUT"  , 0, OUT  , NO_ARG   },
{"ADD"  , 0, ADD  , NO_ARG   },
{"SUB"  , 0, SUB  , NO_ARG   },
{"MUL"  , 0, MUL  , NO_ARG   },
{"DIV"  , 0, DIV  , NO_ARG   },
{"PUSH" , 1, PUSH , PUSH_ARG },
{"POP"  , 1, POP  , POP_ARG  },
{"JMP"  , 1, JMP  , LABEL_ARG},
{"JA"   , 1, JA   , LABEL_ARG},
{"JB"   , 1, JB   , LABEL_ARG},
{"CALL" , 1, CALL , LABEL_ARG},
{"DRAW" , 0, DRAW , NO_ARG   },
{"BREAK", 0, BREAK, NO_ARG   },
{""     , 0, UNDEF, NO_ARG   }}; // uind must be last element

#endif // START_PROCESSOR

#endif // PROCESSOR_H
