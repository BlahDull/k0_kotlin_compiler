#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tree.h"
#include "k0gram.h"
#include "type.h"

enum Kind { VARIABLE, FUNCTION, TEMP, CONSTANT, PARAM };

typedef struct sym_entry
{
    /*   SymbolTable table;			 what symbol table do we belong to*/
    char *s; /* string */
    struct sym_table *scope;
    /* more symbol attributes go here for code generation */
    struct sym_entry *next;
    // bool constant;
    bool built_in;
    enum Kind kind;
    typeptr type;
    int memloc;
} *SymbolTableEntry;

typedef struct sym_table
{
    int nEntries; /* # of symbols in the table */
    int nBuckets;
    /* struct sym_table *parent;		 enclosing scope, superclass etc. */
    struct sym_entry **tbl;
    struct sym_table *parent;
    bool function;
    bool package;
    bool class;
    char *table_name;
    int current_offset;
    /* more per-scope/per-symbol-table attributes go here */
} *SymbolTable;

typedef struct symbol_table_list
{
    SymbolTable table;
    struct symbol_table_list *next;
    int tab_count;
} *ListSymbolTables;

void semantic_error(int error, int lineno, char *filename, ...);
void print_table_header(SymbolTable table);
void print_parameters(paramlist params);
void print_symbol_entry(SymbolTableEntry entry);
void print_table_contents(SymbolTable table);
void printsymbol(char *s);
void print_tables(ListSymbolTables tables);
SymbolTable mksymtab(SymbolTable parent);
SymbolTableEntry create_nentry(char *text, SymbolTable tab, typeptr type);
ListSymbolTables add_symbol_table(ListSymbolTables list, SymbolTable table);
int hash(SymbolTable st, const char *s);
SymbolTableEntry find_symbol(SymbolTable tab, const char *symbolText);
void insert_builtin_function(SymbolTable tab, char *symbolText, int type, int nparams, ...);
void extract_import_name(struct tree *node, char *buffer, size_t buffer_size);
void insert_symbol(SymbolTable tab, char *symbolText, SymbolTableEntry nentry);
void insert_predefined_symbols(SymbolTable tab);
SymbolTable process_import_declaration(struct tree *node, SymbolTable currentScope, ListSymbolTables list);
paramlist process_parameter(struct tree *node);
void process_function_parameters(struct tree *node, SymbolTable func_scope, typeptr func_info);
typeptr process_array_type(struct tree *node);
typeptr process_function_return_type(char *s, struct tree *node, char *func_name);
char *extract_fun_name(struct tree *node);
SymbolTable process_function_declaration(struct tree *node, SymbolTable outer_scope);
void process_arg_node(struct tree *node, SymbolTable scope, paramlist *currentParam, int *argCount, int lineno, char *filename, char *funcName);
void check_argument_list(struct tree *argNode, paramlist expectedParams, SymbolTable scope, int lineno, char *filename, char *funcName);
int validate_function_call(struct tree *node, SymbolTable scope);
typeptr process_declaration_type(struct tree *node, SymbolTable scope);
void process_variable_declaration(struct tree *node, SymbolTable scope);
void process_assignment(struct tree *node, SymbolTable scope);
void process_expression(struct tree *node, SymbolTable scope);
void extract_symbols(struct tree *node, SymbolTable currentScope, ListSymbolTables list, struct tree *parent);
SymbolTable find_symbol_table(ListSymbolTables list, char *name);
int check_expression(struct tree *node, SymbolTable tab);
// void check_functioncall_parameters(SymbolTableEntry function, struct tree *node, SymbolTable tab);
void check_assignment(SymbolTableEntry var, struct tree *node, SymbolTable tab);
void check_symbols(struct tree *node, SymbolTable currentScope, ListSymbolTables list, struct tree *parent);
void free_symtab(ListSymbolTables head);
ListSymbolTables create_symtabs(struct tree *node, int print, int free);

#endif
