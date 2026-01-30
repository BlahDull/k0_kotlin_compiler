#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_LINE 256
#define TMP_REG "\%rax"
#define ARG1_REG "\%rdi"
#define ARG2_REG "\%rsi"
#define ARG3_REG "\%rdx"
#define ARG4_REG "\%rcx"
#define O_LABEL 3000
#define O_ADD   3001
#define O_SUB   3002
#define O_MUL   3003
#define O_DIV   3004
#define O_NEG   3005
#define O_ASN   3006
#define O_ADDR  3007
#define O_LCONT 3008
#define O_SCONT 3009
#define O_GOTO  3010
#define O_BLT   3011
#define O_BLE   3012
#define O_BGT   3013
#define O_BGE   3014
#define O_BEQ   3015
#define O_BNE   3016
#define O_BIF   3017
#define O_BNIF  3018
#define O_PARM  3019
#define O_CALL  3020
#define O_RET   3021

#define INT_TYPE 5000
#define DOUBLE_TYPE 5001
#define STRING_TYPE 5002
#define CHAR_TYPE 5003

typedef struct DataEntry {
    int loc;
    char *type;
    struct DataEntry *next;
} DataEntry;

typedef struct IC_OPERAND {
    int op_type;
    bool immediate;
    int i_val;
    double d_val;
    char *name;
} *operand;

typedef struct IC_INSTRUCTION {
    int opcode;
    operand op1;
    operand op2;
    operand op3;
    char *label;
    struct IC_INSTRUCTION* next;
} *instruction;

typedef struct StringEntry {
    char *name;
    char *text;
    struct StringEntry *next;
} StringEntry;