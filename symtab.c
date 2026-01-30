#include "symtab.h"
#include "type.h"
#include <stdarg.h>

extern typeptr null_typeptr;
extern typeptr integer_typeptr;
extern typeptr double_typeptr;
extern typeptr boolean_typeptr;
extern typeptr char_typeptr;
extern typeptr unit_typeptr;

//int current_offset = 0;

enum sem_errors {
    FUNC_REDECL,
    FUNC_UNDECL,
    FUNC_BAD_ARGS,
    FUNC_NO_RET,
    FUNC_INCOMP_RET,
    FUNC_RET_TYPE_MISMATCH,
    VAR_REDECL,
    VAR_UNDECL,
    TYPE_MISMATCH,
    INVALID_OP,
    BAD_UMINUS,
    DIV_ZERO,
    NO_MAIN
};

void semantic_error(int error, int lineno, char *filename, ...) {
    va_list args;
    va_start(args, filename);
    fprintf(stderr, "Semantic Error: ");
    const char *func_name, *val_name, *type1, *type2, *msg, *op;
    switch(error) {
        case FUNC_REDECL:
            func_name = va_arg(args, const char *);
            fprintf(stderr, "Function '%s' redeclared in %s on line %d.\n", func_name, filename, lineno);
            break;
        case FUNC_UNDECL:
            func_name = va_arg(args, const char *);
            fprintf(stderr, "Function '%s' undeclared before use in %s on line %d.\n", func_name, filename, lineno);
            break;
        case VAR_REDECL:
            val_name = va_arg(args, const char *);
            fprintf(stderr, "Variable '%s' redeclared in %s on line %d.\n", val_name, filename, lineno);
            break;
        case VAR_UNDECL:
            val_name = va_arg(args, const char *);
            fprintf(stderr, "Variable '%s' undeclared before use in %s on line %d.\n", val_name, filename, lineno);
            break;
        case TYPE_MISMATCH:
            type1 = va_arg(args, const char *);
            type2 = va_arg(args, const char *);
            fprintf(stderr, "Type mismatch! Expected type <%s> but got <%s>. Line %d in file %s.\n", type1, type2, lineno, filename);
            break;
        case INVALID_OP:
            type1 = va_arg(args, const char *);
            type2 = va_arg(args, const char *);
            fprintf(stderr, "Invalid operation between <%s> and <%s> types. Found on line %d in file %s.\n", type1, type2, lineno, filename);
            break;
        case DIV_ZERO:
            fprintf(stderr, "Expression on line %d contains division by zero in file %s.\n", lineno, filename);
            break;
        case FUNC_BAD_ARGS:
            func_name = va_arg(args, const char *);
            msg = va_arg(args, const char *);
            fprintf(stderr, "Error in function call for '%s': %s. Found on line %d in file %s.\n", func_name, msg, lineno, filename);
            break;
        case BAD_UMINUS:
            op = va_arg(args, const char *);
            type1 = va_arg(args, const char *);
            fprintf(stderr, "Cannot use operator '%s' with type <%s>. Found on line %d in file %s.\n", op, type1, lineno, filename);
            break;
        case FUNC_NO_RET:
            func_name = va_arg(args, const char *);
            type1 = va_arg(args, const char *);
            fprintf(stderr, "Function '%s' declares return type <%s>, but no return statement found. Found on line %d in file %s.\n", func_name, type1, lineno, filename);
            break;
        case FUNC_INCOMP_RET:
            func_name = va_arg(args, const char *);
            fprintf(stderr, "Function '%s' has return statement but no value was given. Found on line %d in file %s.\n", func_name, lineno, filename);
            break;
        case FUNC_RET_TYPE_MISMATCH:
            func_name = va_arg(args, const char *);
            type1 = va_arg(args, const char *);
            type2 = va_arg(args, const char *);
            fprintf(stderr, "Function '%s' has expected return type <%s> but got <%s>. Found on line %d in file %s.\n", func_name, type1, type2, lineno, filename);
            break;
        case NO_MAIN:
            fprintf(stderr, "Function 'main' not found.\n");
            break;

    }
    va_end(args);
    exit(3);
}

void print_table_header(SymbolTable table)
{
    if (table->parent == NULL)
    {
        printf("Symbol table for %-15s\n", table->table_name);
    }
    else if (table->function)
    {
        printf("Symbol table for func %-12s\tParent scope: %s\n", table->table_name, table->parent->table_name);
    }
    puts("--------------------------------------------------------------------------------------------------------");
}

void print_parameters(paramlist params)
{
    while (params != NULL)
    {
        printf("Symbol: %-20s\tData type: %-15s\n", params->name, typeint_to_name(params->type->basetype));
        params = params->next;
    }
}

void print_symbol_entry(SymbolTableEntry entry)
{
    if (entry->type->basetype == FUNC_TYPE)
    {
        if (entry->type->u.f.returntype->basetype == ARRAY_TYPE)
        {
            printf("Symbol: %-20s\tData type: %-15s\tReturn type: Array<%s>\n",
                   entry->s,
                   typeint_to_name(entry->type->basetype),
                   typeint_to_name(entry->type->u.f.returntype->u.a.elemtype->basetype));
        }
        else
        {
            printf("Symbol: %-20s\tData type: %-15s\tReturn type: %-15s\n",
                   entry->s,
                   typeint_to_name(entry->type->basetype),
                   typeint_to_name(entry->type->u.f.returntype->basetype));
        }
    }
    else if (entry->type->basetype == ARRAY_TYPE)
    {
        printf("Symbol: %-20s\tData type: Array<%s>\n", entry->s, typeint_to_name(entry->type->u.a.elemtype->basetype));
    }
    else
    {
        if (entry->kind != CONSTANT)
            printf("Symbol: %-20s\tData type: %-15s\n",
                   entry->s,
                   typeint_to_name(entry->type->basetype));
        else
        {
            printf("Symbol: %-20s\tData type: Constant %-15s\n",
                   entry->s,
                   typeint_to_name(entry->type->basetype));
        }
    }
}

void print_table_contents(SymbolTable table)
{

    if (table->package == true)
    {
        for (int i = 0; i < table->nBuckets; i++)
        {
            SymbolTableEntry entry = table->tbl[i];
            while (entry != NULL)
            {
                print_symbol_entry(entry);
                entry = entry->next;
            }
        }
        puts("--------------------------------------------------------------------------------------------------------");
        return;
    }

    for (int i = 0; i < table->nBuckets; i++)
    {
        SymbolTableEntry entry = table->tbl[i];
        while (entry != NULL)
        {
            switch (entry->type->basetype)
            {
            case FUNC_TYPE:
                //print_parameters(entry->type->u.f.parameters);
                break;
            default:
                print_symbol_entry(entry);
            }
            entry = entry->next;
        }
    }
    puts("--------------------------------------------------------------------------------------------------------");
}

void printsymbol(char *s)
{
    printf("%s\n", s);
    fflush(stdout);
}

void print_tables(ListSymbolTables tables)
{
    ListSymbolTables current = tables;
    int tableIndex = 1;

    printf("Symbol Tables:\n");
    while (current != NULL)
    {
        print_table_header(current->table);
        print_table_contents(current->table);
        printf("\n\n");
        current = current->next;
        tableIndex++;
    }
}

SymbolTable mksymtab(SymbolTable parent)
{
    SymbolTable ntab = malloc(sizeof(struct sym_table));
    if (ntab == NULL)
    {
        perror("Memory allocation failed");
        exit(4);
    }
    ntab->nBuckets = 50;
    ntab->nEntries = 0;
    ntab->current_offset = 0;
    ntab->parent = parent;
    ntab->tbl = calloc(ntab->nBuckets, sizeof(SymbolTableEntry));
    ntab->function = false;
    ntab->package = false;
    ntab->class = false;
    return ntab;
}

SymbolTableEntry create_nentry(char *text, SymbolTable tab, typeptr type)
{
    SymbolTableEntry nentry = calloc(1, sizeof(struct sym_entry));
    nentry->s = text;
    nentry->scope = tab;
    nentry->type = type;
    nentry->next = NULL;
    nentry->built_in = false;
    // if (type->basetype != FUNC_TYPE) {
    //     nentry->memloc = tab->current_offset;
    //     tab->current_offset += 8;
    // }
    switch (type->basetype) {
        case FUNC_TYPE:
            break;
        case BOOL_TYPE:
            nentry->memloc += tab->current_offset;
            tab->current_offset += 1;
            break;
        case CHAR_TYPE:
            nentry->memloc += tab->current_offset;
            tab->current_offset += 1;
            break;
        default:
            nentry->memloc += tab->current_offset;
            tab->current_offset += 8;
    }
    return nentry;
}

ListSymbolTables add_symbol_table(ListSymbolTables list, SymbolTable table)
{
    ListSymbolTables newListEntry = malloc(sizeof(struct symbol_table_list));
    if (newListEntry == NULL)
    {
        perror("Memory allocation failed");
        exit(4);
    }
    newListEntry->table = table;
    newListEntry->next = NULL;
    newListEntry->tab_count = list->tab_count + 1;

    ListSymbolTables current = list;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = newListEntry;

    return newListEntry;
}

int hash(SymbolTable st, const char *s)
{
    register int h = 0;
    register char c;
    while ((c = *s++))
    {
        h += c & 0377;
        h *= 37;
    }
    if (h < 0)
        h = -h;
    return h % st->nBuckets;
}

SymbolTableEntry find_symbol(SymbolTable tab, const char *symbolText)
{
    if (tab == NULL)
    {
        return NULL;
    }
    if (tab->tbl == NULL)
    {
        printf("ERROR: tab->tbl is NULL!\n");
        exit(4);
    }
    if (tab->nBuckets <= 0)
    {
        printf("ERROR: nBuckets is zero or negative!\n");
        exit(4);
    }

    int bucket = hash(tab, symbolText);
    if (bucket < 0 || bucket >= tab->nBuckets)
    {
        printf("ERROR: Invalid bucket index %d!\n", bucket);
        exit(4);
    }

    SymbolTableEntry entry = tab->tbl[bucket];

    while (entry != NULL)
    {
        if (entry->s != NULL && strcmp(entry->s, symbolText) == 0)
        {
            // printf("%s\n", entry->s);
            return entry;
        }
        entry = entry->next;
    }
    return find_symbol(tab->parent, symbolText);
}

void insert_builtin_function(SymbolTable tab, char *symbolText, int type, int nparams, ...)
{
    typeptr ptr = alctype(FUNC_TYPE);
    ptr->basetype = FUNC_TYPE;
    ptr->u.f.defined = 1;
    ptr->u.f.name = symbolText;
    SymbolTableEntry nentry = create_nentry(symbolText, tab, ptr);
    nentry->kind = FUNCTION;
    nentry->built_in = true;
    if (type)
    {
        ptr->u.f.returntype = alctype(type);
    }
    else
    {
        ptr->u.f.returntype = unit_typeptr;
    }
    nentry->type = ptr;
    ptr->u.f.nparams = nparams;
    // Process variadic args to build param list
    va_list args;
    va_start(args, nparams);
    paramlist head = NULL;
    paramlist *curr = &head;
    for (int i = 0; i < nparams; i++) {
        char *paramType = va_arg(args, char*);  // get next parameter type
        *curr = malloc(sizeof(struct param));
        (*curr)->name = NULL;  // optionally set a name
        (*curr)->type = alctype(name_to_typeint(paramType));
        (*curr)->next = NULL;
        curr = &((*curr)->next);
    }
    va_end(args);
    ptr->u.f.parameters = head;
    insert_symbol(tab, symbolText, nentry);
}

void extract_import_name(struct tree *node, char *buffer, size_t buffer_size)
{
    if (node == NULL)
    {
        return;
    }
    if (node->leaf != NULL)
    {
        strncat(buffer, node->leaf->text, buffer_size - strlen(buffer) - 1);
    }
    for (int i = 0; i < node->nkids; i++)
    {
        extract_import_name(node->kids[i], buffer, buffer_size);
    }
}

void insert_symbol(SymbolTable tab, char *symbolText, SymbolTableEntry nentry)
{
    int bucket = hash(tab, symbolText);

    SymbolTableEntry existing = tab->tbl[bucket];

    while (existing != NULL)
    {
        if (strcmp(existing->s, symbolText) == 0)
        {
            return;
        }
        existing = existing->next;
    }
    nentry->next = tab->tbl[bucket];
    tab->tbl[bucket] = nentry;
}

void insert_predefined_symbols(SymbolTable tab)
{
    insert_builtin_function(tab, "println", 0, 1, "Any");
}

SymbolTable process_import_declaration(struct tree *node, SymbolTable currentScope, ListSymbolTables list)
{

    SymbolTable packageScope = mksymtab(currentScope);
    packageScope->package = true;

    add_symbol_table(list, packageScope);

    return packageScope;
}

paramlist process_parameter(struct tree *node)
{
    if (node == NULL)
        return NULL;
    paramlist parameter = malloc(sizeof(struct param));
    char *name = node->kids[0]->leaf->text;
    char *type = node->kids[1]->kids[1]->leaf->text;
    typeptr ptr = alctype(name_to_typeint(type));       // handles singleton/shared
    parameter->type = ptr;
    parameter->name = name;
    parameter->next = NULL;
    return parameter;
}

void process_function_parameters(struct tree *node, SymbolTable func_scope, typeptr func_info)
{
    // no parameters
    if (node->nkids == 0)
    {
        return;
    }
    for (int i = 0; i < node->nkids; i++)
    {
        if (node->kids[i]->prodrule == FUNCVALPARAMS_RULE)
        {
            struct tree *head = node->kids[i];
            paramlist parameter = process_parameter(head);
            //printf("%s\n", parameter->name)
            SymbolTableEntry nentry = create_nentry(parameter->name, func_scope, parameter->type);
            insert_symbol(func_scope, parameter->name, nentry);
            func_info->u.f.nparams++;
            if (func_info->u.f.parameters == NULL)
            {
                func_info->u.f.parameters = parameter;
            }
            else
            {
                paramlist params_end = func_info->u.f.parameters;
                while (1)
                {
                    if (params_end->next == NULL)
                    {
                        params_end->next = parameter;
                        break;
                    }
                    else
                    {
                        params_end = params_end->next;
                    }
                }
            }
        }
        else if (node->kids[i]->prodrule == FUNCVALPARAMSLIST_RULE)
        {
            process_function_parameters(node->kids[i], func_scope, func_info);
        }
    }
}

// TODO, support getting the size of the array
typeptr process_array_type(struct tree *node)
{
    // struct tree *type_node = node->kids[3];
    char s[32];
    sscanf(node->leaf->text, "Array<%[^>]>", s);
    typeptr ptr = alctype(ARRAY_TYPE);
    ptr->basetype = ARRAY_TYPE;
    ptr->u.a.elemtype = alctype(name_to_typeint(s));
    ptr->u.a.size = 3;
    return ptr;
}

// validates ret val inside ret statement to match func ret type
void validate_return_statement(SymbolTable scope, struct tree* return_node) {
    // get lineno/filename for errors
    int lineno = return_node->kids[0]->leaf->lineno;
    char *filename = return_node->kids[0]->leaf->filename;
    // get current func
    SymbolTableEntry entry = find_symbol(scope, scope->table_name);
    int var_type = 0;
    // ret val is token
    if (return_node->kids[1]->leaf != NULL) {
        switch(return_node->kids[1]->leaf->category) {
            case IntegerLiteral:
                var_type = INT_TYPE;
                break;
            case DoubleLiteral:
                var_type = DOUBLE_TYPE;
                break;
            case FloatLiteral:
                var_type = FLOAT_TYPE;
                break;
            case CharacterLiteral:
                var_type = CHAR_TYPE;
                break;
            case StringLiteral:
                var_type = STRING_TYPE;
                break;
            case ArrayLiteral:
                printf("TODO: validate returns with arrays\n");
                break;
            case BooleanLiteral:
                var_type = BOOL_TYPE;
                break;
            case NullLiteral:
                var_type = NULL_TYPE;
                break;
            case Identifier:
                char *var_text = return_node->kids[1]->leaf->text;
                SymbolTableEntry var_entry = find_symbol(scope, var_text);
                // undeclared var
                if (var_entry == NULL) {
                    semantic_error(
                        VAR_UNDECL,
                        lineno,
                        filename,
                        var_text
                    );
                }
                // check var type
                if (var_entry->type->basetype != entry->type->u.f.returntype->basetype) {
                    semantic_error(
                        FUNC_RET_TYPE_MISMATCH,
                        lineno,
                        filename,
                        scope->table_name,
                        typeint_to_name(entry->type->u.f.returntype->basetype),
                        typeint_to_name(var_entry->type->basetype)
                    );
                }
                return; // passed, can go back
            default:
                printf("debug: unexpected return arg: got token category %d\n", return_node->kids[1]->leaf->category);
                break;
        }
    }
    // ret val is node
    else {
        var_type = check_expression(return_node->kids[1], scope);
        printf("node is %s\n", typeint_to_name(var_type));

    }
    if (var_type != entry->type->u.f.returntype->basetype) {
        semantic_error(
            FUNC_RET_TYPE_MISMATCH,
            lineno,
            filename,
            scope->table_name,
            typeint_to_name(entry->type->u.f.returntype->basetype),
            typeint_to_name(var_type)
        );
    }
}

// make sure function has return statement; does not validate type
void check_function_has_return_statement(struct tree *func_node, char *func_name, char *ret_type) {
    // block node is last child
    int i = func_node->nkids - 1;
    struct tree *block_node = func_node->kids[i];
    // check statements
    struct tree* statements_node = block_node->kids[1];
    i = statements_node->nkids - 1; // return should be last child node
    // check return statement exists
    if (strcmp(statements_node->kids[i]->symbolname, "ReturnStatement") != 0) {
        semantic_error(
            FUNC_NO_RET,
            func_node->kids[0]->leaf->lineno,
            func_node->kids[0]->leaf->filename,
            func_name,
            ret_type
        );
    }
    // check return statement is complete
    struct tree *ret_statement_node = statements_node->kids[i];
    if (ret_statement_node->kids[0]->leaf != NULL) {
        // incomplete return statement; ie only has `return`. no var given
        semantic_error(
            FUNC_INCOMP_RET,
            ret_statement_node->kids[0]->leaf->lineno,
            ret_statement_node->kids[0]->leaf->filename,
            func_name
        );
    }
}

typeptr process_function_return_type(char *s, struct tree *node, char *func_name)
{
    if (s == NULL)
    {
        return unit_typeptr;
    }
    check_function_has_return_statement(node, func_name, s);
    typeptr ret_type = alctype(name_to_typeint(s));
    return ret_type;
}

char *extract_fun_name(struct tree *node)
{
    for (int i = 0; i < node->nkids; i++)
    {
        if (node->kids[i]->leaf != NULL && node->kids[i]->leaf->category == Identifier)
        {
            return node->kids[i]->leaf->text;
        }
    }
    return "NO_NAME_FUNCTION";
}

SymbolTable process_function_declaration(struct tree *node, SymbolTable outer_scope)
{
    SymbolTable functionScope = mksymtab(outer_scope);
    struct tree *param_head = node->kids[3];
    char *func_name = NULL;
    char *ret_type_str = NULL;
    struct tree *type_node = NULL;
    func_name = extract_fun_name(node);
    // check if function already declared
    if (find_symbol(outer_scope, func_name) != NULL)
    {
        semantic_error(
            FUNC_REDECL,
            node->kids[1]->leaf->lineno,
            node->kids[1]->leaf->filename,
            func_name
        );
    }
    // check if return type specified
    for (int i = 0; i < node->nkids; i++)
    {
        if (node->kids[i]->prodrule == TYPE_RULE)
        {
            if (node->kids[i]->nkids == 0) {
                break;
            }
            ret_type_str = node->kids[i]->kids[1]->leaf->text;
            type_node = node;
        }
        else if (node->kids[i]->leaf != NULL && node->kids[i]->leaf->category == ARRAY_TYPE)
        {
            ret_type_str = node->kids[i]->leaf->text;
            type_node = node;
        }
    }
    typeptr func_info = malloc(sizeof(struct typeinfo));
    func_info->basetype = FUNC_TYPE;
    func_info->u.f.nparams = 0;
    func_info->u.f.parameters = NULL;
    func_info->u.f.st = outer_scope;
    func_info->u.f.name = func_name;
    func_info->u.f.returntype = process_function_return_type(ret_type_str, type_node, func_name);
    process_function_parameters(param_head, functionScope, func_info);
    // SymbolTableEntry nentry = create_nentry(func_name, functionScope, func_info);
    // insert_symbol(functionScope, func_name, nentry);
    // insert_symbol(outer_scope, func_name, nentry);
    SymbolTableEntry outer_nentry = create_nentry(func_name, outer_scope, func_info);
    insert_symbol(outer_scope, func_name, outer_nentry);
    SymbolTableEntry inner_nentry = create_nentry(func_name, functionScope, clone_type(func_info));
    insert_symbol(functionScope, func_name, inner_nentry);

    outer_nentry->kind = FUNCTION;
    inner_nentry->kind = FUNCTION;

    return functionScope;
}

void process_arg_node(struct tree *node, SymbolTable scope, paramlist *currentParam, int *argCount, int lineno, char *filename, char *funcName) {
    if (node == NULL) return;

    if (node->symbolname && strcmp(node->symbolname, "FuncArgList") == 0 && node->nkids == 3) {
        process_arg_node(node->kids[0], scope, currentParam, argCount, lineno, filename, funcName);
        struct tree *argExpr = node->kids[2];
        (*argCount)++;

        if (*currentParam == NULL) {
            semantic_error(
                FUNC_BAD_ARGS,
                lineno,
                filename,
                funcName,
                "too many args"
            );
        }
        else {
            printf("here1\n");
            int argType = check_expression(argExpr, scope);
            if (argType != (*currentParam)->type->basetype) {
                semantic_error(
                    FUNC_BAD_ARGS,
                    lineno,
                    filename,
                    funcName,
                    "arg type mismatch"
                );
            }
            *currentParam = (*currentParam)->next;
        }
    }
    else if (node->leaf != NULL) {
        (*argCount)++;

        if (*currentParam == NULL) {
            semantic_error(
                FUNC_BAD_ARGS,
                lineno,
                filename,
                funcName,
                "too many args"
            );
        }
        else {
            int argType = check_expression(node, scope);
            if (argType != (*currentParam)->type->basetype) {
                semantic_error(
                    FUNC_BAD_ARGS,
                    lineno,
                    filename,
                    funcName,
                    "arg type mismatch"
                );
            }
            *currentParam = (*currentParam)->next;
        }
    }
    else if (node->prodrule == EXPRESSION_RULE){
        // arg is expression
        int argType = check_expression(node, scope);
        if (argType != (*currentParam)->type->basetype) {
            semantic_error(
                FUNC_BAD_ARGS,
                lineno,
                filename,
                funcName,
                "arg type mismatch"
            );
        }
        *currentParam = (*currentParam)->next;

    }
    else {
        printf("debug: unexpected args in funcCall\n");
    }
}


void check_argument_list(struct tree *arg_node, paramlist expectedParams, SymbolTable scope, int lineno, char *filename, char *funcName) {
    if (strcmp(funcName, "println") == 0) return;
    paramlist currentParam = expectedParams;
    int argCount = 0;
    process_arg_node(arg_node, scope, &currentParam, &argCount, lineno, filename, funcName);
    if (currentParam != NULL) {
        semantic_error(
            FUNC_BAD_ARGS,
            lineno,
            filename,
            funcName,
            "too few args"
        );
    }
}

// check args are right amount and type
int validate_function_call(struct tree *node, SymbolTable scope) {
    int lineno = node->kids[0]->leaf->lineno;
    char *filename = node->kids[0]->leaf->filename;
    char *funcName = node->kids[0]->leaf->text;
    SymbolTableEntry entry = find_symbol(scope, funcName);
    // func undeclared
    if (entry == NULL) {
        semantic_error(
            FUNC_UNDECL,
            lineno,
            filename,
            node->kids[0]->leaf->text
        );
    }
    // check arguments
    struct tree *arg_node = node->kids[2];
    // list of args
    if (node->kids[2]->symbolname != NULL) {
        check_argument_list(
            arg_node,
            entry->type->u.f.parameters,
            scope,
            lineno,
            filename,
            funcName
        );
    }
    // only one arg
    else {
        paramlist expected = entry->type->u.f.parameters;
        int argType = check_expression(arg_node, scope);
        if (expected->type->basetype == ANY_TYPE) return 0;
        if (argType != expected->type->basetype) {
            semantic_error(
                FUNC_BAD_ARGS,
                lineno,
                filename,
                funcName,
                "wrong arg type"
            );
        }
    }
    return 0;
}

typeptr process_declaration_type(struct tree *node, SymbolTable scope)
{
    typeptr type;
    // for (int i = 0; i < node->nkids; i++) {
    //     if (node->kids[i]->prodrule == TYPE_RULE) {
    //         if (node->kids[i]->nkids == 0) {
    //             fprintf(stderr, "Syntax Error: Undeclared type for symbol %s\n", node->kids[1]->leaf->text);
    //             exit(2);
    //         } else {
    //             break;
    //         }
    //     }
    // }
        // type explicitly declared
    if (node->kids[2]->prodrule == TYPE_RULE && node->kids[2]->nkids != 0)
    {
        struct tree *type_node = node->kids[2];
        char *var_type = type_node->kids[1]->leaf->text;
        if (name_to_typeint(var_type) == -1)
        {
            printf("Semantic Error: Unknown value type '%s'.\n", var_type);
            exit(3);
        }
        // printf("type: %s\n", var_type);
        type = alctype(name_to_typeint(var_type));
    }
    // implicit
    else {
        struct tree *assign_node = node->kids[3];
        int category = assign_node->kids[1]->leaf->category;
        switch(category) {
            case BooleanLiteral:
                type = alctype(name_to_typeint("Boolean"));
                break;
            case NullLiteral:
                type = alctype(name_to_typeint("Null"));
                break;
            case IntegerLiteral:
                type = alctype(name_to_typeint("Int"));
                break;
            case DoubleLiteral:
                type = alctype(name_to_typeint("Double"));
                break;
            case FloatLiteral:
                type = alctype(name_to_typeint("Float"));
                break;
            case CharacterLiteral:
                type = alctype(name_to_typeint("Char"));
                break;
            case StringLiteral:
                type = alctype(name_to_typeint("String"));
                break;
            case Identifier:
                printf("add Identifier support for assignment\n");
                break;
            case ArrayLiteral:
                printf("add ArrayLiteral support for assignment\n");
                break;
            default:
                fprintf(stderr, "Unknown var assignment");
                exit(4);   
        }
    }
    return type;
}

void process_variable_declaration(struct tree *node, SymbolTable scope)
{
    char *name = node->kids[1]->leaf->text;
    if (find_symbol(scope, name) != NULL)
    {
        semantic_error(
            VAR_REDECL,
            node->kids[1]->leaf->lineno,
            node->kids[1]->leaf->filename,
            name
        );
    }
    else if (node->kids[0]->leaf->category == VAL)
    {
        SymbolTableEntry nentry = create_nentry(name, scope, process_declaration_type(node, scope));
        nentry->kind = CONSTANT;
        insert_symbol(scope, name, nentry);
    }
    else if (node->kids[0]->leaf->category == VAR)
    {
        SymbolTableEntry nentry = create_nentry(name, scope, process_declaration_type(node, scope));
        nentry->kind = VARIABLE;
        insert_symbol(scope, name, nentry);
    }

}

void process_assignment(struct tree *node, SymbolTable scope)
{
    struct token *var = node->kids[0]->leaf;
    // for (int i = 0; i < node->nkids; i++)
    // {
    //     if (node->kids[i] != NULL && node->kids[i]->leaf != NULL)
    //     {
    //         printf("%s\n", node->kids[i]->leaf->text);
    //     }
    // }
    // printf("%s\n", var->text);
    SymbolTableEntry entry = find_symbol(scope, var->text);

    // const val is redeclared
    if (entry->kind == CONSTANT)
    {
        semantic_error(
            VAR_REDECL,
            var->lineno,
            var->filename,
            var->text
        );
    }
    //  var is used while undeclared
    else if (entry == NULL)
    {
        semantic_error(
            VAR_UNDECL,
            var->lineno,
            var->filename,
            var->text
        );
    }
}

// void process_expression(struct tree *node, SymbolTable scope)
// {
//     if (strcmp(node->symbolname, "ADD_ASSIGNMENT") == 0)
//     {
//         char *operation = node->kids[1]->leaf->text;
//         SymbolTableEntry entry = find_symbol(scope, node->kids[0]->leaf->text);
//         if (entry != NULL)
//         {
//             switch (entry->type->basetype)
//             {
//             case BOOL_TYPE:
//                 fprintf(stderr, "Semantic Error: Cannot perform '%s' on boolean var '%s'\n", operation, entry->s);
//                 exit(3);
//                 break;
//             }
//             // printf("%d\n", (entry->type->basetype));
//         }
//     }
// }

void extract_symbols(struct tree *node, SymbolTable currentScope, ListSymbolTables list, struct tree *parent)
{
    if (node == NULL)
        return;

    // if (node->prodrule == EXPRESSION_RULE)
    // {
    //     // this block is never accessed, maybe delete
    //     process_expression(node, currentScope);
    // }

    if (node->prodrule == FUNCTIONDECL_RULE)
    {
        SymbolTable funcScope = process_function_declaration(node, currentScope);
        funcScope->function = true;
        funcScope->table_name = extract_fun_name(node);
        add_symbol_table(list, funcScope);
        for (int i = 0; i < node->nkids; i++)
        {
            extract_symbols(node->kids[i], funcScope, list, node);
        }
        return;
    }
    else if (node->prodrule == IMPORTDECL_RULE)
    {
        SymbolTable packageScope = process_import_declaration(node, currentScope, list);

        for (int i = 0; i < node->nkids; i++)
        {
            extract_symbols(node->kids[i], packageScope, list, node);
        }
        return;
    }
    else if (node->prodrule == ASSIGNMENT_RULE)
    {
        process_assignment(node, currentScope);
        return;
    }
    else if (node->prodrule == FUNCTIONCALL_RULE)
    {
        //validate_function_call(node, currentScope);
        return;
    }
    else if (node->prodrule == DECLARATION_RULE)
    {
        process_variable_declaration(node, currentScope);
        return;
    }
    else if (node->prodrule == RETURN_RULE) {
        validate_return_statement(currentScope, node);
        return;
    }
    else if (node->leaf != NULL && node->leaf->category == Identifier)
    {
        insert_symbol(currentScope, node->leaf->text, NULL);
        return;
    } else if (node->prodrule == FORLOOP_RULE) {
        insert_symbol(currentScope, node->kids[1]->kids[1]->kids[0]->leaf->text, create_nentry(node->kids[1]->kids[1]->kids[0]->leaf->text, currentScope, integer_typeptr));
        return;
    }
    for (int i = 0; i < node->nkids; i++)
    {
        extract_symbols(node->kids[i], currentScope, list, node);
    }
}

SymbolTable find_symbol_table(ListSymbolTables list, char *name) {
    while (list != NULL) {
        if (strcmp(list->table->table_name, name) == 0) {
            return list->table;
        }
        list = list->next;
    }
    return NULL;
}

int check_expression(struct tree *node, SymbolTable tab) {
    if (node == NULL) return 0;
    if (node->leaf != NULL) {
        switch (node->leaf->category) {
            case Identifier:
                SymbolTableEntry entry = find_symbol(tab, node->leaf->text);
                if (entry == NULL) {
                    semantic_error(
                        FUNC_UNDECL,
                        node->leaf->lineno,
                        node->leaf->filename,
                        node->leaf->text
                    );
                }
                if (entry->type->basetype == FUNC_TYPE) {
                    // function call
                    return entry->type->u.f.returntype->basetype;
                }
                // var
                return entry->type->basetype;
            case IntegerLiteral:
                return INT_TYPE;
            case FloatLiteral:
                return FLOAT_TYPE;
            case DoubleLiteral:
                return DOUBLE_TYPE;
            case BooleanLiteral:
                return BOOL_TYPE;
            case CharacterLiteral:
                return CHAR_TYPE;
            case StringLiteral:
                return STRING_TYPE;
            case LPAREN:
            case RPAREN:
                printf("can't handle parens yet: lineno %d\n", node->leaf->lineno);
                exit(4);
            default:
                printf("expression err: %s, lineno %d\n", node->leaf->text, node->leaf->lineno);
                exit(4);
            // case ARRAY_INT_TYPE:
            //     printf("expression not supported yet: arra_int_type\n");
            //     exit(4);
            // case ARRAY_STRING_TYPE:
            //     printf("expression not supported yet: array_string_type\n");
            //     exit(4);
            // case FUNC_TYPE:
            //     printf("expression not supported yet: funcs\n");
            //     exit(4);
            // case N_INT_TYPE:
            //     printf("expression not supported yet: n_ints \n");
            //     exit(4);
            // case N_REAL_TYPE:
            //     printf("expression not supported yet: n_reals\n");
            //     exit(4);
            // case N_BOOL_TYPE:
            //     printf("expression not supported yet: n_bools\n");
            //     exit(4);
            // case N_CHAR_TYPE:
            //     printf("expression not supported yet: n_chars\n");
            //     exit(4);
            // case N_STRING_TYPE:
            //     printf("expression not supported yet: n_strings\n");
            //     exit(4);
        }
    }
    // for errors
    int lineno;
    char *filename;
    for (int i = 0; i < node->nkids; i++) {
        if (node->kids[i]->leaf != NULL) {
            lineno = node->kids[i]->leaf->lineno;
            filename = node->kids[i]->leaf->filename;
            break;
        }
    }
    // printf("%d\n", lineno);
    char *expression_type = node->symbolname;
    // parenthesis
    if (strcmp(expression_type, "ParenthesizedExpression") == 0) {
        return check_expression(node->kids[1], tab);
    }
    // unary minus
    if (strcmp(expression_type, "UMINUS") == 0) {
        int numType = check_expression(node->kids[1], tab);
        switch(numType){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
                return numType;
        }
        semantic_error(
            BAD_UMINUS,
            lineno,
            filename,
            "-",
            typeint_to_name(numType)
        );
    }
    // other expressions
    int type1 = check_expression(node->kids[0], tab);
    int type2 = check_expression(node->kids[2], tab);
    if (strcmp(expression_type, "FunctionCall") == 0) {
        validate_function_call(node, tab);
        // get the function identifier node, first kid
        struct tree *func_id_node = node->kids[0];
        if (func_id_node->leaf && func_id_node->leaf->category == Identifier) {
            SymbolTableEntry entry = find_symbol(tab, func_id_node->leaf->text);
            if (entry->type->basetype == FUNC_TYPE) {
                // printf("here?\n");
                return entry->type->u.f.returntype->basetype;
            }
            else {
                printf("debug: failed to get func ret type\n");
            }
        }
    }
    else if (strcmp(expression_type, "ADD") == 0) {
        switch(type1){
            case STRING_TYPE:
            case N_STRING_TYPE:
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
                if (type1 == type2) {
                    return type1;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "SUB") == 0) {
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
                if (type1 == type2) {
                    return type1;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "MULT") == 0) {
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
                if (type1 == type2) {
                    return type1;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "MOD") == 0) {
        return 0;
        // switch(type1){
        //     case INT_TYPE:
        //     case N_INT_TYPE:
        //         if (type1 == type2) {
        //             return type1;
        //         }
        // }
        // semantic_error(
        //     INVALID_OP,
        //     lineno,
        //     filename,
        //     typeint_to_name(type1),
        //     typeint_to_name(type2)
        // );
    }
    else if (strcmp(expression_type, "DIV") == 0) {
        // printf("%d\n", node->kids[2]->id);
        // todo: div by node
        if (node->kids[2]->leaf == NULL) return type1;
        if (strcmp(node->kids[2]->leaf->text, "0") == 0) {
            semantic_error(DIV_ZERO, node->leaf->lineno, node->leaf->filename);
        }
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
                if (type1 == type2) {
                    return type1;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "ELVIS") == 0) {
        int expected_non_null = get_non_null_type(type1);
        if (expected_non_null != -1 && type2 == expected_non_null) {
            return type2;  // Valid Elvis operation, return non-null type
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "DISJ") == 0) {
        switch(type1){
            case BOOL_TYPE:
            case N_BOOL_TYPE:
                if (type1 == type2) {
                    return BOOL_TYPE;
                }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "CONJ") == 0) {
        switch(type1){
            case BOOL_TYPE:
            case N_BOOL_TYPE:
                if (type1 == type2) {
                    return BOOL_TYPE;
                }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "LANGLE") == 0) {
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
            case BOOL_TYPE:
            case N_BOOL_TYPE:
                if (type1 == type2) {
                    return BOOL_TYPE;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "RANGLE") == 0) {
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
            case BOOL_TYPE:
            case N_BOOL_TYPE:
                if (type1 == type2) {
                    return BOOL_TYPE;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "LE") == 0) {
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
            case BOOL_TYPE:
            case N_BOOL_TYPE:
                if (type1 == type2) {
                    return BOOL_TYPE;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "GE") == 0) {
        switch(type1){
            case INT_TYPE:
            case N_INT_TYPE:
            case DOUBLE_TYPE:
            case N_DOUBLE_TYPE:
            case FLOAT_TYPE:
            case N_FLOAT_TYPE:
            case BOOL_TYPE:
            case N_BOOL_TYPE:
                if (type1 == type2) {
                    return BOOL_TYPE;
                }
                // allow mixing Float and Double (returns Double)
                else if ((type1 == FLOAT_TYPE && type2 == DOUBLE_TYPE) ||
                (type1 == DOUBLE_TYPE && type2 == FLOAT_TYPE) ||
                (type1 == N_FLOAT_TYPE && type2 == N_DOUBLE_TYPE) ||
                (type1 == N_DOUBLE_TYPE && type2 == N_FLOAT_TYPE)) {
                return DOUBLE_TYPE;
            }
        }
        semantic_error(
            INVALID_OP,
            lineno,
            filename,
            typeint_to_name(type1),
            typeint_to_name(type2)
        );
    }
    else if (strcmp(expression_type, "NOT_EQ") == 0) {
        if (type1 == type2) return BOOL_TYPE;
        else {
            fprintf(stderr, "Semantic Error: Comparison between two operands of different types\n");
        }
    }
    else if (strcmp(expression_type, "EQEQ") == 0) {
        if (type1 == type2) return BOOL_TYPE;
        else {
            fprintf(stderr, "Semantic Error: Comparison between two operands of different types\n");
        }
    }
    return -1;
}

// void check_functioncall_parameters(SymbolTableEntry function, struct tree *node, SymbolTable tab) {
//     if (find_symbol(tab, node->kids[0]->leaf->text) == NULL) {
//         fprintf(stderr, "Semantic Error: Function %s is undeclared\n", node->kids[0]->leaf->text);
//         exit(3);
//     }
//     if (function->type->u.f.nparams == 0) return;
//     paramlist curr_param = function->type->u.f.parameters;
//     struct tree *func_arg_list = node->kids[2];
//     // int num_args = 0;
//     // for (int i = 0; i < func_arg_list->nkids; i++) {
//     //     if (func_arg_list->kids[i]->leaf != NULL) {
//     //         if (func_arg_list->kids[i]->leaf->category == COMMA) {
//     //             continue;
//     //         }
//     //     }
//     //     num_args++;
//     // }
//     // printf("Num args: %d\tNum params: %d\tNum kids: %d\n", num_args, function->type->u.f.nparams, func_arg_list->nkids);
//     // if (num_args != function->type->u.f.nparams) {
//     //     fprintf(stderr, "Semantic Error: ");
//     //     fprintf(stderr, "Number of arguments to function %s should be %d but got %d arguments.\n", function->s, function->type->u.f.nparams, func_arg_list->nkids / 2);
//     //     exit(3);
//     // }
//     int i = 0;
//     printf("dont think it gets called?\n");
//     while (curr_param != NULL) {
//         switch (func_arg_list->kids[i]->leaf->category) {
//             case Identifier: {
//                 SymbolTableEntry var_arg = find_symbol(tab, func_arg_list->kids[i]->leaf->text);
//                 if (var_arg->type->basetype != curr_param->type->basetype) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(var_arg->type->basetype)
//                     );
//                 }
//                 break;
//             }
//             case IntegerLiteral: {
//                 if (curr_param->type->basetype != INT_TYPE) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(INT_TYPE)
//                     );
//                 }
//                 break;
//             }
//             case DoubleLiteral: {
//                 if (curr_param->type->basetype != DOUBLE_TYPE) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(DOUBLE_TYPE)
//                     );
//                 }
//                 break;
//             }
//             case FloatLiteral: {
//                 if (curr_param->type->basetype != FLOAT_TYPE) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(FLOAT_TYPE)
//                     );
//                 }
//                 break;
//             }
//             case BooleanLiteral: {
//                 if (curr_param->type->basetype != BOOL_TYPE) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(BOOL_TYPE)
//                     );
//                 }
//                 break;
//             }
//             case CharacterLiteral: {
//                 if (curr_param->type->basetype != CharacterLiteral) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(CharacterLiteral)
//                     );
//                 }
//                 break;
//             }
//             case StringLiteral: {
//                 if (curr_param->type->basetype != STRING_TYPE) {
//                     semantic_error(
//                         TYPE_MISMATCH,
//                         func_arg_list->kids[i]->leaf->lineno,
//                         func_arg_list->kids[i]->leaf->filename,
//                         typeint_to_name(curr_param->type->basetype),
//                         typeint_to_name(StringLiteral)
//                     );
//                 }
//                 break;
//             }
//         }
//         i += 2;
//         curr_param = curr_param->next;
//     }
// }

void check_assignment(SymbolTableEntry var, struct tree *node, SymbolTable tab) {
    if (node == NULL) {return;}

    if (node->leaf != NULL) {
        // printf("%s\n", node->leaf->text);
        switch (node->leaf->category) {
            case Identifier: {
                SymbolTableEntry entry = find_symbol(tab, node->leaf->text);
                if (var->type->basetype == entry->type->basetype) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        typeint_to_name(entry->type->basetype)
                    );
                }
            }
            case IntegerLiteral: {
                if (var->type->basetype == INT_TYPE || var->type->basetype == ARRAY_INT_TYPE) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        "Int"
                    );
                }
            }
            case DoubleLiteral: {
                if (var->type->basetype == DOUBLE_TYPE) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        "Double"
                    );
                }
            }
            case FloatLiteral: {
                if (var->type->basetype == FLOAT_TYPE) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        "Float"
                    );
                }
            }
            case CharacterLiteral: {
                if (var->type->basetype == CHAR_TYPE) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        "Char"
                    );
                }
            }
            case BooleanLiteral: {
                if (var->type->basetype == BOOL_TYPE) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        "Boolean"
                    );
                }
            }
            case StringLiteral: {
                if (var->type->basetype == STRING_TYPE || var->type->basetype == ARRAY_STRING_TYPE) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        node->leaf->lineno,
                        node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        "String"
                    );
                }
            }
        }
    }
    else if (node->prodrule == FUNCTIONCALL_RULE) {
        validate_function_call(node, tab);
        SymbolTableEntry function = find_symbol(tab, node->kids[0]->leaf->text);
        // check_functioncall_parameters(function, node, tab);
        if (var->type->basetype == function->type->u.f.returntype->basetype) {
            return;
        } else {
            semantic_error(
                TYPE_MISMATCH,
                node->kids[0]->leaf->lineno,
                node->kids[0]->leaf->filename,
                typeint_to_name(var->type->basetype),
                typeint_to_name(function->type->u.f.returntype->basetype)
            );
        }
    }
    else {
        struct tree *assgn_node = NULL;
        if (node->nkids == 2) {
            assgn_node = node->kids[1];
        } else {
            assgn_node = node;
        }
        if (assgn_node->prodrule == FUNCTIONCALL_RULE) {
            validate_function_call(node->kids[1], tab);
            SymbolTableEntry function = find_symbol(tab, assgn_node->kids[0]->leaf->text);
            // check_functioncall_parameters(function, assgn_node, tab);
            if (var->type->basetype == function->type->u.f.returntype->basetype) {
                return;
            } else {
                semantic_error(
                    TYPE_MISMATCH,
                    assgn_node->kids[0]->leaf->lineno,
                    assgn_node->kids[0]->leaf->filename,
                    typeint_to_name(var->type->basetype),
                    typeint_to_name(function->type->u.f.returntype->basetype)
                );
            }
        }
        if (assgn_node->leaf != NULL) {
            if (!assgn_node->nkids) {
                // printf("%s\n", assgn_node->leaf->text);
                switch (assgn_node->leaf->category) {
                    case Identifier:
                        SymbolTableEntry entry = find_symbol(tab, assgn_node->leaf->text);
                        if (var->type->basetype == entry->type->basetype) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                typeint_to_name(entry->type->basetype)
                            );
                        }
                        break;
                    case IntegerLiteral:
                        if (var->type->basetype == INT_TYPE || var->type->basetype == N_INT_TYPE || var->type->basetype == ARRAY_INT_TYPE) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                "Int"
                            );
                        }
                        break;
                    case DoubleLiteral:
                        if (var->type->basetype == DOUBLE_TYPE || var->type->basetype == N_DOUBLE_TYPE) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                "Double"
                            );
                        }
                        break;
                    case FloatLiteral:
                        if (var->type->basetype == FLOAT_TYPE || var->type->basetype == N_FLOAT_TYPE) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                "Float"
                            );
                        }
                        break;
                    case CharacterLiteral:
                        if (var->type->basetype == CHAR_TYPE || var->type->basetype == N_CHAR_TYPE) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                "Char"
                            );
                        }
                        break;
                    case BooleanLiteral:
                        if (var->type->basetype == BOOL_TYPE || var->type->basetype == N_BOOL_TYPE) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                "Boolean"
                            );
                        }
                        break;
                    case StringLiteral:
                        if (var->type->basetype == STRING_TYPE || var->type->basetype == N_STRING_TYPE) {
                            return;
                        } else {
                            semantic_error(
                                TYPE_MISMATCH,
                                assgn_node->leaf->lineno,
                                assgn_node->leaf->filename,
                                typeint_to_name(var->type->basetype),
                                "String"
                            );
                        }
                        break;
                }
                return;
            }

            // printf("in check assignment\n");
            int result_type = check_expression(assgn_node, tab);
            if (result_type != -1) {
                if (var->type->basetype == result_type) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        assgn_node->leaf->lineno,
                        assgn_node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        typeint_to_name(result_type)
                    );
                }
            }
            else {
                fprintf(stderr, "Semantic Error: Invalid operands for operator %s\n", assgn_node->symbolname);
                exit(3);
            }
        }
        else {
            // printf("assigning via expression\n");
            int result_type = check_expression(assgn_node, tab);
            // printf("done assigning via expression\n");
            if (result_type != -1) {
                if (var->type->basetype == result_type) {
                    return;
                } else {
                    semantic_error(
                        TYPE_MISMATCH,
                        assgn_node->leaf->lineno,
                        assgn_node->leaf->filename,
                        typeint_to_name(var->type->basetype),
                        typeint_to_name(result_type)
                    );
                }
            }
            return;
            // else {
            //     semantic_error(
            //         INVALID_OP,
            //         assgn_node->leaf->lineno,
            //         assgn_node->leaf->filename,
            //         typeint_to_name(var->type->basetype),
            //         typeint_to_name(result_type)

            //     );
            // }
        }
    }
}

void check_symbols(struct tree *node, SymbolTable currentScope, ListSymbolTables list, struct tree *parent) {
    if (node == NULL) return;
    if (node->prodrule == FUNCTIONDECL_RULE) {
        char *func_name = extract_fun_name(node);
        for (int i = 0; i < node->nkids; i++) {
            if (node->kids[i]->prodrule == BLOCK_RULE) {
                check_symbols(node->kids[i], find_symbol_table(list, func_name), list, node);
            }
        }
        return;
    }
    if (node->prodrule == DECLARATION_RULE) {
        SymbolTableEntry variable = find_symbol(currentScope, node->kids[1]->leaf->text);
        struct tree *assigned_to = node->kids[3];
        check_assignment(variable, assigned_to, currentScope);
        return;
    }
    if (node->prodrule == ASSIGNMENT_RULE) {
        SymbolTableEntry variable = find_symbol(currentScope, node->kids[0]->leaf->text);
        if (variable->type->basetype == ARRAY_INT_TYPE || variable->type->basetype == ARRAY_STRING_TYPE) {
            check_assignment(variable, node->kids[5], currentScope);
        }
        else {
            check_assignment(variable, node->kids[2], currentScope);
        }
        return;
    }
    if (node->prodrule == FUNCTIONCALL_RULE) {
        validate_function_call(node, currentScope);
        return;
    }
    if (node->prodrule == EXPRESSION_RULE) {
        // int check = check_expression(node, currentScope);
        // printf("check: %d\n", check);
        if (check_expression(node, currentScope) == -1) {
                // for errors
                int lineno;
                // char *filename;
                for (int i = 0; i < node->nkids; i++) {
                    if (node->kids[i]->leaf != NULL) {
                        lineno = node->kids[i]->leaf->lineno;
                        // filename = node->kids[i]->leaf->filename;
                        break;
                    }
                }
            fprintf(stderr, "Semantic Error: Expression on line %d contains invalid operators\n", lineno);
            exit(3);
        }
        return;
    }
    for (int i = 0; i < node->nkids; i++)
    {
        check_symbols(node->kids[i], currentScope, list, node);
    }
}

void free_symtab(ListSymbolTables head)
{
    ListSymbolTables current = head;
    while (current != NULL)
    {
        ListSymbolTables temp = current;
        current = current->next;

        for (int i = 0; i < temp->table->nBuckets; i++)
        {
            SymbolTableEntry entry = temp->table->tbl[i];
            while (entry != NULL)
            {
                SymbolTableEntry tempEntry = entry;
                entry = entry->next;
                if (tempEntry->type)
                {
                    free_type(tempEntry->type);  // You already fixed this
                }
                free(tempEntry);
            }
        }
        free(temp->table->tbl);
        free(temp->table);
        free(temp);
    }
}


ListSymbolTables create_symtabs(struct tree *node, int print, int free)
{
    ListSymbolTables tables = malloc(sizeof(struct symbol_table_list));
    if (tables == NULL)
    {
        perror("Memory allocation failed");
        exit(4);
    }

    SymbolTable global_tab = mksymtab(NULL);
    global_tab->package = true;
    global_tab->table_name = "global scope";
    global_tab->parent = NULL;
    tables->tab_count = 1;
    tables->table = global_tab;
    tables->next = NULL;

    insert_predefined_symbols(global_tab);
    //insert_symbol(global_tab, "temp", create_nentry("temp", global_tab, unit_typeptr));
    extract_symbols(node, global_tab, tables, node);
    check_symbols(node, global_tab, tables, node);
    if (print) {
        print_tables(tables);
    }
    if (free) {
        free_symtab(tables);
    }
    // check for main func
    if (find_symbol_table(tables, "main") == NULL) semantic_error(
        NO_MAIN,
        0,
        NULL
    );
    return tables;
}