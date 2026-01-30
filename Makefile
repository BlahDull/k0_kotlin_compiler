# Compiler and tools
CC = gcc
CFLAGS = -c -g -Wall
BISON = bison
FLEX = flex

# Source files
BISON_SRC = k0gram.y
FLEX_SRC = k0lex.l
MAIN_SRC = main.c
TREE_SRC = tree.c
SYMTAB_SRC = symtab.c
TYPE_SRC = type.c
TAC_SRC = tac.c
IC_SRC = ic.c
ASM_SRC = tac2asm.c


# Generated files
BISON_C = k0gram.c
BISON_H = k0gram.h
FLEX_C = k0lex.c
TREE_PNG = tree.png
DOT_FILE = tree.dot
IC_FILE = *.ic
ASSEM_FILE = *.S
# Object files
BISON_O = k0gram.o
FLEX_O = k0lex.o
MAIN_O = main.o
TREE_O = tree.o
SYMTAB_O = symtab.o
TYPE_O = type.o
TAC_O = tac.o
IC_O = ic.o
ASM_O = tac2asm.o

# Output executable
EXEC = k0

# Default rule
all: $(EXEC)

# Generate Bison parser files
$(BISON_C) $(BISON_H): $(BISON_SRC)
	$(BISON) -d -t $(BISON_SRC) -o $(BISON_C)

# Generate Flex scanner files
$(FLEX_C): $(FLEX_SRC)
	$(FLEX) -o $(FLEX_C) $(FLEX_SRC)

# Compile Bison output
$(BISON_O): $(BISON_C)
	$(CC) $(CFLAGS) $(BISON_C) -o $(BISON_O)

# Compile Flex output
$(FLEX_O): $(FLEX_C)
	$(CC) $(CFLAGS) $(FLEX_C) -o $(FLEX_O)

# Compile main module
$(MAIN_O): $(MAIN_SRC)
	$(CC) $(CFLAGS) $(MAIN_SRC) -o $(MAIN_O)

# Compile tree module
$(TREE_O): $(TREE_SRC) tree.h
	$(CC) $(CFLAGS) $(TREE_SRC) -o $(TREE_O)

# Compile symtab module
$(SYMTAB_O): $(SYMTAB_SRC) symtab.h
	$(CC) $(CFLAGS) $(SYMTAB_SRC) -o $(SYMTAB_O)

# Compile type module
$(TYPE): $(TYPE_SRC) type.h 
	$(CC) $(CFLAGS) $(TYPE_SRC) -o $(TYPE_O)

# Compile tac module
$(TAC): $(TAC_SRC) tac.h 
	$(CC) $(CFLAGS) $(TAC_SRC) -o $(TAC_O)

# Compile ic module
$(IC): $(IC_SRC) tac.h 
	$(CC) $(CFLAGS) $(IC_SRC) -o $(IC_O)

# Compile ic module
$(ASM): $(ASM_SRC) tac.h 
	$(CC) $(CFLAGS) $(IC_SRC) -o $(ASM_O)

# Link everything into the final executable
$(EXEC): $(BISON_O) $(FLEX_O) $(TREE_O) $(SYMTAB_O) $(TYPE_O) $(TAC_O) $(IC_O) $(ASM_O) $(MAIN_O)
	$(CC) -o $(EXEC) $(MAIN_O) $(BISON_O) $(FLEX_O) $(TREE_O) $(SYMTAB_O) $(TYPE_O) $(TAC_O) $(IC_O) $(ASM_O) -lfl

# Check for leaks
valgrind: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./$(EXEC) in.kt

# Clean up generated files
clean:
	rm -f $(EXEC) $(BISON_C) $(BISON_H) $(FLEX_C) $(BISON_O) $(FLEX_O) $(MAIN_O) $(TREE_O) $(SYMTAB_O) $(TYPE_O) $(TAC_O) $(IC_O) $(ASM_O) $(TREE_PNG) $(DOT_FILE) $(IC_FILE) $(ASSEM_FILE) a.out *.o

# *.ic *.s *.o
