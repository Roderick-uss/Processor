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

static int run_proc(spu_t* spu);

static size_t get_size(void* code);
// static int verify_code_version(void** code);
static int copy_code(spu_t* spu, size_t size, const short* code);

int main(int argc, const char* argv[]) {
    const char* input_file = BASE_INPUT_FILE;
    const char* output_file = argc > 1 ? argv[1] : BASE_OUTPUT_FILE;

    FILE* output_ptr = fopen(output_file, "w");

    if (!output_file) {run_file_error(output_file); return 1;}

    spu_t* spu = spu_ctor();
    // return 1;

    short* code_buffer = (short*)get_file_buffer(input_file);
    if (!code_buffer) {
        LOG_FATAL("NO CODE GOT\n");
        return 0;
    }
    size_t code_size = get_size(&code_buffer);
    copy_code(spu, code_size, code_buffer);

    run_proc(spu);

    spu_dtor(spu);
    free(code_buffer);
    fclose(output_ptr);
    return 0;
}

static size_t get_size(void* code) {
    char** code_ = (char**)code;
    assert(code_);
    assert(*code_);

    size_t size = *(size_t*)*code_;
    *code_ += sizeof(size);

    return size;
}

static int copy_code(spu_t* spu, size_t size, const short* code) {
    assert(spu);
    assert(size <= MAX_CODE_SIZE);
    assert(code);

    spu->code_size = size;
    for(size_t i = 0; i < size; ++i) {
        spu->code[i] = (int)code[i];
    }

    return 0;
}

static int run_proc(spu_t* spu) {
    spu_assert(spu);
    assert(spu->ip == 0);

    int error = 0;
    int to_scan = 0;

    const COMMAND* check_cmd = {};

    while (!error && spu->ip != END_PROC) {
        spu_print(spu);
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
