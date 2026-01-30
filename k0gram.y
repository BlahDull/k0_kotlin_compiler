%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

extern int yylex();
extern int yylineno;
extern char *yytext;
void yyerror(const char *s);
extern void syntax_error(const char *token, int yychar, int line);
struct tree *root; // root of the syntax tree
%}

%union {
   struct tree *treeptr;
};

%token <treeptr> NL BREAK CONTINUE DO ELSE FOR FUN IF IN RETURN VAL VAR WHEN WHILE IMPORT CONST TYPE ARRAY_TYPE BAD_RW BAD_MODIFIERS DOT COMMA LPAREN RPAREN LSQUARE RSQUARE LCURL RCURL COLON SEMICOLON BAD_PUNC ASSIGNMENT ADD_ASSIGNMENT SUB_ASSIGNMENT ADD SUB MULT DIV MOD INCR DECR EQEQ NOT_EQ LANGLE RANGLE LE GE EQEQEQ NOT_EQEQ CONJ DISJ NOT NOT_NULL_ASSERTION SUBSCRIPT_DOT SAFE_CALL ELVIS NULLABLE RANGE RANGE_UNTIL TYPE_CAST BAD_OPS BAD_TOKEN BooleanLiteral NullLiteral IntegerLiteral DoubleLiteral FloatLiteral CharacterLiteral StringLiteral MultilineStringLiteral Identifier FieldIdentifier ArrayLiteral BinLiteral OctalLiteral UnsignedLiteral RealScientificLiteral InvalidCharacterLiteral

%type <treeptr> program topLevelObjectList importSection importList importDeclaration importName functionSection functionList functionDeclaration funcParamSection funcParamList funcParam typeDeclaration type block statements statement globalVarsSection globalVarsList controlStructure ifStruc controlCondition elseIfList else whileLoop forLoop forCondition range rangeParam returnStatement declaration assignment varDec expression functionCall safeCall funcCallParamList memberAccess eol optionalSemi nl_star

// silence conflicts
%expect 59

// pemdas
%left ADD SUB MULT DIV MOD CONJ DISJ      
%nonassoc EQEQ EQEQEQ NOT_EQ LANGLE LE RANGLE GE    
%right ASSIGNMENT ADD_ASSIGNMENT SUB_ASSIGNMENT UMINUS

// unary minus
// %right UMINUS
// %precedence UMINUS


%%

program:
    topLevelObjectList { root = $1; }
    ;

topLevelObjectList:
    nl_star importSection nl_star globalVarsSection nl_star functionSection nl_star { $$ = alctree(TOPLEVEL_RULE, "TopLevelObjectList", 3, $2, $4, $6); }
    ;

importSection:
    /* empty */ { $$ = alctree(IMPORTSECTION_RULE, "ImportSection", 0); }
    | importList { $$ = alctree(IMPORTSECTION_RULE, "ImportSection", 1, $1); }
    ;

importList:
    importList importDeclaration eol { $$ = alctree(IMPORTLIST_RULE, "ImportList", 3, $1, $2, $3); }
    | importDeclaration eol { $$ = alctree(IMPORTLIST_RULE, "ImportList", 2, $1, $2); }
    ;

importDeclaration:
    IMPORT importName { $$ = alctree(IMPORTDECL_RULE, "ImportDeclaration", 2, $1, $2); }
    ;

importName:
    Identifier { $$ = $1; }
    | importName DOT Identifier { $$ = alctree(IMPORTNAME_RULE, "ImportName", 3, $1, $2, $3); }
    | importName DOT MULT { $$ = alctree(IMPORTNAME_RULE, "ImportName", 3, $1, $2, $3); }
    ;

functionSection:
    /* empty */ { $$ = alctree(FUNCTIONSECTION_RULE, "FunctionSection", 0); }
    | functionList { $$ = alctree(FUNCTIONSECTION_RULE, "FunctionSection", 1, $1); }
    ;

functionList:
    functionList functionDeclaration { $$ = alctree(FUNCTIONLIST_RULE, "FunctionList", 2, $1, $2); }
    | functionDeclaration nl_star { $$ = alctree(FUNCTIONLIST_RULE, "FunctionList", 1, $1); }
    ;

functionDeclaration:
    FUN Identifier LPAREN funcParamSection RPAREN typeDeclaration block { $$ = alctree(FUNCTIONDECL_RULE, "FunctionDeclaration", 7, $1, $2, $3, $4, $5, $6, $7); }
    | FUN Identifier LPAREN funcParamSection RPAREN typeDeclaration ASSIGNMENT expression
    {
        fprintf(stderr, "Syntax Error: Expression-bodied functions are not allowed in k0. Use curly braces.\n");
        syntax_error(0, 0, 0);
    }
    ;

funcParamSection:
    /* empty */ { $$ = alctree(FUNCPARAMSECTION_RULE, "FuncParamSection", 0); }
    | funcParamList { $$ = alctree(FUNCPARAMSECTION_RULE, "FuncParamSection", 1, $1); }
    ;

funcParamList:
    funcParam { $$ = alctree(FUNCVALPARAMSLIST_RULE, "FunctionValueParametersList", 1, $1); }
    | funcParamList COMMA funcParam { $$ = alctree(FUNCVALPARAMSLIST_RULE, "FunctionValueParametersList", 3, $1, $2, $3); }
    ;

funcParam:
    Identifier typeDeclaration { $$ = alctree(FUNCVALPARAMS_RULE, "FunctionValueParameters", 2, $1, $2); }
    ;

typeDeclaration:
    /* empty */ { $$ = alctree(TYPE_RULE, "typeDeclaration", 0); }
    | COLON type { $$ = alctree(TYPE_RULE, "typeDeclaration", 2, $1, $2); }
    | COLON ArrayLiteral { $$ = alctree(TYPE_RULE, "typeDeclaration", 2, $1, $2); }
    ;

type:
    TYPE { $$ = $1; }
    | ARRAY_TYPE { $$ = $1; }
    | NULLABLE { $$ = $1; }
    ;

block:
    nl_star LCURL nl_star statements RCURL nl_star { $$ = alctree(BLOCK_RULE, "Block", 3, $2, $4, $5); }
    | nl_star LCURL nl_star RCURL
    { 
        fprintf(stderr, "Syntax Error: k0 does not support empty blocks (line %d).\n", yylineno);
        syntax_error(0, 0, 0);
    }
    ;

statements:
    statement { $$ = alctree(STATEMENTS_RULE, "Statements", 1, $1); }
    | statements statement { $$ = alctree(STATEMENTS_RULE, "Statements", 2, $1, $2); }
    ;

statement:
    controlStructure { $$ = alctree(STATEMENT_RULE, "ControlStructure", 1, $1); }
    | returnStatement eol { $$ = alctree(STATEMENT_RULE, "ReturnStatement", 2, $1, $2); }
    | declaration eol { $$ = alctree(STATEMENT_RULE, "Declaration", 2, $1, $2); }
    | assignment eol { $$ = alctree(STATEMENT_RULE, "Assignment", 2, $1, $2); }
    | expression eol { $$ = alctree(STATEMENT_RULE, "Expression", 2, $1, $2); }
    ;

globalVarsSection:
    /* empty */ { $$ = alctree(GLOBALVARS_SECTION, "GlobalVarsSection", 0); }
    | globalVarsList { $$ = alctree(GLOBALVARS_SECTION, "GlobalVarsSection", 1, $1); }
    ;

globalVarsList:
    declaration eol { $$ = alctree(GLOBALVARSLIST_RULE, "Declaration", 2, $1, $2); }
    | assignment eol { $$ = alctree(GLOBALVARSLIST_RULE, "Assignment", 2, $1, $2); }
    | globalVarsList declaration eol { $$ = alctree(GLOBALVARSLIST_RULE, "Declaration", 3, $1, $2, $3); }
    | globalVarsList assignment eol { $$ = alctree(GLOBALVARSLIST_RULE, "Declaration", 3, $1, $2, $3); }
    ;

controlStructure:
    ifStruc { $$ = alctree(CONTROLESTRUC_RULE, "IfStruc", 1, $1); }
    | whileLoop { $$ = alctree(CONTROLESTRUC_RULE, "WhileLoop", 1, $1); }
    | forLoop { $$ = alctree(CONTROLESTRUC_RULE, "ForLoop", 1, $1); }
    ;

ifStruc:
    IF controlCondition block elseIfList else { $$ = alctree(IFSTRUC_RULE, "IfStruc", 5, $1, $2, $3, $4, $5); }
    ;

controlCondition:
    LPAREN expression RPAREN { $$ = alctree(CONDITION_RULE, "controlCondition", 3, $1, $2, $3); }
    ;

elseIfList:
    /* empty */ { $$ = alctree(ELSEIFLIST_RULE, "ElseIf", 0); }
    | elseIfList ELSE IF controlCondition block { $$ = alctree(ELSEIFLIST_RULE, "ElseIf", 5, $1, $2, $3, $4, $5); }
    ;

else:
    /* empty */ { $$ = alctree(ELSE_RULE, "Else", 0); }
    | ELSE block { $$ = alctree(ELSE_RULE, "Else", 2, $1, $2); }
    ;

whileLoop:
    WHILE controlCondition block { $$ = alctree(WHILELOOP_RULE, "WhileLoop", 3, $1, $2, $3); }
    ;

forLoop:
    FOR forCondition block { $$ = alctree(FORLOOP_RULE, "ForLoop", 3, $1, $2, $3); }
    ;

forCondition:
    LPAREN range RPAREN { $$ = alctree(CONDITION_RULE, "ForCondition", 3, $1, $2, $3); }
    ;

range:
    rangeParam IN rangeParam RANGE rangeParam { $$ = alctree(RANGE_RULE, "Range", 5, $1, $2, $3, $4, $5); }
    | rangeParam IN rangeParam RANGE_UNTIL rangeParam { $$ = alctree(RANGE_RULE, "Range", 5, $1, $2, $3, $4, $5); }
    | rangeParam IN rangeParam { $$ = alctree(RANGE_RULE, "Range", 3, $1, $2, $3); }
    ;

rangeParam:
    Identifier { $$ = $1; }
    | IntegerLiteral { $$ = $1; }
    ;

returnStatement:
    RETURN expression { $$ = alctree(RETURN_RULE, "Return", 2, $1, $2); }
    | RETURN { $$ = $1; }
    ;

declaration:
    varDec Identifier typeDeclaration { $$ = alctree(DECLARATION_RULE, "Declaration", 3, $1, $2, $3); }
    | varDec Identifier typeDeclaration assignment { $$ = alctree(DECLARATION_RULE, "Declaration", 4, $1, $2, $3, $4); }
    ;

assignment:
    Identifier ASSIGNMENT expression { $$ = alctree(ASSIGNMENT_RULE, "Assignment", 3, $1, $2, $3); }
    | Identifier LSQUARE IntegerLiteral RSQUARE ASSIGNMENT expression { $$ = alctree(ASSIGNMENT_RULE, "Assignment", 6, $1, $2, $3, $4, $5, $6); }
    | ASSIGNMENT expression { $$ = alctree(ASSIGNMENT_RULE, "Assignment", 2, $1, $2); }
    ;

varDec:
    VAR { $$ = $1; }
    | VAL { $$ = $1; }
    ;

expression:
    Identifier { $$ = $1; }
    | DoubleLiteral { $$ = $1; }
    | FloatLiteral { $$ = $1; }
    | CharacterLiteral { $$ = $1; }
    | StringLiteral { $$ = $1; }
    | BooleanLiteral { $$ = $1; }
    | NullLiteral { $$ = $1; }
    | ArrayLiteral { $$ = $1; }
    | IntegerLiteral { $$ = $1; }
    | memberAccess { $$ = $1; }
    | functionCall { $$ = $1; }
    | Identifier INCR { $$ = alctree(EXPRESSION_RULE, "Increment", 2, $1, $2); }
    | Identifier DECR { $$ = alctree(EXPRESSION_RULE, "Decrement", 2, $1, $2); }
    | SUB expression %prec UMINUS { $$ = alctree(EXPRESSION_RULE, "UMINUS", 2, $1, $2); }
    | expression SUB_ASSIGNMENT expression { $$ = alctree(EXPRESSION_RULE, "SUB_ASSIGNMENT", 3, $1, $2, $3); }
    | expression ADD_ASSIGNMENT expression { $$ = alctree(EXPRESSION_RULE, "ADD_ASSIGNMENT", 3, $1, $2, $3); }
    | expression ADD expression { $$ = alctree(EXPRESSION_RULE, "ADD", 3, $1, $2, $3); }
    | expression SUB expression { $$ = alctree(EXPRESSION_RULE, "SUB", 3, $1, $2, $3); }
    | expression MULT expression { $$ = alctree(EXPRESSION_RULE, "MULT", 3, $1, $2, $3); }
    | expression DIV expression { $$ = alctree(EXPRESSION_RULE, "DIV", 3, $1, $2, $3); }
    | expression MOD expression { $$ = alctree(EXPRESSION_RULE, "MOD", 3, $1, $2, $3); }
    | expression LANGLE expression { $$ = alctree(EXPRESSION_RULE, "LANGLE", 3, $1, $2, $3); }
    | expression LE expression { $$ = alctree(EXPRESSION_RULE, "LE", 3, $1, $2, $3); }
    | expression RANGLE expression { $$ = alctree(EXPRESSION_RULE, "RANGLE", 3, $1, $2, $3); }
    | expression GE expression { $$ = alctree(EXPRESSION_RULE, "GE", 3, $1, $2, $3); }
    | expression EQEQ expression { $$ = alctree(EXPRESSION_RULE, "EQEQ", 3, $1, $2, $3); }
    | expression EQEQEQ expression { $$ = alctree(EXPRESSION_RULE, "EQEQEQ", 3, $1, $2, $3); }
    | expression NOT_EQ expression { $$ = alctree(EXPRESSION_RULE, "NOT_EQ", 3, $1, $2, $3); }
    | expression CONJ expression { $$ = alctree(EXPRESSION_RULE, "CONJ", 3, $1, $2, $3); }
    | expression DISJ expression { $$ = alctree(EXPRESSION_RULE, "DISJ", 3, $1, $2, $3); }
    | expression ELVIS expression { $$ = alctree(EXPRESSION_RULE, "ELVIS", 3, $1, $2, $3); }
    | LPAREN expression RPAREN { $$ = alctree(EXPRESSION_RULE, "ParenthesizedExpression", 3, $1, $2, $3); }
    ;

functionCall:
    Identifier LPAREN funcCallParamList RPAREN safeCall { $$ = alctree(FUNCTIONCALL_RULE, "FunctionCall", 5, $1, $2, $3, $4, $5); }
    ;

safeCall:
    /* empty */ { $$ = NULL; }
    | SAFE_CALL { $$ = $1; }
    ;

funcCallParamList:
    /* empty */ { $$ = alctree(FUNCARGLIST_RULE, "FuncArgList", 0); }
    | funcCallParamList COMMA expression { $$ = alctree(FUNCARGLIST_RULE, "FuncArgList", 3, $1, $2, $3); }
    | expression { $$ = $1; }
    ;

memberAccess:
    Identifier DOT Identifier { $$ = alctree(EXPRESSION_RULE, "MemberAccess", 3, $1, $2, $3); }
    | Identifier LSQUARE IntegerLiteral RSQUARE { $$ = alctree(EXPRESSION_RULE, "MemberAccess", 4, $1, $2, $3, $4); }
    /*| Identifier DOT functionCall { $$ = alctree(EXPRESSION_RULE, "MemberAccess", 3, $1, $2, $3); }*/
    ;

eol:
    optionalSemi nl_star { $$ = alctree(OPSEMI_RULE, "OptionalSemi", 1, $1); }
    ;

optionalSemi:
    /* empty */ { $$ = NULL; }
    | SEMICOLON { $$ = $1; }
    ;

nl_star:
    /* empty */ { $$ = NULL; }
    | nl_star NL { $$ = NULL; }
    ;

%%

void yyerror(const char *s) {
    syntax_error(yytext, yychar, yylineno);
}

const char* token_name(int t) {
    return yysymbol_name(YYTRANSLATE(t));
}