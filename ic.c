#include "ic.h"

extern SymbolTableEntry find_symbol(SymbolTable tab, const char *symbolText);
extern SymbolTable find_symbol_table(ListSymbolTables, char *);
extern char* typeint_to_name(int);
StringTable string_table = {.count = 0, .current_offset = 0};

char *CURRENT_SCOPE_NAME;
char type_hint[64];
int CURRENT_BRANCH_NUM;
int STRING_NUM = 0;

// rewrite output file with .string section
void write_string_section(char *filename) {
    FILE *file = fopen(filename, "r+");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // read the whole file into memory
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(filesize + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }

    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0'; // null terminate

    // find the end of the .string section
    char *string_section_end = strstr(buffer, ".data");
    if (!string_section_end) {
        printf(".data section not found.\n");
        free(buffer);
        fclose(file);
        return;
    }

    // find where to insert (before .data)
    long insert_position = string_section_end - buffer;

    // prepare the new strings to insert
    FILE *temp = tmpfile();
    if (!temp) {
        perror("Failed to create temporary file");
        free(buffer);
        fclose(file);
        return;
    }

    // write up to `.data`
    fwrite(buffer, 1, insert_position, temp);

    // insert each string
    string_section(temp);

    // write the rest of the original file (from ".data" onward)
    fwrite(buffer + insert_position, 1, filesize - insert_position, temp);

    // rewrite the original file
    rewind(file);
    rewind(temp);

    // copy temp back into original file
    char ch;
    while (fread(&ch, 1, 1, temp) == 1) {
        fwrite(&ch, 1, 1, file);
    }

    // truncate leftover content if needed
    fflush(file);
    ftruncate(fileno(file), ftell(file));

    // cleanup
    free(buffer);
    fclose(file);
    fclose(temp);
}

// get type category for println arg
char *handle_println_arg(struct token *arg) {
    char *str_output;
    switch(arg->category){
        case IntegerLiteral:
        case BooleanLiteral:
            str_output = "%d";
            break;
        case DoubleLiteral:
        case FloatLiteral:
            str_output = "%f";
            break;
        case CharacterLiteral:
            str_output = "%c";
            break;
        case StringLiteral:
            return NULL;  // no format string needed
        default:
            printf("debug: unknown category in ic handle_println_arg\n");
            return NULL;
    }
    // update str table
    string_table.entries[string_table.count].data = strdup(str_output);
    string_table.entries[string_table.count].offset = string_table.current_offset;
    char *name = malloc(sizeof(char) * 8);
    sprintf(name, "s%d", STRING_NUM);
    STRING_NUM++;
    string_table.entries[string_table.count].name = name;
    string_table.current_offset += strlen(str_output) + 1;
    string_table.count++;

    // char *name = malloc(8);
    // snprintf(name, 8, "s%d", STRING_NUM);
    // string_table.entries[string_table.count].name = name;
    // STRING_NUM++;

    return str_output;
}

// isolate println arg
char *handle_println(struct tree* t) {
    struct token *arg = NULL;
    if (t->kids[2]->leaf == NULL) {
        // printf("debug: ic println arg is node\n");
        arg = t->kids[2]->kids[0]->leaf;
    }
    else {
        // printf("debug: ic println arg is leaf\n");
        arg = t->kids[2]->leaf;
    }
    if (arg == NULL) {
        printf("debug: unexpected println arg in ic.c\n");
        return NULL;
    }
    return handle_println_arg(arg);
}

void string_section(FILE *out)
{
    fprintf(out, ".string %d\n", string_table.current_offset);
    for (int i = 0; i < string_table.count; i++)
    {
        const char *str = string_table.entries[i].data;
        fprintf(out, "\t%s: ", string_table.entries[i].name);
        for (const char *p = str; *p; ++p)
        {
            if (isprint(*p))
            {
                fputc(*p, out);
            }
            else
            {
                fprintf(out, "\\%03o", (unsigned char)*p);
            }
        }
        fprintf(out, "\\000\t\t; loc: %d\n", string_table.entries[i].offset);
    }
    fprintf(out, "\n");
}

void free_string_table() {
    for (int i = 0; i < string_table.count; i++)
    {
        free(string_table.entries[i].data);
        free(string_table.entries[i].name);
    }
}

void collect_strings(struct tree *t)
{
    if (!t)
        return;

    if (t->leaf &&
        (t->leaf->category == StringLiteral || t->leaf->category == MultilineStringLiteral))
    {
        const char *str = t->leaf->sval;

        if (!str)
            return;
        for (int i = 0; i < string_table.count; i++)
        {
            if (strcmp(string_table.entries[i].data, str) == 0)
                return;
        }

        if (string_table.count < 1000)
        {
            string_table.entries[string_table.count].data = strdup(str);
            string_table.entries[string_table.count].offset = string_table.current_offset;
            string_table.current_offset += 8;

            char *name = malloc(8);
            snprintf(name, 8, "s%d", STRING_NUM);
            string_table.entries[string_table.count].name = name;
            STRING_NUM++;

            string_table.count++;
        }
    }

    for (int i = 0; i < t->nkids; i++)
    {
        collect_strings(t->kids[i]);
    }
}


// traverse symbol tables to get variables for data section
struct data_decl *create_data_decls(ListSymbolTables list)
{
    struct data_decl *head = NULL;
    struct data_decl *tail = NULL;
    while (list != NULL)
    {
        SymbolTable table = list->table;
        for (int i = 0; i < table->nBuckets; i++)
        {
            SymbolTableEntry entry = table->tbl[i];
            while (entry != NULL)
            {
                if (entry->kind == VARIABLE)
                {
                    struct data_decl *decl = gen_decl(typeint_to_name(entry->type->basetype), typeptr_to_size(entry->type), entry->s, entry->memloc);
                    decl->next = NULL;
                    if (!head)
                    {
                        head = decl;
                        tail = decl;
                    }
                    else
                    {
                        tail->next = decl;
                        tail = decl;
                    }
                }
                entry = entry->next;
            }
        }
        list = list->next;
    }
    return head;
}

// print .data to file
void print_data_section(FILE *fp, struct data_decl *decl_list)
{
    fprintf(fp, ".data\n");
    while (decl_list != NULL)
    {
        fprintf(fp, "\tloc:%d\t; text: %s, type: %s, size: %d\n",
                decl_list->memory_location,
                decl_list->text,
                decl_list->data_type,
                decl_list->byte_size);

        decl_list = decl_list->next;
    }
}

void free_data_decls(struct data_decl *head)
{
    while (head)
    {
        struct data_decl *next = head->next;
        free(head);
        head = next;
    }
}

struct instr *create_instr(int opcode, struct addr *dest, struct addr *src1, struct addr *src2)
{
    struct instr *instruction = malloc(sizeof(struct instr));
    instruction->opcode = opcode;
    instruction->dest = NULL;
    instruction->src1 = NULL;
    instruction->src2 = NULL;
    instruction->next = NULL;
    if (dest != NULL)
        instruction->dest = dest;
    if (src1 != NULL)
        instruction->src1 = src1;
    if (src2 != NULL)
        instruction->src2 = src2;
    return instruction;
}

char *trim_quotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        str[len - 1] = '\0';   // remove ending quote
        return str + 1;        // skip starting quote
    }
    return str;  // return unchanged if not quoted
}

// find string in string table, return string name
char *find_string(char *s) {
    s = trim_quotes(s);
    for (int i = 0; i < string_table.count; i++) {
        if (strcmp(s, string_table.entries[i].data) == 0) {
            return string_table.entries[i].name;
        }
    }
    return NULL;
}

struct addr *create_addr(int region, int offset, char *s)
{
    struct addr *naddr = malloc(sizeof(struct addr));
    naddr->region = region;
    if (s != NULL)
    {
        if (region == R_CONST) {
            char *s_name = find_string(s);
            if (s_name != NULL) s = s_name;
        }
        naddr->u.name = strdup(s);
    } else {
        naddr->u.offset = offset;
    }
    return naddr;
}

struct tree *find_child(struct tree *parent, int category)
{
    for (int i = 0; i < parent->nkids; i++)
    {
        if (parent->kids[i]->leaf != NULL)
        {
            if (parent->kids[i]->leaf->category == category)
                return parent->kids[i];
        }
    }
    return NULL;
}

struct tree *find_rule(struct tree *parent, char *s) {
    for (int i = 0; i < parent->nkids; i++) {
        if (parent->kids[i]->leaf == NULL) {
            if (strcmp(parent->kids[i]->symbolname, s) == 0) {
                return parent->kids[i];
            }
        }
    }
    return NULL;
}

void append_instr(struct instr *ics, struct instr *inst)
{
    while (ics->next != NULL)
    {
        ics = ics->next;
    }
    ics->next = inst;
}

// int find_location(struct data_decl *decl_list, char *s) {
//     while (decl_list != NULL) {
//         if (strcmp(decl_list->text, s) == 0) {
//             return decl_list->memory_location;
//         }
//         decl_list = decl_list->next;
//     }
//     return -1;
// }

// SymbolTable find_symbol_table(ListSymbolTables tabs, char *table_name) {
//     while (tabs != NULL) {
//         if (strcmp(tabs->table->table_name, table_name) == 0) {
//             return tabs->table;
//         }
//         tabs = tabs->next;
//     }
//     return NULL;
// }

int find_location(ListSymbolTables tabs, char *func_name, char *symbol_name) {
    SymbolTable func_table = find_symbol_table(tabs, func_name);
    SymbolTableEntry entry = find_symbol(func_table, symbol_name);
    return entry->memloc;
}

char* find_string_name(char *s, bool format_str) {
    if (!format_str) {
        for (int i = 0; i < string_table.count; i++) {
            if (strncmp(string_table.entries[i].data, s + 1, strlen(s) - 2) == 0 &&
                string_table.entries[i].data[strlen(s) - 2] == '\0') {
                    return string_table.entries[i].name;
            }
        }
    } else {
        for (int i = 0; i < string_table.count; i++) {
            if (strcmp(s, string_table.entries[i].data)) {
                    return string_table.entries[i].name;
            }
        }
    }
    return NULL;
}

struct addr* gen_expression(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    if (t == NULL) return NULL;
    
    if (t->leaf != NULL) {
        if (t->leaf->category == Identifier) {
            strcpy(type_hint, typeint_to_name(find_symbol(find_symbol_table(tables, CURRENT_SCOPE_NAME), t->leaf->text)->type->basetype));
            return create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->leaf->text), NULL);
        } else {
            switch (t->leaf->category) {
                case IntegerLiteral:
                    strcpy(type_hint, "Int");
                    break;
                case DoubleLiteral:
                    strcpy(type_hint, "Double");
                    break;
                case BooleanLiteral:
                    strcpy(type_hint, "Boolean");
                    break;
                case StringLiteral:
                    strcpy(type_hint, "String");
                    break;
                case MultilineStringLiteral:
                    strcpy(type_hint, "MultiLineString");
                    break;
                case CharacterLiteral:
                    strcpy(type_hint, "Char");
                    break;
                case NullLiteral:
                    strcpy(type_hint, "Null");
                    break;
                default:
                    strcpy(type_hint, "Unknown");
            }
            return create_addr(R_CONST, -1, t->leaf->text);
        }
    }
    
    struct addr* result = create_addr(R_NAME, -1, "temp");
    struct addr *left, *right;
    
    if (strcmp(t->symbolname, "ADD") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_ADD, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "UMINUS") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_SUB, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "MULT") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_MUL, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "DIV") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_DIV, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "LANGLE") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_BLT, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "RANGLE") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_BGT, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "LE") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_BLE, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "GE") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_BGE, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "NOT_EQ") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_BNE, result, left, right));
        return result;
    } else if (strcmp(t->symbolname, "EQEQ") == 0) {
        left = gen_expression(t->kids[0], ics, tables, labels);
        right = gen_expression(t->kids[2], ics, tables, labels);
        append_instr(ics, create_instr(O_BEQ, result, left, right));
        return result;
    } else {
        for (int i = 0; i < t->nkids; i++) {
            return gen_expression(t->kids[i], ics, tables, labels);
        }
    }
    // should never get here but just in case i guess
    if (result) {
        if (result->region == R_NAME && result->u.name) {
            free(result->u.name);
        }
        free(result);
    }
    return NULL;
}

void gen_return(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    struct addr* result = gen_expression(t->kids[1], ics, tables, labels);
    append_instr(ics, create_instr(O_RET, result, create_addr(R_NONE, -1, type_hint), NULL));
}

void gen_function_decl(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    struct tree* identifier = find_child(t, Identifier);
    if (!identifier) return;
    char* func_name = identifier->leaf->text;
    SymbolTableEntry function_info = find_symbol(tables->table, func_name);
    append_instr(ics, create_instr(D_LABEL, create_addr(R_GLOBAL, -1, func_name), NULL, NULL));
    CURRENT_SCOPE_NAME = func_name;
    for (int i = 0; i < t->nkids; i++) {
        if (t->kids[i]->prodrule == BLOCK_RULE) {
            generate_code(t->kids[i], ics, tables, labels);
            break;
        }
    }
    if (function_info->type->u.f.returntype->basetype == UNIT_TYPE) {
        append_instr(ics, create_instr(O_RET, NULL, NULL, NULL));
    }
    CURRENT_SCOPE_NAME = "global scope";
}

void gen_function_call(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    char *func_name = find_child(t, Identifier)->leaf->text;
    char *format_string = NULL;
    SymbolTableEntry function_info = find_symbol(tables->table, func_name);
    int num_params = function_info->type->u.f.nparams;
    bool println = false;
    if (strcmp(func_name, "println") == 0) {
        // put printf stuff here
        println = true;
        format_string = handle_println(t);
    }

    struct tree *func_arg_list_node = t->kids[2];
    if (func_arg_list_node->prodrule != FUNCARGLIST_RULE) {
        append_instr(ics, create_instr(O_PARM, gen_expression(func_arg_list_node, ics, tables, labels), NULL, NULL));
    } else {
        for (int i = func_arg_list_node->nkids - 1; i >= 0; i -= 2) {
            append_instr(ics, create_instr(O_PARM, gen_expression(func_arg_list_node->kids[i], ics, tables, labels), NULL, NULL));
        }
    }
    if (println && format_string) {
        char *temp_str = malloc(sizeof(char) * (strlen(format_string) + 4));
        temp_str = strcat(temp_str, "\\000");
        //printf("%s\n", temp_str);
        append_instr(ics, create_instr(O_PARM, create_addr(R_STRING, -1, find_string_name(temp_str, true)), NULL, NULL));
        free(temp_str);
    }
    append_instr(ics, create_instr(O_CALL, create_addr(R_LABEL, -1, func_name), create_addr(R_NONE, num_params + 1, NULL), create_addr(R_NONE, num_params * 8, NULL)));
}

void gen_assignment(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    append_instr(ics, create_instr(O_ASN, create_addr(R_LOCAL,find_location(tables, CURRENT_SCOPE_NAME, find_child(t, Identifier)->leaf->text), NULL), gen_expression(t->kids[2], ics, tables, labels), NULL));
}

void gen_declaration(struct tree *t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    if (t->nkids >= 4) {
        if (t->kids[3]->kids[1]->leaf != NULL) {
            if (t->kids[3]->kids[1]->leaf->category != StringLiteral && t->kids[3]->kids[1]->leaf->category != MultilineStringLiteral) {
                append_instr(ics, create_instr(O_ADDR, create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->kids[1]->leaf->text), NULL), create_addr(R_CONST, -1, t->kids[3]->kids[1]->leaf->text), NULL));
            } else {
                append_instr(ics, create_instr(O_ADDR, create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->kids[1]->leaf->text), NULL), create_addr(R_STRING, -1, find_string_name(t->kids[3]->kids[1]->leaf->text, false)), NULL));
            }
        } else {
            gen_expression(t->kids[3]->kids[1], ics, tables, labels);
            append_instr(ics, create_instr(O_ADDR, create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->kids[1]->leaf->text), NULL), create_addr(R_NAME, -1, "temp"), NULL));
        }
    } else {
        append_instr(ics, create_instr(O_ADDR, create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->kids[1]->leaf->text), NULL), NULL, NULL));
    }
}

char* create_label_name() {
    char *s = malloc(sizeof(char) * 32);
    sprintf(s, "label%d", CURRENT_BRANCH_NUM);
    CURRENT_BRANCH_NUM++;
    return s;
}

char* gen_label(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    char *s = create_label_name();
    if (labels) append_instr(labels, create_instr(D_LABEL, create_addr(R_GLOBAL, -1, s), NULL, NULL));
    return s;
}

void gen_branch(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    // if section
    char* label = gen_label(t, ics, tables, labels);
    char *return_label = gen_label(t, ics, tables, NULL);
    struct addr* result = gen_expression(t->kids[1]->kids[1], ics, tables, labels);
    //gen_expression(t->kids[2]->kids[1], labels, tables, labels);
    generate_code(t->kids[2]->kids[1], labels, tables, labels);
    append_instr(labels, create_instr(O_GOTO, create_addr(R_LABEL, -1, return_label), NULL, NULL));
    append_instr(ics, create_instr(O_BIF, result, NULL, NULL));
    append_instr(ics, create_instr(O_GOTO, create_addr(R_LABEL, -1, label), NULL, NULL));
    // else section
    if (t->kids[4]->nkids != 0) {
        label = gen_label(t, ics, tables, labels);
        generate_code(t->kids[4]->kids[1]->kids[1], labels, tables, labels);
        append_instr(labels, create_instr(O_GOTO, create_addr(R_LABEL, -1, return_label), NULL, NULL));
        append_instr(ics, create_instr(O_BNIF, NULL, NULL, NULL));
        append_instr(ics, create_instr(O_GOTO, create_addr(R_LABEL, -1, label), NULL, NULL));
    }
    append_instr(ics, create_instr(D_LABEL, create_addr(R_LABEL, -1, return_label), NULL, NULL));
    free(label);
    free(return_label);
}

void gen_range(struct tree *t, struct instr* ics, ListSymbolTables tables, struct instr *labels){
    append_instr(ics, create_instr(O_BEQ, create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->kids[0]->leaf->text), NULL), gen_expression(t->kids[4], ics, tables, labels), NULL));
}

void gen_while(struct tree *t, struct instr* ics, ListSymbolTables tables, struct instr*labels) {
    char* begin_label = gen_label(t, ics, tables, NULL);
    append_instr(ics, create_instr(D_LABEL, create_addr(R_GLOBAL, -1, begin_label), NULL, NULL));
    struct addr* result = gen_expression(t->kids[1]->kids[1], ics, tables, labels);
    append_instr(ics, create_instr(O_BIF, result, NULL, NULL));
    char *block_label = gen_label(t, ics, tables, labels);
    append_instr(ics, create_instr(O_GOTO, create_addr(R_LABEL, -1, block_label), NULL, NULL));

    generate_code(t->kids[2]->kids[1], labels, tables, labels);
    append_instr(labels, create_instr(O_GOTO, create_addr(R_LABEL, -1, begin_label), NULL, NULL));
    //struct addr* result = gen_expression(t->kids[1]->kids[1], ics, tables, labels);
}

void gen_for(struct tree *t, struct instr* ics, ListSymbolTables tables, struct instr *labels) {
    char* begin_label = gen_label(t, ics, tables, NULL);
    append_instr(ics, create_instr(D_LABEL, create_addr(R_GLOBAL, -1, begin_label), NULL, NULL));

    char *block_label = gen_label(t, ics, tables, labels);
    generate_code(t->kids[2], labels, tables, labels);
    append_instr(labels, create_instr(O_ADD, create_addr(R_LOCAL, find_location(tables, CURRENT_SCOPE_NAME, t->kids[1]->kids[1]->kids[0]->leaf->text), NULL), create_addr(R_CONST, -1, "1"), NULL));
    append_instr(labels, create_instr(O_GOTO, create_addr(R_LABEL, -1, begin_label), NULL, NULL));

    gen_range(t->kids[1]->kids[1], ics, tables, labels);
    append_instr(ics, create_instr(O_GOTO, create_addr(R_LABEL, -1, block_label), NULL, NULL));
}

void generate_code(struct tree* t, struct instr* ics, ListSymbolTables tables, struct instr* labels) {
    if (!t) return;
    switch (t->prodrule) {
        case FUNCTIONDECL_RULE:
            gen_function_decl(t, ics, tables, labels);
            break;
        case FUNCTIONCALL_RULE:
            gen_function_call(t, ics, tables, labels);
            break;
        case EXPRESSION_RULE:
            gen_expression(t, ics, tables, labels);
            break;
        case DECLARATION_RULE:
            gen_declaration(t, ics, tables, labels);
            break;
        case ASSIGNMENT_RULE:
            gen_assignment(t, ics, tables, labels);
            break;
        case RETURN_RULE:
            gen_return(t, ics, tables, labels);
            break;
        case IFSTRUC_RULE:
            gen_branch(t, ics, tables, labels);
            break;
        case WHILELOOP_RULE:
            gen_while(t, ics,tables, labels);
            break;
        case FORLOOP_RULE:
            gen_for(t, ics, tables, labels);
            break;
        default:
            for (int i = 0; i < t->nkids; i++) {
                generate_code(t->kids[i], ics, tables, labels);
            }
            break;
    }
}

void format_operand(char* buffer, struct addr* operand) {
    if (operand->region == R_CONST) {
        sprintf(buffer, "const:%s", operand->u.name);
    } else if (operand->region == R_NONE) {
        sprintf(buffer, "%d", operand->u.offset);
    } else if (operand->region == R_LABEL) {
        sprintf(buffer, "%s", operand->u.name);
    } else if (operand->region == R_NAME) {
        sprintf(buffer, "%s", operand->u.name);
    } else if (operand->region == R_LOCAL) {
        sprintf(buffer, "loc:%d", operand->u.offset);
    } else if (operand->region == R_STRING) {
        sprintf(buffer, "%s", operand->u.name);
    }
    else {
        sprintf(buffer, "loc:%d", operand->u.offset);
    }
}

void format_instruction(struct instr* instruction, FILE *fp, char *opcode) {
    char buffer[256];
    char operand_buffer[64];
    
    sprintf(buffer, "\t%s\t", opcode);
    fwrite(buffer, sizeof(char), strlen(opcode) + 2, fp);
    
    if (instruction->dest) {
        format_operand(operand_buffer, instruction->dest);
        sprintf(buffer, "%s", operand_buffer);
        fwrite(buffer, sizeof(char), strlen(buffer), fp);
    }
    
    if (instruction->src1) {
        if (instruction->opcode == O_RET) {
            sprintf(buffer, "\t;%s", instruction->src1->u.name);
            fwrite(buffer, sizeof(char), strlen(buffer), fp);
        } else {
            format_operand(operand_buffer, instruction->src1);
            sprintf(buffer, ",%s", operand_buffer);
            fwrite(buffer, sizeof(char), strlen(buffer), fp);
        }
    }
    
    if (instruction->src2) {
        format_operand(operand_buffer, instruction->src2);
        sprintf(buffer, ",%s", operand_buffer);
        fwrite(buffer, sizeof(char), strlen(buffer), fp);
    }
    
    fwrite("\n", sizeof(char), 1, fp);
}

void write_instr(FILE *fp, struct instr* ics) {
    char buffer[128];
    while (ics != NULL) {
        // printf("%d\n", ics->opcode);
        switch (ics->opcode) {
            case D_LABEL: {
                sprintf(buffer, "\n%s\n", ics->dest->u.name);
                fwrite(buffer, sizeof(char), strlen(buffer), fp);
                break;
            }
            case O_ASN: {
                format_instruction(ics, fp, "asn");
                break;
            }
            case O_ADD: {
                format_instruction(ics, fp, "add");
                break;
            }
            case O_MUL: {
                format_instruction(ics, fp, "mul");
                break;
            }
            case O_SUB: {
                format_instruction(ics, fp, "sub");
                break;
            }
            case O_DIV: {
                format_instruction(ics, fp, "div");
                break;
            }
            case O_CALL: {
                format_instruction(ics, fp, "call");
                break;
            }
            case O_PARM: {
                format_instruction(ics, fp, "parm");
                break;
            }
            case O_ADDR: {
                format_instruction(ics, fp, "addr");
                break;
            }
            case O_RET: {
                format_instruction(ics, fp, "return");
                break;
            }
            case O_BLT: {
                format_instruction(ics, fp, "lt");
                break;
            }
            case O_BLE: {
                format_instruction(ics, fp, "le");
                break;
            }
            case O_BGT: {
                format_instruction(ics, fp, "gt");
                break;
            }
            case O_BGE: {
                format_instruction(ics, fp, "ge");
                break;
            }
            case O_BIF: {
                format_instruction(ics, fp, "if");
                break;
            }
            case O_BEQ: {
                format_instruction(ics, fp, "eq");
                break;
            }
            case O_BNE: {
                format_instruction(ics, fp, "neq");
                break;
            }
            case O_BNIF: {
                format_instruction(ics, fp, "else");
                break;
            }
            case O_GOTO: {
                format_instruction(ics, fp, "goto");
                break;
            }
        }
        ics = ics->next;
    }
}

void free_addr(struct addr *a) {
    if (!a) return;

    // Free dynamically allocated strings only for regions that use them
    if ((a->region == R_CONST || a->region == R_LABEL ||
         a->region == R_NAME || a->region == R_GLOBAL || a->region == R_STRING) &&
        a->u.name != NULL)
    {
        free(a->u.name);
    }

    free(a);
}

// Very basic pointer deduplication helper
bool addr_freed(struct addr **freed_addrs, int count, struct addr *a) {
    for (int i = 0; i < count; i++) {
        if (freed_addrs[i] == a) return true;
    }
    return false;
}

void free_instr(struct instr *head) {
    struct addr *freed_addrs[1000]; // Simple fixed-size dedup list
    int freed_count = 0;

    while (head) {
        struct instr *next = head->next;

        if (head->dest && !addr_freed(freed_addrs, freed_count, head->dest)) {
            free_addr(head->dest);
            freed_addrs[freed_count++] = head->dest;
        }
        if (head->src1 && !addr_freed(freed_addrs, freed_count, head->src1)) {
            free_addr(head->src1);
            freed_addrs[freed_count++] = head->src1;
        }
        if (head->src2 && !addr_freed(freed_addrs, freed_count, head->src2)) {
            free_addr(head->src2);
            freed_addrs[freed_count++] = head->src2;
        }

        free(head);
        head = next;
    }
}


// create and populate intermediate code file
void create_ic(char *ic_file, ListSymbolTables tables, struct tree *node)
{
    // open ic file
    FILE *fp = fopen(ic_file, "w");
    if (!fp)
    {
        perror("Failed to open file");
        exit(4);
    }

    // .data section
    struct data_decl *decl_list = create_data_decls(tables);
    print_data_section(fp, decl_list);

    // .code section

    collect_strings(node);

    struct instr *ics = create_instr(O_BEGIN, NULL, NULL, NULL);
    struct instr *labels = create_instr(O_BEGIN, NULL, NULL, NULL);
    CURRENT_SCOPE_NAME = "global scope";
    CURRENT_BRANCH_NUM = 0;
    generate_code(node, ics, tables, labels);
    fwrite("\n.code", sizeof(char), 6, fp);
    write_instr(fp, ics);
    write_instr(fp, labels);

    // close file
    fclose(fp);

    // .string section
    write_string_section(ic_file);

    // free memory
    free_instr(ics);
    free_instr(labels);
    free_string_table();
    free_data_decls(decl_list);
    free_symtab(tables);
}