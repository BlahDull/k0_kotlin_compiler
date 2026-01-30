/*
 * Three Address Code - skeleton for CS 423
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tac.h"

int labelcounter;

char *regionnames[] = {
    "global",
    "loc",
    "class",
    "lab",
    "const",
    "",
    "none"
};

char *opcodenames[] = {
   "ADD",
   "SUB",
   "MUL",
   "DIV",
   "NEG",
   "ASN",
   "ADDR",
   "LCONT",
   "SCONT",
   "GOTO",
   "BLT",
   "BLE",
   "BGT",
   "BGE",
   "BEQ",
   "BNE",
   "BIF", 
   "BNIF", 
   "PARM", 
   "CALL",
   "RETURN"
};

char *pseudonames[] = {
    "glob",
    "proc",
    "loc", 
    "lab", 
    "end", 
    "prot"
};

char *regionname(int i) {
    return regionnames[i-R_GLOBAL];
}

char *opcodename(int i) {
    return opcodenames[i-O_ADD];
}

char *pseudoname(int i) {
    return pseudonames[i-D_GLOB];
}

// struct addr *genlabel() {
//    struct addr *a = malloc(sizeof(struct addr));
//    a->region = R_LABEL;
//    a->u.offset = labelcounter++;
//    printf("generated a label %d\n", a->u.offset);
//    return a;
// }

struct addr genlabel() {
    struct addr a;
    a.region = R_LABEL;
    a.u.offset = labelcounter++;
    printf("generated a label %d\n", a.u.offset);
    return a;
}

struct instr *gen(int op, struct addr a1, struct addr a2, struct addr a3) {
  struct instr *rv = malloc(sizeof (struct instr));
  if (rv == NULL) {
     fprintf(stderr, "out of memory\n");
     exit(4);
     }
  rv->opcode = op;
  rv->dest = &a1;
  rv->src1 = &a2;
  rv->src2 = &a3;
  rv->next = NULL;
  return rv;
}

struct instr *copylist(struct instr *l) {
   if (l == NULL) return NULL;
   struct instr *lcopy = gen(l->opcode, *l->dest, *l->src1, *l->src2);
   lcopy->next = copylist(l->next);
   return lcopy;
}

struct instr *append(struct instr *l1, struct instr *l2) {
   if (l1 == NULL) return l2;
   struct instr *ltmp = l1;
   while(ltmp->next != NULL) ltmp = ltmp->next;
   ltmp->next = l2;
   return l1;
}

struct instr *concat(struct instr *l1, struct instr *l2) {
   return append(copylist(l1), l2);
}


// create a constant address
struct addr mkconst(int value) {
    struct addr a;
    a.region = R_CONST;
    a.u.offset = value;
    return a;
}

// create a local address (local: offset)
struct addr mklocal(int offset) {
    struct addr a;
    a.region = R_LOCAL;
    a.u.offset = offset;
    return a;
}

// create a string region address
struct addr mkstr(int offset) {
    struct addr a;
    a.region = R_GLOBAL;
    a.u.offset = offset;
    return a;
}

// create a named function address
struct addr mkname(char *name) {
    struct addr a;
    a.region = R_NAME;
    a.u.name = name;
    return a;
}

// print a single address
void printaddr(struct addr a) {
    switch (a.region) {
        case R_CONST:
            printf("const:%d", a.u.offset);
            break;
        case R_LOCAL:
            printf("loc:%d", a.u.offset);
            break;
        case R_LABEL:
            printf("L%d", a.u.offset);
            break;
        case R_NAME:
            printf("%s", a.u.name);
            break;
        case R_GLOBAL:
            printf("str:%d", a.u.offset);
            break;
        default:
            printf("??");
    }
}

struct data_decl *gen_decl(char *data_type, int byte_size, char *text, int memloc) {
    struct data_decl *data = malloc(sizeof(struct data_decl));
    data->data_type = data_type;
    data->text = text;
    data->byte_size = byte_size;
    data->memory_location = memloc;
    data->next = NULL;
    return data;
}

void print_data_declarations(struct data_decl *head) {
    while (head != NULL) {
        printf("%s %d\n", head->data_type, head->byte_size);
        printf("\t%s\n", head->text);
        head = head->next;
    }
}

// print the instruction list
void tacprint(struct instr *code, struct data_decl *data_declarations) {
    print_data_declarations(data_declarations);
    printf(".code\n");
    printf("proc main,0,32\n");
    while (code != NULL) {
        if (code->opcode >= O_ADD && code->opcode <= O_RET) {
            printf("\t%s\t", opcodename(code->opcode));
            printaddr(*code->dest);
            if (code->opcode != O_NEG && code->opcode != O_ASN && code->opcode != O_RET && code->opcode != O_GOTO) {
                printf(", ");
                printaddr(*code->src1);
                printf(", ");
                printaddr(*code->src2);
            } else if (code->opcode == O_NEG || code->opcode == O_ASN || code->opcode == O_RET) {
                printf(", ");
                printaddr(*code->src1);
            }
        } else {
            printf("\t%s\t", pseudoname(code->opcode));
            printaddr(*code->dest);
        }
        printf("\n");
        code = code->next;
    }
}

// int main() {
//     // set up data labels
//     struct data_decl *label = gen_decl(".string", 8, "Variable i is \%d\\000");

//     struct instr *code = NULL, *tmp;
//     // i = 5
//     tmp = gen(O_ASN, mklocal(0), mkconst(5), mklocal(0));
//     code = append(code, tmp);

//     // t1 = i * i (loc:8 = loc:0 * loc:0)
//     tmp = gen(O_MUL, mklocal(8), mklocal(0), mklocal(0));
//     code = append(code, tmp);

//     // t2 = t1 + 1 (loc:16 = loc:8 + const:1)
//     tmp = gen(O_ADD, mklocal(16), mklocal(8), mkconst(1));
//     code = append(code, tmp);

//     // i = t2 (loc:0 = loc:16)
//     tmp = gen(O_ASN, mklocal(0), mklocal(16), mklocal(0));
//     code = append(code, tmp);

//     // PARAM i
//     tmp = gen(O_PARM, mklocal(0), mklocal(0), mklocal(0));
//     code = append(code, tmp);

//     // PARAM string:0 ("Variable i is %d.\0")
//     tmp = gen(O_PARM, mkstr(0), mkstr(0), mkstr(0));
//     code = append(code, tmp);

//     // CALL printf, 2 args -> store result in loc:24
//     tmp = gen(O_CALL, mklocal(24), mkname("printf"), mkconst(2));
//     code = append(code, tmp);

//     // RETURN
//     tmp = gen(O_RET, mklocal(0), mklocal(0), mklocal(0));
//     code = append(code, tmp);

//     // print the TAC
//     tacprint(code, label);

//     return 0;
// }