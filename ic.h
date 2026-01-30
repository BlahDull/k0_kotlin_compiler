#ifndef ic_H
#define ic_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "k0gram.h"
#include "tac.h"
#include "tree.h"
#include "symtab.h"
#include "type.h"

typedef struct {
    char *name;
    char *data;
    int offset;
} StringTableEntry;

typedef struct {
    StringTableEntry entries[1000];
    int count;
    int current_offset;
} StringTable;

extern StringTable string_table;
void create_ic(char *ic_file, ListSymbolTables tables, struct tree *node);
struct data_decl *create_data_decls(ListSymbolTables list);
void print_data_section(FILE *fp, struct data_decl *decl_list);
void collect_strings(struct tree* t);
void string_section(FILE* out);
void intermediate_code(struct tree* t, FILE* out);
bool needs_first_label(int prodrule);
void assign_first(struct tree *t);
bool needs_follow_label(int prodrule);
void assign_follow(struct tree *t);
bool needs_condition_labels(int prodrule);
void assign_condition(struct tree *t);
void print_labels(struct tree *t, int depth);
void generate_code(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr *labels);
#endif
