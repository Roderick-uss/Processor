#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "commoner.h"
#include "processor_info.h"
#include "trie.h"

static const char * const BASE_INPUT_FILE  = "text_file/input.txt";
static const char * const BASE_OUTPUT_FILE = "text_file/processed.bin";

static const char REGISTER_IDENTIFICATOR = 'x';

static const int MAX_ARGUMENT_SIZE = 20;
static const int MAX_COMMAND_SIZE  = 20;
static const int MAX_LINE_SIZE     = 50;

struct asm_label_unit {
    char*  name;
    size_t ip;
};

struct asm_label_info {
    asm_label_unit* data;
    size_t          size;
    trie_t*         trie;
};

struct possible_argument_t {
    short value;
    ARG_MASK_T type;
};

struct current_code_state {
    const char* line;
    size_t* pos;
    size_t line_num;
    short** code;
};


int compile_labels (const char* buffer,       asm_label_info* labels);
int compile_code   (const char* buffer, const asm_label_info* labels, short* code, size_t* code_size);

static int compile_num_arg     (const char* line, size_t* pos, size_t line_num, short** code, int n_args);
static int compile_complex_arg (const char* line, size_t* pos, size_t line_num, short** code);
static int compile_label_arg   (const char* line, size_t* pos, size_t line_num, short** code, const asm_label_info* labels);
//todo struct current code state
// static int compile_label   (const char* line, size_t* scanned, asm_label_info* labels);

static int run_syntax_error       (const char* line, const char* message, size_t cmd_number);
static int run_unidentified_error (const char* line, const char* message, size_t cmd_number);

static int try_register(const char* argument);
static int exist_arg   (const char* buffer);

static int label_buffer_ctor(asm_label_info* labels);
static int label_buffer_dtor(asm_label_info* labels);

//todo file information
// static int write_info  (FILE* out_ptr);
//todo file with labels
// static int write_labels(FILE* out_ptr, asm_label_info* labels);
static int write_code  (FILE* out_ptr, short* code, size_t code_size);

int main(int argc, const char* argv[]) {
    const char* input_file = argc > 1 ? argv[1] : BASE_INPUT_FILE;
    assert(input_file);

    const char* output_file = BASE_OUTPUT_FILE;
    assert(output_file);

    int error = 0;

    FILE* output_ptr = fopen(output_file, "wb");
    if (!output_ptr) {run_file_error(output_file); return 1;}

    char* buffer = (char*)get_file_buffer(input_file);
    if(!buffer) error = 1;

    LOG_YELLOW ("%s", buffer);

    short* code = (short*)calloc(MAX_CODE_SIZE, sizeof(short));
    if (!code) error = 1;
    size_t code_size = 0;
    asm_label_info labels = {};
    error |= label_buffer_ctor(&labels);

    if (!error) error = compile_labels(buffer, &labels);
    if (!error) error = compile_code  (buffer, &labels, code, &code_size);

    // write_info  (output_ptr);
    // write_labels(output_ptr, &labels);
    if (!error) {
        write_code(output_ptr, code, code_size);
        LOG_WHITE("WRITED\n");
    }

    fclose(output_ptr);
    error != label_buffer_dtor(&labels);
    free(buffer);
    free(code  );
    fclose(output_ptr);
    if (error) {
        LOG_FATAL("ENDED WITH ERROR\n");
        return 1;
    }
    return 0;
}

int compile_labels (const char* buffer, asm_label_info* labels) {
    assert(labels);
    assert(buffer);
    trie_cell_assert(labels->trie);

    char cur_line [MAX_LINE_SIZE   ] = {};
    char junk     [MAX_LINE_SIZE   ] = {};
    char cur_label[MAX_COMMAND_SIZE] = {};

    size_t line_pos  = 0;
    size_t line_size = 0;

    size_t ip = 0;

    int error = 0;

    const COMMAND* check_cmd = {};

    for(int line_cnt = 1; !error && *buffer; ++line_cnt) {
        if(isblank(*buffer)) sscanf(buffer, "%[ \t\v]%[^\r\n]\r\n%lln", junk, cur_line, &line_size);
        else                 sscanf(buffer, "%[^\r\n]\r\n%lln"        ,       cur_line, &line_size);

        buffer += line_size;
        if (!*cur_line) continue;

        if (cur_line[MAX_LINE_SIZE - 1]) {
            error = run_syntax_error(cur_line, "TOO LONG LINE", line_cnt);
            break;
        }
        if (!sscanf(cur_line, "%[A-Z:]%lln", cur_label, &line_pos)) {
            error = run_unidentified_error(cur_line, "LABEL SCAN ERROR", line_cnt);
            break;
        }
        if (cur_label[MAX_COMMAND_SIZE - 1]) {
            error = run_syntax_error(cur_line, "TOO LONG COMMAND", line_cnt);
            break;
        }
        if (cur_line[line_pos] && !isblank(cur_line[line_pos])) {
            error = run_syntax_error(cur_line, "WRONG SINTAX IN COMMAND", line_cnt);
            break;
        }

        if (cur_label[line_pos - 1] != ':') {
            for(size_t index_cmd = 0; index_cmd < sizeof(CMD_LIST) / sizeof(CMD_LIST[0]); ++index_cmd) {
                check_cmd = CMD_LIST + index_cmd;

                if (!strcmp(cur_label, check_cmd->name)) {
                    ip++;
                    switch (check_cmd->id) {
                    case PUSH:
                    case POP:
                        if (strchr(cur_line, '+')) ip++;
                    default: {
                        ip += check_cmd->n_args;
                        break;
                    }}
                    break;
                }
            }
            continue;
        }

        if (exist_arg(cur_line + line_pos)) {
            error = run_syntax_error(cur_line, "ARGUMENTS WITH LABEL", line_cnt);
            break;
        }

        cur_label[line_pos - 1] = '\0';

        if (trie_exist(labels->trie, cur_label)) {
            error = run_syntax_error(cur_line, "LABEL ALREADY EXIST", line_cnt);
            break;
        }

        trie_insert(labels->trie, cur_label);

        asm_label_unit new_label = {};
        new_label.name = (char*)calloc(line_pos - 1,sizeof(char));
        if (!new_label.name) {
            error = run_unidentified_error(cur_line, "NO LABEL NAME MEMORY ALLOCATE", line_cnt);
            continue;
        }
        strcpy(new_label.name, cur_label);
        new_label.ip = ip;

        memcpy(labels->data + labels->size++, &new_label, sizeof(asm_label_unit));
    }

    if (error) {
        LOG_FATAL("ASSEMBLE CRASHED\nRUN AGAIN\n");
        return 1;
    }
    return 0;
}

int compile_code   (const char* buffer, const asm_label_info* labels, short* code, size_t* code_size) {
    assert(labels   );
    assert(buffer   );
    assert(code     );
    assert(code_size);
    trie_cell_assert(labels->trie);

    char cur_line[MAX_LINE_SIZE   ] = {};
    char junk    [MAX_LINE_SIZE   ] = {};
    char cur_cmd [MAX_COMMAND_SIZE] = {};

    const short* const code_begin = code;

    size_t line_pos  = 0;
    size_t line_size = 0;

    int error = 0;

    const COMMAND* check_cmd = {};

    for(int line_cnt = 1; *buffer && !error; ++line_cnt) {
        if(isblank(*buffer)) sscanf(buffer, "%[ \t\v][^\r\n]\r\n%lln", junk, cur_line, &line_size);
        else                 sscanf(buffer, "%[^\r\n]\r\n%lln"       ,       cur_line, &line_size);

        buffer += line_size;
        // printf("%s\n", cur_line);
        if (!*cur_line) continue;

        if (cur_line[MAX_LINE_SIZE - 1]) {
            error = run_syntax_error(cur_line, "TOO LONG LINE", line_cnt);
            break;
        }
        if (!sscanf(cur_line, "%[A-Z:]%lln", cur_cmd, &line_pos)) {
            error = run_unidentified_error(cur_line, "COMMAND SCAN ERROR", line_cnt);
            break;
        }
        if (cur_cmd[MAX_COMMAND_SIZE - 1]) {
            error = run_syntax_error(cur_line, "TOO LONG COMMAND", line_cnt);
            break;
        }
        if (cur_line[line_pos] && !isblank(cur_line[line_pos])) {
            error = run_syntax_error(cur_line, "WRONG SINTAX IN COMMAND", line_cnt);
            break;
        }

        if (cur_cmd[line_pos - 1] == ':') continue;

        for(size_t index_cmd = 0; index_cmd < sizeof(CMD_LIST) / sizeof(CMD_LIST[0]); ++index_cmd) {
            check_cmd = CMD_LIST + index_cmd;

            if (check_cmd->id == UNDEF){
                error = run_syntax_error(cur_line, "UNDEFINED COMMAND", line_cnt);
            }

            if (!strcmp(cur_cmd, check_cmd->name)) {
                *(code++) = check_cmd->id;
                LOG_PURPLE("%-5s %lld\n", check_cmd->name, code - code_begin);

                switch (check_cmd->agr_type){
                case NUM_ARG: {
                    line_pos++;
                    error = compile_num_arg    (cur_line, &line_pos, line_cnt, &code, check_cmd->n_args);
                    break;
                }
                case LABEL_ARG: {
                    line_pos++;
                    error = compile_label_arg  (cur_line, &line_pos, line_cnt, &code, labels);
                    break;
                }
                case POP_ARG:
                case PUSH_ARG: {
                    line_pos++;
                    error = compile_complex_arg (cur_line, &line_pos, line_cnt, &code);
                    break;
                }
                case NO_ARG: {
                    break;
                }
                default: {
                    LOG_FATAL("%s", cur_line);
                    assert(!"Wrong arg_type in CMD_LIST");
                    break;
                }}

                if(!error) {
                    if (exist_arg(cur_line + line_pos)) {
                        run_syntax_error(cur_line, "TOO MANY ARGUMENTS", line_cnt);
                        error = 1;
                    }
                }

                break;
            }
        }
        if (code - code_begin >= MAX_CODE_SIZE) {
            error = run_unidentified_error(cur_line, "Code buffer overflow", line_cnt);
        }
    }

    *code_size = code - code_begin;

    if (error) {
        LOG_FATAL("ASSEMBLE CRASHED\nRUN AGAIN\n");
        return 1;
    }
    LOG_INFO("ASSEMBLER COMPLITED\nREADY TO PROCESSOR\n");
    return 0;
}

static int run_syntax_error(const char* line, const char* message, size_t line_num) {
    assert(line);
    assert(message);

    LOG_FATAL("________________________\n");
    LOG_FATAL("SYNTAX ERROR\nIn line: ");
    LOG_CERR ("%llu\n", line_num);
    LOG_FATAL("%s\n%s\n", line, message);
    LOG_FATAL("________________________\n");

    return 1;
}

static int run_unidentified_error(const char* line, const char* message, size_t line_num) {
    assert(line);
    assert(message);

    LOG_FATAL("________________________\n");
    LOG_FATAL("UNDEFINED ERROR\nIn line: ");
    LOG_CERR ("%llu\n", line_num);
    LOG_FATAL("%s\n%s\n", line, message);
    LOG_FATAL("________________________\n");

    return 1;
}

static int exist_arg   (const char* buffer) {
    if (!*buffer) return 0;
    char some_arg = '\0';
    return sscanf(buffer, "%c\n", &some_arg) != 0;
}

static int try_register(const char* argument) {
    if (argument[2] == '\0' && argument[1] == REGISTER_IDENTIFICATOR &&
        argument[0] >= 'a'  && argument[0] <= 'a' + REGS_SIZE) {
        return argument[0] - 'a';
    }
    return -1;
}

static int label_buffer_ctor(asm_label_info* labels) {
    assert(labels);

    *labels = {};
    labels->trie = trie_ctor();
    labels->data = (asm_label_unit*)calloc(MAX_LABELS, sizeof(asm_label_unit));
    if (!labels->data) return 1;

    return 0;
}
static int label_buffer_dtor(asm_label_info* labels) {
    assert(labels);

    trie_dtor(labels->trie);
    free(labels->data);
    *labels = {};

    return 0;
}
static possible_argument_t convert_to_num_or_reg(const char* argument) {
    assert(argument);

    possible_argument_t result = {};

    if ((result.value = (short)try_register(argument)) != -1) {
        result.type = REG_T;
        return result;
    }

    int is_num = 1;
    int ind    = 0;

    if(argument[ind] == '-') ind++;
    while(argument[ind]) is_num &= isdigit(argument[ind++]);

    if (is_num && *argument) {
        result.type  = NUM_T;
        result.value = (short)atoi(argument);
        return result;
    }

    result.type = ERR_T;
    return result;
}

static int compile_num_arg   (const char* line, size_t* pos, size_t line_num, short** code, int n_args) {
    assert(line);
    assert(code);
    assert(pos );

    short  num_arg = 0;
    size_t delta   = 0;

    for(int i = 0; i < n_args; ++i) {
        if(sscanf(line + (*pos += ++delta), "%hd%lln", &num_arg, &delta) == 0) {
            return run_syntax_error(line, "TOO FEW ARGUMENTS", line_num);
        }
        // printf("%d %d\n", i, num_arg);
        *((*code)++) = num_arg;
    }
    *pos += delta;
    // printf("%d %c\n", *(line + *pos), *(line + *pos));
    return 0;
}
static int compile_complex_arg  (const char* line, size_t* pos, size_t line_num, short** code) {
    assert(line);
    assert(code);
    assert(pos );

    char   str_arg[MAX_ARGUMENT_SIZE] = {};
    short  arg_type  = 0;

    size_t code_delta = 2;
    size_t delta      = 0;

    int    scanf_res = 0;
    int    wrong_arg = 0;
    possible_argument_t arg_converted = {};


    (*code)--;


    if(strchr(line, '[')) {
        arg_type |= RAM_T;
        scanf_res = sscanf(line + *pos, "[%[^] ]]%lln", str_arg, &delta);
    }
    else {
        scanf_res = sscanf(line + *pos, "%s%lln", str_arg, &delta);
    }

    *pos += delta;

    if (!scanf_res) {
        return run_unidentified_error(line, "ARGUMENTS SCAN ERROR", line_num);
    }
    if (str_arg[MAX_COMMAND_SIZE - 1]) {
        return run_syntax_error(line, "TOO LONG COMMAND", line_num);
    }
    if (strchr(str_arg, '+')) {
        code_delta++;
        char str_arg_list[2][MAX_ARGUMENT_SIZE] = {{}, {}};
        sscanf(str_arg, "%[^+]+%s", str_arg_list[0], str_arg_list[1]);

        arg_converted = convert_to_num_or_reg(str_arg_list[0]);
        if (arg_converted.type == ERR_T) wrong_arg = 1;
        *(*code + 1) = arg_converted.value;
        arg_type |= (short)arg_converted.type;

        arg_converted = convert_to_num_or_reg(str_arg_list[1]);
        if (arg_converted.type == ERR_T) wrong_arg = 1;
        *(*code + 2) = arg_converted.value;
        arg_type |= (short)arg_converted.type;

        if (!wrong_arg && (arg_type & (REG_T | NUM_T)) != (REG_T | NUM_T)) {
            return run_syntax_error(line, "ARGUMENTS EQUAL TYPES", line_num);
        }
    }
    else {
        arg_converted = convert_to_num_or_reg(str_arg);
        if (arg_converted.type == ERR_T) wrong_arg = 1;
        *(*code + 1) = arg_converted.value;
        arg_type += (short)arg_converted.type;
    }

    if (wrong_arg) {
        return run_syntax_error(line, "WRONG ARGUMENTS SYNTAX", line_num);
    }

    **code |= arg_type;
    *code  += code_delta;
    // printf("%d\n", arg_type);
    return 0;

}
static int compile_label_arg (const char* line, size_t* pos, size_t line_num, short** code, const asm_label_info* labels) {
    assert(line  );
    assert(code  );
    assert(pos   );
    assert(labels);

    char str_arg[MAX_ARGUMENT_SIZE] = {};
    size_t delta  = 0;
    assert(str_arg);

    if (!sscanf(line + *pos, "%[A-Z]%lln", str_arg, &delta)) {
        return run_unidentified_error(line, "LABEL ARG SCAN ERROR", line_num);
    }
    *pos += delta;
    if (str_arg[MAX_ARGUMENT_SIZE - 1]) {
        return run_syntax_error(line, "TOO LONG LABEL ARG", line_num);
    }
    if (line[*pos] && !isblank(line[*pos])) {
        return run_syntax_error(line, "WRONG SINTAX IN COMMAND", line_num);
    }

    if (!trie_exist(labels->trie, str_arg)) {
        return run_syntax_error(line, "LABEL NOT EXIST", line_num);
    }

    **code = -1;
    for (size_t i = 0; i < labels->size; i++) {
        if (!strcmp(labels->data[i].name, str_arg)) {
            *((*code)++) = (short)labels->data[i].ip;
            break;
        }
    }

    if (**code == -1) {
        return run_unidentified_error(line, "LABEL BUFFER ERROR", line_num);
    }
    return 0;
}

static int write_code  (FILE* out_ptr, short* code, size_t code_size) {
    int error = 0;
    error |= !fwrite(&code_size, sizeof(size_t), 1        , out_ptr);
    error |= !fwrite(code      , sizeof(short ), code_size, out_ptr);
    return error;
}

#undef BASE_INPUT_FILE
#undef BASE_OUTPUT_FILE
#undef REGISTER_IDENTIFICATOR
#undef STACK_TYPE_ARGUMENT
#undef MAX_ARGUMENT_SIZE
#undef MAX_COMMAND_SIZE
#undef MAX_LINE_SIZE
