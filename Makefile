ded_flags = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef \
-Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations		\
-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain   \
-Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion            \
-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2           \
-Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers       \
-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo    \
-Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel             \
-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

.phony: all clean intall uninstall

B_PREFIX = bin/
O_PREFIX = objects/
S_PREFIX = sources/
H_PREFIX = headers/
T_PREFIX = text_file/

STACK_HEADER = ../Stack/sources/stack.h

ASSEMBLER_S    = assembler commoner trie
PROCESSOR_S    = processor commoner soft_processor_unit cmd_proc stack
DISASSEMBLER_S =

ASSEMBLER_O    := $(patsubst %,$(O_PREFIX)%.o,$(ASSEMBLER_S))
PROCESSOR_O    := $(patsubst %,$(O_PREFIX)%.o,$(PROCESSOR_S))
DISASSEMBLER_O := $(patsubst %,$(O_PREFIX)%.o,$(DISASSEMBLER_S))

HEADER_LIST =$(H_PREFIX)*.h $(STACK_HEADER)

all: assembler processor

print:
	@echo $(ASSEMBLER_O)

run_all: run_assembler run_processor

run_assembler: assembler
	@./$(B_PREFIX)assembler.exe

run_processor: processor
	@./$(B_PREFIX)processor.exe

assembler: $(ASSEMBLER_O)
	@mkdir -p $(B_PREFIX)
	@gcc $(ded_flags) $^ -o $(B_PREFIX)$@

processor: $(PROCESSOR_O)
	@mkdir -p $(B_PREFIX)
	@gcc $(ded_flags) $^ -o $(B_PREFIX)$@

$(O_PREFIX)%.o: $(S_PREFIX)%.cpp $(HEADER_LIST)
	@mkdir -p $(O_PREFIX)
	@gcc $(ded_flags) -I $(H_PREFIX) $< -c -o $@

clean:
	rm -rf $(O_PREFIX)*.o $(B_PREFIX)*.exe $(T_PREFIX)logs.txt
