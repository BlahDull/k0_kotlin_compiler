%option noinput
%option nounput
%option yylineno

%{
    #include "k0gram.h"
    #include "tree.h"

    extern void lexical_error(const char *format, const char *token, int line);
    extern char *current_file;
%}

    /* Whitespace and Comments */
LineComment             \/\/.*
DelimitedComment        "/*"([^\*]|\*+[^*/])*\*+"/"
    /* spaces, tabs, newlines, and carriage returns */
WS                      [ \t\r]*
NL                      [\n]
    /* below are rejected */
ShebangLine             ^#!.*


    /* Reserved Words */
BREAK                   "break"
CONTINUE                "continue"
DO                      "do"
ELSE                    "else"
FOR                     "for"
FUN                     "fun"
IF                      "if"
IN                      "in"
RETURN                  "return"
VAL                     "val"
VAR                     "var"
WHEN                    "when"
WHILE                   "while"
IMPORT                  "import"
CONST                   "const"
TYPE                    ("Int"|"Short"|"Byte"|"Long"|"Float"|"Double"|"Boolean"|"String"|"Char")
ARRAY_TYPE              "Array<"{TYPE}">"
    /* below are rejected */
BAD_RW                  ("as"|"as?"|"class"|"!in"|"is"|"!is"|"object"|"package"|"super"|"this"|"throw"|"try"|"typealias"|"typeof"|"by"|"catch"|"constructor"|"delegate"|"dynamic"|"field"|"file"|"finally"|"get"|"init"|"param"|"property"|"receiver"|"set"|"setparam"|"value"|"where")
BAD_MODIFIERS           ("abstract"|"actual"|"annotation"|"companion"|"crossinline"|"data"|"enum"|"expect"|"external"|"final"|"infix"|"inline"|"inner"|"internal"|"lateinit"|"noinline"|"open"|"operator"|"out"|"private"|"protected"|"public"|"reified"|"sealed"|"suspend"|"tailrec"|"vararg")


    /* Operators */
ASSIGNMENT              "="
ADD_ASSIGNMENT          "+="
SUB_ASSIGNMENT          "-="
ADD                     "+"
SUB                     "-"
MULT                    "*"
DIV                     "/"
MOD                     "%"
INCR                    "++"
DECR                    "--"
EQEQ                    "=="
NOT_EQ                  "!="
LANGLE                  "<"
RANGLE                  ">"
LE                      "<="
GE                      ">="
EQEQEQ                  "==="
NOT_EQEQ               "!=="
CONJ                    "&&"
DISJ                    "||"
NOT                     "!"
NOT_NULL_ASSERTION      "!!"
SUBSCRIPT_DOT           "[ ]."
SAFE_CALL               "?."
ELVIS                   "?:"
NULLABLE                {TYPE}"?"
RANGE                   ".."
RANGE_UNTIL             "..<"
TYPE_CAST               "("{TYPE}")"
    /* below are rejected */
BAD_OPS                 ("~"|"<<"|">>"|"&="|"\|="|"\^="|">>="|"<<="|"&"|"\|"|"\^"|"*="|"/="|"%=")


    /* Punctuation */
DOT                     "."
COMMA                   ","
LPAREN                  "("
RPAREN                  ")"
LSQUARE                 "["
RSQUARE                 "]"    
LCURL                   "{"
RCURL                   "}"
COLON                   ":"
SEMICOLON               ";"
    /* below are rejected */
BAD_PUNC                ("#"|"@"|"$"|"\\"|"`"|"::"|";;")


    /* Literals */
BooleanLiteral          ("true"|"false")
NullLiteral             "null"
IntegerLiteral          (((0|[1-9][0-9]*(_?[0-9])*)[lL]?)|(0x[0-9A-Fa-f](_?[0-9A-Fa-f])*[lL]?))
DoubleLiteral           (([0-9]+(\.[0-9]+)?[dD]?)|(\.[0-9]+[dD]?))
FloatLiteral            (([0-9]+(\.[0-9]+)?[fF]?)|(\.[0-9]+[fF]?))
CharacterLiteral        '([^\\]|\\.)'
StringLiteral           \"([^"\\$]*(\\.[^"\\$]*)*(\$[a-zA-Z_][a-zA-Z0-9_]*)?[^"\\]*)*\"
MultilineStringLiteral  \"\"\"([^"\\$]*(\\.[^"\\$]*)*(\$[a-zA-Z_][a-zA-Z0-9_]*)?[^"\\]*)*\"\"\"
Identifier              [_a-zA-Z][_a-zA-Z0-9]*
FieldIdentifier         ^$.{Identifier}
ArrayLiteral            ({ARRAY_TYPE}{WS}{LPAREN}{WS}{IntegerLiteral}{WS}{RPAREN}{WS}{LCURL}{WS}({IntegerLiteral}|{FloatLiteral}|{StringLiteral}){WS}{RCURL}|{ARRAY_TYPE}{WS}{LPAREN}{WS}{IntegerLiteral}{WS}{RPAREN}{WS}{LCURL}{WS}({IntegerLiteral}|{DoubleLiteral}|{StringLiteral}){WS}{RCURL})
    /* below are rejected */
BinLiteral              ((-?0[bB][01](_?[01])*)|(-?0[bB][^01_]*))
OctalLiteral            ((-?0[oO][0-7](_?[0-7])*)|(-?0[0-9]+))
UnsignedLiteral         ((0|[1-9][0-9]*(_?[0-9])*[uU][lL]?)|(0x[0-9A-Fa-f](_?[0-9A-Fa-f])*[uU][lL]?)|(0b[01](_?[01])*[uU][lL]?)|(0o[0-7](_?[0-7])*[uU][lL]?))
RealScientificLiteral   (-?[0-9]+(\.[0-9]+)?[eE][+-]?[0-9]+[dDfF]?)|(-?\.[0-9]+[eE][+-]?[0-9]+[dDfF]?)
InvalidCharacterLiteral '([^\\']{2,}|[^\\]..)'

    /* Anything Else */
UNKNOWN                 .

%%

    /* Whitespace and Comments */
{LineComment}           { /* return alctoken(LineComment, yytext, yylineno, current_file); */ }
{DelimitedComment}      { /* return alctoken(DelimitedComment, yytext, yylineno, current_file); */ }
{WS}                    { /* return alctoken(WS, yytext, yylineno, current_file); */}
{NL}                    { return alctoken(NL, yytext, yylineno, current_file); }
{ShebangLine}           { lexical_error("k0 does not support shebang lines. Found '%s' at line %d", yytext, yylineno); }


    /* Reserved Words */
{BREAK}                   { return alctoken(BREAK, yytext, yylineno, current_file); }
{CONTINUE}                { return alctoken(CONTINUE, yytext, yylineno, current_file); }
{DO}                      { return alctoken(DO, yytext, yylineno, current_file); }
{ELSE}                    { return alctoken(ELSE, yytext, yylineno, current_file); }
{FOR}                     { return alctoken(FOR, yytext, yylineno, current_file); }
{FUN}                     { return alctoken(FUN, yytext, yylineno, current_file); }
{IF}                      { return alctoken(IF, yytext, yylineno, current_file); }
{IN}                      { return alctoken(IN, yytext, yylineno, current_file); }
{RETURN}                  { return alctoken(RETURN, yytext, yylineno, current_file); }
{VAL}                     { return alctoken(VAL, yytext, yylineno, current_file); }
{VAR}                     { return alctoken(VAR, yytext, yylineno, current_file); }
{WHEN}                    { return alctoken(WHEN, yytext, yylineno, current_file); }
{WHILE}                   { return alctoken(WHILE, yytext, yylineno, current_file); }
{IMPORT}                  { return alctoken(IMPORT, yytext, yylineno, current_file); }
{CONST}                   { return alctoken(CONST, yytext, yylineno, current_file); }
{TYPE}                    { return alctoken(TYPE, yytext, yylineno, current_file); }
{ARRAY_TYPE}              { return alctoken(ARRAY_TYPE, yytext, yylineno, current_file); }
{BAD_RW}                  { lexical_error("k0 does not support the following reserved word: found '%s' at line %d", yytext, yylineno); }
{BAD_MODIFIERS}           { lexical_error("k0 does not support the following modifier: found '%s' at line %d", yytext, yylineno); }


    /* Operators */
{ASSIGNMENT}              { return alctoken(ASSIGNMENT, yytext, yylineno, current_file); }
{ADD_ASSIGNMENT}          { return alctoken(ADD_ASSIGNMENT, yytext, yylineno, current_file); }
{SUB_ASSIGNMENT}          { return alctoken(SUB_ASSIGNMENT, yytext, yylineno, current_file); }
{ADD}                     { return alctoken(ADD, yytext, yylineno, current_file); }
{SUB}                     { return alctoken(SUB, yytext, yylineno, current_file); }
{MULT}                    { return alctoken(MULT, yytext, yylineno, current_file); }
{DIV}                     { return alctoken(DIV, yytext, yylineno, current_file); }
{MOD}                     { return alctoken(MOD, yytext, yylineno, current_file); }
{INCR}                    { return alctoken(INCR, yytext, yylineno, current_file); }
{DECR}                    { return alctoken(DECR, yytext, yylineno, current_file); }
{EQEQ}                    { return alctoken(EQEQ, yytext, yylineno, current_file); }
{NOT_EQ}                  { return alctoken(NOT_EQ, yytext, yylineno, current_file); }
{LANGLE}                  { return alctoken(LANGLE, yytext, yylineno, current_file); }
{RANGLE}                  { return alctoken(RANGLE, yytext, yylineno, current_file); }
{LE}                      { return alctoken(LE, yytext, yylineno, current_file); }
{GE}                      { return alctoken(GE, yytext, yylineno, current_file); }
{EQEQEQ}                  { return alctoken(EQEQEQ, yytext, yylineno, current_file); }
{NOT_EQEQ}                { return alctoken(NOT_EQEQ, yytext, yylineno, current_file); }
{CONJ}                    { return alctoken(CONJ, yytext, yylineno, current_file); }
{DISJ}                    { return alctoken(DISJ, yytext, yylineno, current_file); }
{NOT}                     { return alctoken(NOT, yytext, yylineno, current_file); }
{NOT_NULL_ASSERTION}      { return alctoken(NOT_NULL_ASSERTION, yytext, yylineno, current_file); }
{SUBSCRIPT_DOT}           { return alctoken(SUBSCRIPT_DOT, yytext, yylineno, current_file); }
{SAFE_CALL}               { return alctoken(SAFE_CALL, yytext, yylineno, current_file); }
{ELVIS}                   { return alctoken(ELVIS, yytext, yylineno, current_file); }
{NULLABLE}                { return alctoken(NULLABLE, yytext, yylineno, current_file); }
{RANGE}                   { return alctoken(RANGE, yytext, yylineno, current_file); }
{RANGE_UNTIL}             { return alctoken(RANGE_UNTIL, yytext, yylineno, current_file); }
{TYPE_CAST}               { return alctoken(TYPE_CAST, yytext, yylineno, current_file); }
{BAD_OPS}                 { lexical_error("k0 does not support the following operator: found '%s' at line %d", yytext, yylineno); }


    /* Punctuation */
{DOT}                     { return alctoken(DOT, yytext, yylineno, current_file); }
{COMMA}                   { return alctoken(COMMA, yytext, yylineno, current_file); }
{LPAREN}                  { return alctoken(LPAREN, yytext, yylineno, current_file); }
{RPAREN}                  { return alctoken(RPAREN, yytext, yylineno, current_file); }
{LSQUARE}                 { return alctoken(LSQUARE, yytext, yylineno, current_file); }
{RSQUARE}                 { return alctoken(RSQUARE, yytext, yylineno, current_file); }
{LCURL}                   { return alctoken(LCURL, yytext, yylineno, current_file); }
{RCURL}                   { return alctoken(RCURL, yytext, yylineno, current_file); }
{COLON}                   { return alctoken(COLON, yytext, yylineno, current_file); }
{SEMICOLON}               { return alctoken(SEMICOLON, yytext, yylineno, current_file); }
{BAD_PUNC}                { lexical_error("k0 does not support the following punctuation: found '%s' at line %d", yytext, yylineno); }


    /* Literals */
{BooleanLiteral}          { return alctoken(BooleanLiteral, yytext, yylineno, current_file); }
{NullLiteral}             { return alctoken(NullLiteral, yytext, yylineno, current_file); }
{IntegerLiteral}          { return alctoken(IntegerLiteral, yytext, yylineno, current_file); }
{DoubleLiteral}           { return alctoken(DoubleLiteral, yytext, yylineno, current_file); }
{FloatLiteral}            { return alctoken(FloatLiteral, yytext, yylineno, current_file); }
{CharacterLiteral}        { return alctoken(CharacterLiteral, yytext, yylineno, current_file); }
{StringLiteral}           { return alctoken(StringLiteral, yytext, yylineno, current_file); }
{MultilineStringLiteral}  { return alctoken(MultilineStringLiteral, yytext, yylineno, current_file); }
{Identifier}              { return alctoken(Identifier, yytext, yylineno, current_file); }
{FieldIdentifier}         { return alctoken(FieldIdentifier, yytext, yylineno, current_file); }
{ArrayLiteral}            { return alctoken(ArrayLiteral, yytext, yylineno, current_file); }
{BinLiteral}              { lexical_error("k0 does not support binary literals. Found '%s' at line %d", yytext, yylineno); }
{OctalLiteral}            { lexical_error("k0 does not support octal literals. Found '%s' at line %d", yytext, yylineno); }
{UnsignedLiteral}         { lexical_error("k0 does not support unsigned literals. Found '%s' at line %d", yytext, yylineno); }
{RealScientificLiteral}   { lexical_error("k0 does not support the scientific/exponent. Found '%s' at line %d", yytext, yylineno); }
{InvalidCharacterLiteral} { lexical_error("k0 does not support character literals with more than one character. Found '%s' at line %d", yytext, yylineno); }
{UNKNOWN}                 { lexical_error("k0 does not recognize the following token: found '%s' at line %d", yytext, yylineno); }

%%

int yywrap() {
    return 1;
}