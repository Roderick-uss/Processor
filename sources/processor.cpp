#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_PROCESSOR

#include "commoner.h"
#include "processor_info.h"
#include "cmd_proc.h"
#include "soft_processor_unit.h"
#include "stack.h"

#define BASE_INPUT_FILE  "text_file/processed.bin"
#define BASE_OUTPUT_FILE "text_file/output.txt"

struct label_t {
    char* name;
    size_t ip;
};

typedef short elem_t;

static int run_proc(spu_t* spu);

static size_t get_size(char* code, size_t* size);
// static int verify_code_version(void** code);
static int copy_code(spu_t* spu, size_t size, const char* code);

int main(int argc, const char* argv[]) {
    const char* input_file = BASE_INPUT_FILE;
    const char* output_file = argc > 1 ? argv[1] : BASE_OUTPUT_FILE;

    FILE* output_ptr = fopen(output_file, "w");

    if (!output_file) {run_file_error(output_file); return 1;}

    spu_t* spu = spu_ctor();
    if (!spu) {
        LOG_FATAL("SPU DID NOT CREATED");
        return 1;
    }
    // return 1;
    char* code_buffer = (char*)get_file_buffer(input_file);
    if (!code_buffer) {
        LOG_FATAL("NO CODE SCANNED\n");
        return 1;
    }
    size_t buffer_shift = 0;
    size_t code_size = 0;
    buffer_shift = get_size(code_buffer, &code_size);
    copy_code(spu, code_size, code_buffer + buffer_shift);

    run_proc(spu);

    spu_dtor(spu);
    free(code_buffer);
    fclose(output_ptr);
    return 0;
}

static size_t get_size(char* code, size_t* size) {
    assert(code);
    *size = *(size_t*)code;
    return sizeof(size_t);
}

static int copy_code(spu_t* spu, size_t size, const char* code) {
    ASSERT_SPU(spu);
    assert(code);

    spu->code_size = size;
    for(size_t i = 0; i < size; i++) spu->code[i] = ((elem_t*)code)[i];

    return 0;
}

static int run_proc(spu_t* spu) {
    ASSERT_SPU(spu);
    assert(spu->ip == 0);

    int error = 0;
    int to_scan = 0;

    const COMMAND* check_cmd = {};

    while (!error && spu->ip != END_PROC) {
        // spu_print(spu);
        // scanf("%d", &to_scan);

        for(size_t index_cmd = 0; index_cmd < sizeof(CMD_LIST) / sizeof(CMD_LIST[0]); ++index_cmd) {
            check_cmd = CMD_LIST + index_cmd;

            if (check_cmd->id == UNDEF) {
                error = 1;
                LOG_FATAL("UNDEFINED COMMAND\n");
                break;
            }
            if ((spu->code[spu->ip] & cmd_mask) == check_cmd->id) {
                error = check_cmd->process_cmd(spu);
                break;
            }
        }
    }

    if (error) {
        LOG_FATAL("FINISHED WITH ERRORS\n");
        return 1;
    }
    else {
        LOG_INFO("PROCESSOR COMPLITE\n");
    }

    return 0;
}
