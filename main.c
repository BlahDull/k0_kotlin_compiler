#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "symtab.h"
#include "ic.h"
#include "k0gram.h"
#include "tac2asm.h"

extern int yylex();
extern int yyparse();
extern void yy_scan_string(const char *str);
extern int yylex_destroy();
extern FILE *yyin;
extern int yylineno;
extern char *yytext;
char *current_file = NULL;
extern struct tree *root;
extern void print_graph(struct tree *t, char *file_name);
extern struct symbol_table_list* create_symtabs(struct tree*, int print, int free);
extern void tac2asm(char *);

// for usage
enum ACTION {
    COMPILE_EXECUTABLE = 0,
    ASSEMBLER = 1,
    OBJECT = 2,
    IC = 3,
    SYMBOL_TABLE = 4,
    SYNTAX_TREE = 5,
    DOT_TREE = 6,
    LEXER = 7,
    PRINT_ERRORS = 8
};

// report errors from k0lex.l
void lexical_error(const char *format, const char *token, int line) {
    fprintf(stderr, "Lexical Error: ");
    fprintf(stderr, format, token, line);
    fprintf(stderr, " in file %s.\n", current_file);
    exit(1);
}

// report errors from k0gram.y
void syntax_error(const char *token, int yychar, int line) {
    if (token != 0 && line != 0) {
        fprintf(stderr, "Syntax Error: ");
        fprintf(stderr, "Unexpected token '%s' (code %d) on line %d in file %s.\n", token, yychar, line, current_file);
    }
    exit(2);
}

// usage message for supported use cases
void print_usage() {
    fprintf(stderr, "Usage: ./k0 <input-files.kt>\n");
    fprintf(stderr, "       ./k0 -s <input-files.kt>\n");
    fprintf(stderr, "       ./k0 -c <input-files.kt>\n");
    fprintf(stderr, "       ./k0 -ic <input-files.kt>\n");
    fprintf(stderr, "       ./k0 -symtab <input-file.kt>\n");
    fprintf(stderr, "       ./k0 -tree <input-file.kt>\n");
    fprintf(stderr, "       ./k0 -dot <input-file.kt>\n");
    fprintf(stderr, "       ./k0 -lexer\n");
    fprintf(stderr, "       ./k0 -h\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  NONE        Compile to executable (performs all steps)\n");
    fprintf(stderr, "  -s          Generate assembler (.s file)\n");
    fprintf(stderr, "  -c          Produce object file (.o file)\n");
    fprintf(stderr, "  -ic         Generate intermediate code (.ic file)\n");
    fprintf(stderr, "  -symtab     Print symbol table\n");
    fprintf(stderr, "  -tree       Print syntax tree\n");
    fprintf(stderr, "  -dot        Generate DOT representation of the syntax tree\n");
    fprintf(stderr, "  -lexer      Begin lexer loop (to test tokens)\n");
    fprintf(stderr, "  -h          Display usage message\n");
    exit(4);
}

// add extension to input file if none is given
char* add_extension(const char* filename) {
    size_t len = strlen(filename);
    char* new_filename = malloc(len + 4);
    if (!new_filename) {
        perror("Memory allocation failed");
        exit(4);
    }
    strcpy(new_filename, filename);
    strcat(new_filename, ".kt");
    return new_filename;
}

// check extension of input file; return appropriate filename based on action
char* check_extension(const char* filename, int action) {
    const char* ext = strrchr(filename, '.');
    
    if (!ext) {
        return add_extension(filename);
    }
    
    if (strcmp(ext, ".kt") == 0) {
        return strdup(filename);
    } else {
        if (action == ASSEMBLER && strcmp(ext, ".ic") == 0) {
            return strdup(filename);
        } else if (action == OBJECT && strcmp(ext, ".s") == 0) {
            return strdup(filename);
        } else {
            fprintf(stderr, "Error: Input file must have .kt extension or no extension.\n");
            print_usage();
        }
    }
    
    return NULL;
}

const char* get_token_name(int token) {
    switch (token) {
        case YYEMPTY: return "YYEMPTY";
        case YYEOF: return "YYEOF";
        case YYerror: return "YYerror";
        case YYUNDEF: return "YYUNDEF";
        case BREAK: return "BREAK";
        case CONTINUE: return "CONTINUE";
        case DO: return "DO";
        case ELSE: return "ELSE";
        case FOR: return "FOR";
        case FUN: return "FUN";
        case IF: return "IF";
        case IN: return "IN";
        case RETURN: return "RETURN";
        case VAL: return "VAL";
        case VAR: return "VAR";
        case WHEN: return "WHEN";
        case WHILE: return "WHILE";
        case IMPORT: return "IMPORT";
        case CONST: return "CONST";
        case TYPE: return "TYPE";
        case ARRAY_TYPE: return "ARRAY_TYPE";
        case DOT: return "DOT";
        case COMMA: return "COMMA";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LSQUARE: return "LSQUARE";
        case RSQUARE: return "RSQUARE";
        case LCURL: return "LCURL";
        case RCURL: return "RCURL";
        case COLON: return "COLON";
        case ASSIGNMENT: return "ASSIGNMENT";
        case ADD_ASSIGNMENT: return "ADD_ASSIGNMENT";
        case SUB_ASSIGNMENT: return "SUB_ASSIGNMENT";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MULT: return "MULT";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case INCR: return "INCR";
        case DECR: return "DECR";
        case EQEQ: return "EQEQ";
        case NOT_EQ: return "NOT_EQ";
        case LANGLE: return "LANGLE";
        case RANGLE: return "RANGLE";
        case LE: return "LE";
        case GE: return "GE";
        case EQEQEQ: return "EQEQEQ";
        case NOT_EQEQ: return "NOT_EQEQ";
        case CONJ: return "CONJ";
        case DISJ: return "DISJ";
        case NOT: return "NOT";
        case NOT_NULL_ASSERTION: return "NOT_NULL_ASSERTION";
        case SUBSCRIPT_DOT: return "SUBSCRIPT_DOT";
        case SAFE_CALL: return "SAFE_CALL";
        case ELVIS: return "ELVIS";
        case NULLABLE: return "NULLABLE";
        case RANGE: return "RANGE";
        case RANGE_UNTIL: return "RANGE_UNTIL";
        case TYPE_CAST: return "TYPE_CAST";
        case BooleanLiteral: return "BooleanLiteral";
        case NullLiteral: return "NullLiteral";
        case ArrayLiteral: return "ArrayLiteral";
        case IntegerLiteral: return "IntegerLiteral";
        case DoubleLiteral: return "DoubleLiteral";
        case FloatLiteral: return "FloatLiteral";
        case CharacterLiteral: return "CharacterLiteral";
        case StringLiteral: return "StringLiteral";
        case MultilineStringLiteral: return "MultilineStringLiteral";
        case Identifier: return "Identifier";
        case FieldIdentifier: return "FieldIdentifier";
        default: return "UNKNOWN_TOKEN";
    }
}

void lexer_loop() {
    char input[256];
    current_file = NULL;
    printf("Lexer Loop - Enter text to tokenize (type 'exit' to quit)\n");
    while (1) {
        printf("\n>>> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;  // stop on input error
        }
        input[strcspn(input, "\n")] = 0;
        // exit condition
        if (strcmp(input, "exit") == 0) {
            printf("Exiting lexer...\n");
            break;
        }
        yy_scan_string(input);
        int token;
        while ((token = yylex()) != 0) {
            printf("Token: %s, Text: %s\n", get_token_name(token), yytext);
        }
    }
    free_tree(root);
    yylex_destroy();
    free(current_file);
    exit(0);
}

char* generate_ic(struct tree* ast_root, char* source_file) {
    ListSymbolTables tables = create_symtabs(ast_root, 0, 0);
    
    char* ic_file = malloc(strlen(source_file) + 1);
    if (!ic_file) {
        perror("Memory allocation failed");
        exit(4);
    }
    
    strcpy(ic_file, source_file);
    char* ext = strrchr(ic_file, '.');
    if (ext) {
        ext[1] = 'i';
        ext[2] = 'c';
        ext[3] = '\0';
    } else {
        free(ic_file);
        ic_file = malloc(strlen(source_file) + 4);
        if (!ic_file) {
            perror("Memory allocation failed");
            exit(4);
        }
        sprintf(ic_file, "%s.ic", source_file);
    }
    
    printf("Generating intermediate code: %s\n", ic_file);
    create_ic(ic_file, tables, ast_root);
    
    return ic_file;
}

char* generate_asm(char* ic_file) {
    char* asm_file = malloc(strlen(ic_file) + 1);
    if (!asm_file) {
        perror("Memory allocation failed");
        exit(4);
    }
    
    strcpy(asm_file, ic_file);
    char* ext = strrchr(asm_file, '.');
    if (ext) {
        ext[1] = 'S';
        ext[2] = '\0';
    } else {
        free(asm_file);
        asm_file = malloc(strlen(ic_file) + 3);
        if (!asm_file) {
            perror("Memory allocation failed");
            exit(4);
        }
        sprintf(asm_file, "%s.s", ic_file);
    }
    
    printf("Generating assembly code: %s\n", asm_file);
    tac2asm(ic_file);
    
    return asm_file;
}

char* generate_obj(char* asm_file) {
    char* obj_file = malloc(strlen(asm_file) + 1);
    if (!obj_file) {
        perror("Memory allocation failed");
        exit(4);
    }
    
    strcpy(obj_file, asm_file);
    char* ext = strrchr(obj_file, '.');
    if (ext) {
        ext[1] = 'o';
        ext[2] = '\0';
    } else {
        free(obj_file);
        obj_file = malloc(strlen(asm_file) + 3);
        if (!obj_file) {
            perror("Memory allocation failed");
            exit(4);
        }
        sprintf(obj_file, "%s.o", asm_file);
    }
    
    printf("Generating object file: %s\n", obj_file);
    
    char cmd_buffer[512];
    sprintf(cmd_buffer, "gcc -c -o %s %s", obj_file, asm_file);
    
    int result = system(cmd_buffer);
    if (result != 0) {
        fprintf(stderr, "Error: Assembling failed\n");
        exit(4);
    }
    
    return obj_file;
}

void generate_executable(char* obj_file) {
    char* base_name = strdup(obj_file);
    char* ext = strrchr(base_name, '.');
    if (ext) {
        *ext = '\0';
    }
    
    char* slash = strrchr(base_name, '/');
    char* exe_file = slash ? slash + 1 : base_name;
    
    printf("Generating executable: %s\n", exe_file);
    
    char cmd_buffer[512];
    sprintf(cmd_buffer, "gcc -no-pie -o %s %s -lm", exe_file, obj_file);
    
    int result = system(cmd_buffer);
    if (result != 0) {
        fprintf(stderr, "Error: Linking failed\n");
        free(base_name);
        exit(4);
    }
    
    printf("Successfully generated executable: %s\n", exe_file);
    free(base_name);
}

void process_source_file(int action) {
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed for file: %s\n", current_file);
        if (yylval.treeptr) {
            free_tree(yylval.treeptr);
            yylval.treeptr = NULL;
        }
        exit(2);
    }
    
    switch (action) {
        case SYMBOL_TABLE:
            create_symtabs(root, 1, 1);
            break;
            
        case SYNTAX_TREE:
            printf("Syntax Tree:\n");
            print_tree(root, 0);
            break;
            
        case DOT_TREE:
            print_graph(root, "tree.dot");
            if (system("dot -Tpng tree.dot -o tree.png") == 0) {
                printf("Successfully generated tree.png\n");
            } else {
                printf("Error: Failed to execute dot command. Please ensure GraphViz is installed.\n");
                printf("You can run the command manually: dot -Tpng tree.dot -o tree.png\n");
            }
            break;
            
        case PRINT_ERRORS:
            create_symtabs(root, 0, 1);
            printf("No errors found.\n");
            break;
            
        case IC:
        case ASSEMBLER:
        case OBJECT:
        case COMPILE_EXECUTABLE:
            {
                char* ic_file = generate_ic(root, current_file);
                
                char* asm_file = NULL;
                if (action != IC) {
                    asm_file = generate_asm(ic_file);
                    free(ic_file);
                }
                
                char* obj_file = NULL;
                if (action != IC && action != ASSEMBLER) {
                    obj_file = generate_obj(asm_file);
                    free(asm_file);
                }
                
                if (action == COMPILE_EXECUTABLE) {
                    generate_executable(obj_file);
                    free(obj_file);
                }
            }
            break;
    }
}

int main(int argc, char *argv[]) {
    int file_arg_num;
    int action = COMPILE_EXECUTABLE;

    if (argc < 2) {
        print_usage();
    }
    

    if (argc == 2 && (strcmp(argv[1], "-lexer") == 0)) {
        lexer_loop();
    }
    else if (argc == 2 && (strcmp(argv[1], "-h") == 0)) {
        print_usage();
    }
    
    // Parse action flags
    if (argc >= 3 && argv[1][0] == '-') {
        if (strcmp(argv[1], "-dot") == 0) {
            action = DOT_TREE;
        }
        else if (strcmp(argv[1], "-tree") == 0) {
            action = SYNTAX_TREE;
        }
        else if (strcmp(argv[1], "-symtab") == 0) {
            action = SYMBOL_TABLE;
        }
        else if (strcmp(argv[1], "-ic") == 0) {
            action = IC;
        }
        else if (strcmp(argv[1], "-s") == 0) {
            action = ASSEMBLER;
        }
        else if (strcmp(argv[1], "-c") == 0) {
            action = OBJECT;
        }
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[1]);
            print_usage();
        }
        file_arg_num = 2;
    } else {
        file_arg_num = 1;
    }
    
    if (file_arg_num >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage();
    }

    current_file = check_extension(argv[file_arg_num], action);
    
    yyin = fopen(current_file, "r");
    if (!yyin) {
        perror("Error opening file");
        free(current_file);
        exit(4);
    }
    
    process_source_file(action);
    
    free_tree(root);
    fclose(yyin);
    yylex_destroy();
    free(current_file);
    
    return 0;
}