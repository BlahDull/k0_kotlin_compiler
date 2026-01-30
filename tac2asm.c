#include "tac2asm.h"

int CURR_PARM_NUM = 0;
int TOTAL_PARMS = -1;
int LOCAL_VAR_REG_NUM = 12;
DataEntry *data_head = NULL;
StringEntry *string_head = NULL;

FILE *open_file(char *file_name, char *mode) {
    FILE *fp = fopen(file_name, mode);
    if (!fp) {
        perror("tac2asm: Failed to open file");
        exit(4);
    }
    return fp;
}

char *gen_file_name(char *file_name) {
    size_t len = strlen(file_name);
    file_name[len - 2] = 'S';
    file_name[len - 1] = '\0';
    return file_name;
}

void read_line(FILE *ic, char *line) {
    fgets(line, MAX_LINE, ic);
}

DataEntry *find_data_entry_by_loc(int loc) {
    DataEntry *curr = data_head;
    while (curr) {
        if (curr->loc == loc) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void read_data_section(FILE *ic) {
    char line[MAX_LINE];
    int in_data_section = 0;
    DataEntry *tail = NULL;

    rewind(ic);

    while (fgets(line, sizeof(line), ic)) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0') continue;
        if (!in_data_section) {
            if (strcmp(line, ".data") == 0) {
                in_data_section = 1;
            }
            continue;
        }
        if (line[0] != '\0' && line[0] == '.') {
            break;
        }
        int loc, size;
        char text[64], type[16];
        int matched = sscanf(line, " loc:%d ; text: %63[^,], type: %15[^,], size: %d",
                             &loc, text, type, &size);
        if (matched == 4) {
            DataEntry *entry = malloc(sizeof(DataEntry));
            entry->loc = loc;
            entry->type = type;
            entry->next = NULL;

            if (!data_head) {
                data_head = tail = entry;
            }
            else {
                tail->next = entry;
                tail = entry;
            }
        }
    }
}

void free_data_entries() {
    DataEntry *curr = data_head;
    while (curr != NULL) {
        DataEntry *next = curr->next;
        free(curr);
        curr = next;
    }
}

void free_string_entries() {
    StringEntry *curr = string_head;
    while (curr != NULL) {
        StringEntry *next = curr->next;
        free(curr->name);
        free(curr->text);
        free(curr);
        curr = next;
    }
}

// Frees a single operand
void free_operand(operand op) {
    if (!op) return;
    if (op->name) free(op->name);
    free(op);
}

// Frees a linked list of instructions and their operands
void free_instruction_list(instruction head) {
    instruction curr = head;
    while (curr) {
        instruction next = curr->next;
        if (curr->op1) free_operand(curr->op1);
        if (curr->op2) free_operand(curr->op2);
        if (curr->op3) free_operand(curr->op3);
        if (curr->label) free(curr->label);
        free(curr);
        curr = next;
    }
}


void write_strings(FILE *of) {
    StringEntry *curr = string_head;
    while (curr != NULL) {
        fprintf(of, "%s:\n", curr->name);
        fprintf(of, "\t.asciz\t\"%s\\n\"\n", curr->text);
        curr = curr->next;
    }
}

char *find_local_register() {
    switch (LOCAL_VAR_REG_NUM) {
        case 12:
            return "\%r12";
        case 13:
            return "\%r13";
        case 14:
            return "\%r14";
        case 15:
            return "\%r15";
    }
    return "(rsp)";
}

void read_string_section(FILE *ic) {
    char line[MAX_LINE];
    StringEntry *tail = NULL;

    rewind(ic);
    fgets(line, sizeof(line), ic);

    while (fgets(line, sizeof(line), ic)) {
        if (line[0] != '\0' && line[0] == '\n') {
            break;
        }
        char name[4];
        char text[MAX_LINE];
        sscanf(line, " %31[^:]: %255[^\t]", name, text);

        StringEntry *entry = malloc(sizeof(StringEntry));
        entry->name = strdup(name);
        text[strlen(text) - 4] = '\0';
        entry->text = strdup(text);
        entry->next = NULL;

        if (!string_head) {
            string_head = tail = entry;
        }
        else {
            tail->next = entry;
            tail = entry;
        }
    }
}

int get_opcode_from_string(const char *str) {
    if (str == NULL) {
        return -1;
    }
    
    if (strcmp(str, "asn") == 0) {
        return O_ASN;
    } else if (strcmp(str, "add") == 0) {
        return O_ADD;
    } else if (strcmp(str, "mul") == 0) {
        return O_MUL;
    } else if (strcmp(str, "sub") == 0) {
        return O_SUB;
    } else if (strcmp(str, "div") == 0) {
        return O_DIV;
    } else if (strcmp(str, "call") == 0) {
        return O_CALL;
    } else if (strcmp(str, "parm") == 0) {
        return O_PARM;
    } else if (strcmp(str, "addr") == 0) {
        return O_ADDR;
    } else if (strcmp(str, "return") == 0) {
        return O_RET;
    } else if (strcmp(str, "lt") == 0) {
        return O_BLT;
    } else if (strcmp(str, "le") == 0) {
        return O_BLE;
    } else if (strcmp(str, "gt") == 0) {
        return O_BGT;
    } else if (strcmp(str, "ge") == 0) {
        return O_BGE;
    } else if (strcmp(str, "if") == 0) {
        return O_BIF;
    } else if (strcmp(str, "eq") == 0) {
        return O_BEQ;
    } else if (strcmp(str, "neq") == 0) {
        return O_BNE;
    } else if (strcmp(str, "else") == 0) {
        return O_BNIF;
    } else if (strcmp(str, "goto") == 0) {
        return O_GOTO;
    } else {
        return O_LABEL;
    }
}

operand create_operand(char *s) {
    if (s == NULL || strlen(s) == 0) return NULL;
    operand noperand = malloc(sizeof(struct IC_OPERAND));
    memset(noperand, 0, sizeof(struct IC_OPERAND));
    noperand->name = NULL;
    char *loc = NULL;
    if ((loc = strstr(s, "const:")) != NULL) {
        noperand->immediate = true;
        loc += 6;
        
        if (loc[0] == 's') {
            noperand->name = strdup(loc);
            noperand->op_type = STRING_TYPE;
        } else if (strchr(loc, '.') != NULL) {
            noperand->d_val = atof(loc);
            noperand->op_type = DOUBLE_TYPE;
        } else if (isdigit(*loc) || *loc == '-') {
            noperand->i_val = atoi(loc);
            noperand->op_type = INT_TYPE;
        } else {
            noperand->name = strdup(loc);
            noperand->op_type = STRING_TYPE;
        }
    } else if (s[0] == 's') {
        noperand->immediate = false;
        noperand->name = strdup(s);
        noperand->op_type = STRING_TYPE;
    } else {
        noperand->immediate = false;
        noperand->name = strdup(s);
        noperand->op_type = INT_TYPE;
    }
    
    return noperand;
}

instruction create_instruction(char *opcode, char *o1, char *o2, char *o3) {
    instruction ninst = malloc(sizeof(struct IC_INSTRUCTION));
    memset(ninst, 0, sizeof(struct IC_INSTRUCTION));
    
    if (opcode == NULL || strcmp(opcode, "label") == 0) {
        ninst->opcode = O_LABEL;
        return ninst;
    }
    
    ninst->op1 = o1 ? create_operand(o1) : NULL;
    ninst->op2 = o2 ? create_operand(o2) : NULL;
    ninst->op3 = o3 ? create_operand(o3) : NULL;
    ninst->opcode = get_opcode_from_string(opcode);
    ninst->next = NULL;
    return ninst;
}

instruction process_line(char *line) {
    if (line == NULL || line[0] == '\0' || line[0] == '\n') {
        return NULL;
    }
    
    char opcode[32] = {'\0'};
    char operand1[64] = {'\0'};
    char operand2[64] = {'\0'};
    char operand3[64] = {'\0'};
    
    instruction ninst = NULL;
    
    if (line[0] != '\t') {
        char label[64];
        sscanf(line, "%s", label);
        ninst = create_instruction("label", NULL, NULL, NULL);
        ninst->label = strdup(label);
        return ninst;
    }
    
    int res = sscanf(line, "\t%s\t%[^,\n],%[^,\n],%[^,\n]", opcode, operand1, operand2, operand3);
    if (res < 1) {
        return NULL;
    }
    
    if (strcmp("call", opcode) == 0 && strcmp(operand1, "println") == 0) {
        strcpy(operand1, "printf");
    }
    // if (strcmp("addr", opcode) == 0 && (operand1[0] == 's' || operand2[0] == 's')) {
    //     return NULL;
    // }

    return create_instruction(opcode, operand1, operand2, operand3);
}

void format_operands(FILE *S, operand op, bool as_source) {
    if (op == NULL) {
        fprintf(S, "NULL");
        return;
    }
    
    if (op->name) {
        if (strstr(op->name, "loc")) {
            fprintf(S, "%s", find_local_register());
            return;
        }
        if (strcmp(op->name, "temp") == 0) {
            fprintf(S, "%%rax");
            return;
        }
    }

    if (op->immediate) {
        if (op->op_type == INT_TYPE) {
            fprintf(S, "$%d", op->i_val);
        } else if (op->op_type == DOUBLE_TYPE) {
            fprintf(S, "$%lf", op->d_val);
        } else if (op->op_type == STRING_TYPE) {
            fprintf(S, "%s", op->name);
        }
    } else {
        if (as_source) {
            fprintf(S, "%s", op->name);
        } else {
            fprintf(S, "%s", op->name);
        }
    }
}

const char* get_param_register(int param_num) {
    switch (TOTAL_PARMS) {
        case 1: TOTAL_PARMS--; return ARG1_REG;
        case 2: TOTAL_PARMS--; return ARG2_REG;
        case 3: TOTAL_PARMS--; return ARG3_REG;
        case 4: TOTAL_PARMS--; return ARG4_REG;
        default: return NULL;
    }
}

int find_param_count(instruction instr) {
    int count = 0;
    while (instr->opcode != O_CALL) {
        instr = instr->next;
        count++;
    }
    return count;
}

void handle_label_instruction(FILE *S, instruction current) {
    if (current->label) {
        fprintf(S, "%s:\n", current->label);
    }
}

void handle_parameter_instruction(FILE *S, instruction current) {
    if (TOTAL_PARMS == -1) {
        TOTAL_PARMS = find_param_count(current);
    }
    if (CURR_PARM_NUM < 6) {
        const char* reg = get_param_register(CURR_PARM_NUM + 1);
        if (current->op1 && current->op1->op_type == STRING_TYPE) {
            fprintf(S, "\tlea\t");
            format_operands(S, current->op1, true);
            fprintf(S, ", %s\n", reg);
        } else if (current->op2 && current->op2->op_type == STRING_TYPE) {
            fprintf(S, "\tlea\t");
            format_operands(S, current->op2, true);
            fprintf(S, ", %s\n", reg);
        } else if (current->op3 && current->op3->op_type == STRING_TYPE) {
            fprintf(S, "\tlea\t");
            format_operands(S, current->op3, true);
            fprintf(S, ", %s\n", reg);
        } else {
            fprintf(S, "\tmovq\t");
            if (current->op1) {
                format_operands(S, current->op1, true);
                fprintf(S, ", %s\n", reg);
            }
        }
    } else {
        fprintf(S, "\tpushq\t");
        if (current->op1) {
            format_operands(S, current->op1, true);
        }
        fprintf(S, "\n");
    }
}

void handle_call_instruction(FILE *S, instruction current) {
    TOTAL_PARMS = -1;
    if (current->op1) {
        fprintf(S, "\tsubq\t$8, %%rsp\t\t# Align stack\n");
        fprintf(S, "\tcall\t%s\n", current->op1->name);
        fprintf(S, "\taddq\t$8, %%rsp\t\t# Restore stack\n");
    }
}

void handle_return_instruction(FILE *S, instruction current) {
    if (current->op1) {
        fprintf(S, "\tmovq\t");
        format_operands(S, current->op1, true);
        fprintf(S, ", %%rax\n");
    }
    fprintf(S, "\tret\n");
}

void handle_arithmetic_instruction(FILE *S, instruction current, int opcode) {
    if (current->op1 && current->op2 && current->op3) {
        fprintf(S, "\tmovq\t");
        format_operands(S, current->op2, true);
        fprintf(S, ", %%rax\n");
        
        switch (opcode) {
            case O_ADD:
                fprintf(S, "\taddq\t");
                break;
            case O_SUB:
                fprintf(S, "\tsubq\t");
                break;
            case O_MUL:
                fprintf(S, "\timulq\t");
                break;
            case O_DIV:
                fprintf(S, "\tcdq\n");
                fprintf(S, "\tidivq\t");
                break;
        }
        
        format_operands(S, current->op3, true);
        if (opcode != O_DIV) fprintf(S, ", %%rax");
        fprintf(S, "\n");
        
        fprintf(S, "\tmovq\t%%rax, ");
        format_operands(S, current->op1, false);
        fprintf(S, "\n");
    }
}

void handle_assignment_instruction(FILE *S, instruction current) {
    if (current->op1 && current->op2) {
        fprintf(S, "\tmovq\t");
        format_operands(S, current->op2, true);
        fprintf(S, ", %%rax\n");
        
        fprintf(S, "\tmovq\t%%rax, ");
        format_operands(S, current->op1, false);
        fprintf(S, "\n");
    }
}


void handle_address_instruction(FILE *S, instruction current) {
    if (current->op1 && current->op2) {
        if (current->op2->op_type == STRING_TYPE) fprintf(S, "\tleaq\t");
        else fprintf(S, "\tmovq\t");
        format_operands(S, current->op2, false);
        fprintf(S, ", %%rax\n");
        
        fprintf(S, "\tmovq\t%%rax, ");
        //format_operands(S, current->op1, false);
        fprintf(S, "%s", find_local_register(LOCAL_VAR_REG_NUM));
        fprintf(S, "\n");
    }
}

void handle_goto_instruction(FILE *S, instruction current) {
    if (current->op1) {
        fprintf(S, "\tjmp\t%s\n", current->op1->name);
    }
}

void handle_conditional_jump_instruction(FILE *S, instruction current, int opcode) {
    if (opcode == O_BIF && current->op1 && current->op2) {
        fprintf(S, "\tcmpq\t$0, ");
        format_operands(S, current->op1, true);
        fprintf(S, "\n");
        fprintf(S, "\tjne\t%s\n", current->op2->name);
    }
    else if (opcode == O_BNIF && current->op1 && current->op2) {
        fprintf(S, "\tcmpq\t$0, ");
        format_operands(S, current->op1, true);
        fprintf(S, "\n");
        fprintf(S, "\tje\t%s\n", current->op2->name);
    }
}

void handle_comparison_instruction(FILE *S, instruction current, int opcode) {
    if (current->op1 && current->op2 && current->op3) {
        fprintf(S, "\tmovq\t");
        format_operands(S, current->op2, true);
        fprintf(S, ", %%rax\n");
        
        fprintf(S, "\tcmpq\t");
        format_operands(S, current->op3, true);
        fprintf(S, ", %%rax\n");
        
        const char* setcc;
        switch (opcode) {
            case O_BLT: setcc = "setl"; break;
            case O_BLE: setcc = "setle"; break;
            case O_BGT: setcc = "setg"; break;
            case O_BGE: setcc = "setge"; break;
            case O_BEQ: setcc = "sete"; break;
            case O_BNE: setcc = "setne"; break;
            default: setcc = "set"; break;
        }
        
        fprintf(S, "\t%s\t%%al\n", setcc);
        fprintf(S, "\tmovzbq\t%%al, %%rax\n");
        
        fprintf(S, "\tmovq\t%%rax, ");
        format_operands(S, current->op1, false);
        fprintf(S, "\n");
    }
}

void write_instruction(FILE *S, instruction instr) {
    if (instr == NULL) return;
    
    instruction current = instr;
    while (current != NULL) {
        switch (current->opcode) {
            case O_LABEL:
                handle_label_instruction(S, current);
                break;
                
            case O_PARM:
                handle_parameter_instruction(S, current);
                break;
                
            case O_CALL:
                handle_call_instruction(S, current);
                break;
                
            case O_RET:
                handle_return_instruction(S, current);
                break;
                
            case O_ADD:
            case O_SUB:
            case O_MUL:
            case O_DIV:
                handle_arithmetic_instruction(S, current, current->opcode);
                break;
                
            case O_ASN:
                handle_assignment_instruction(S, current);
                break;
                
            case O_ADDR:
                handle_address_instruction(S, current);
                break;
                
            case O_GOTO:
                handle_goto_instruction(S, current);
                break;
                
            case O_BIF:
            case O_BNIF:
                handle_conditional_jump_instruction(S, current, current->opcode);
                break;
                
            case O_BLT:
            case O_BLE:
            case O_BGT:
            case O_BGE:
            case O_BEQ:
            case O_BNE:
                handle_comparison_instruction(S, current, current->opcode);
                break;
                
            default:
                fprintf(S, "\t# Unhandled opcode: %d\n", current->opcode);
                break;
        }
        
        current = current->next;
    }
}

void append_to_list(instruction list, instruction inst) {
    while (list->next != NULL) {
        list = list->next;
    }
    list->next = inst;
}

void text_section(FILE *ics, FILE *S) {
    fprintf(S, ".section .text\n");
    fprintf(S, "\t.global main\n");
    char line[MAX_LINE];
    instruction head = NULL;
    while (!feof(ics)) {
        read_line(ics, line);
        instruction cinstr = process_line(line);
        if (cinstr) {
            if (!head) head = cinstr; else {
                append_to_list(head, cinstr);
            } 
        }
        strcpy(line, "\n");
    }
    write_instruction(S, head);
    free_instruction_list(head);
}

void tac2asm(char *file_name) {
    FILE *ic_file = open_file(file_name, "r");
    char *assembly_file_name = gen_file_name(file_name);
    FILE *assembly_file = open_file(assembly_file_name, "w+");
    
    fprintf(assembly_file, ".section .note.GNU-stack, \"\", @progbits\n");
    
    fprintf(assembly_file, ".section .data\n");
    read_string_section(ic_file);
    write_strings(assembly_file);
    read_data_section(ic_file);
    text_section(ic_file, assembly_file);

    free_string_entries();
    free_data_entries();
    fclose(ic_file);
    fclose(assembly_file);
}