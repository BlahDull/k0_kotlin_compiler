#ifndef TREE_H
#define TREE_H

#include <stdbool.h>
#include "tac.h"

typedef enum {
   TOPLEVEL_RULE,
   IMPORTLIST_RULE,
   IMPORTSECTION_RULE,
   IMPORTDECL_RULE,
   IMPORTNAME_RULE,
   GLOBALVARS_SECTION,
   GLOBALVARSLIST_RULE,
   FUNCTIONSECTION_RULE,
   FUNCTIONLIST_RULE,
   FUNCTIONDECL_RULE,
   FUNCPARAMSECTION_RULE,
   FUNCVALPARAMSLIST_RULE,
   FUNCVALPARAMS_RULE,
   TYPE_RULE,
   BLOCK_RULE,
   STATEMENTS_RULE,
   STATEMENT_RULE,
   DECLARATION_RULE,
   ASSIGNMENT_RULE,
   EXPRESSION_RULE,
   FORLOOP_RULE,
   WHILELOOP_RULE,
   CONDITION_RULE,
   RANGE_RULE,
   IFSTRUC_RULE,
   ELSEIFLIST_RULE,
   ELSE_RULE,
   FUNCTIONCALL_RULE,
   FUNCARGLIST_RULE,
   RETURN_RULE,
   CONTROLESTRUC_RULE,
   OPSEMI_RULE,
} ProdRule;

struct tree {
   int prodrule;
   char *symbolname;
   int nkids;
   struct tree *kids[10]; /* if nkids > 0 */
   struct token *leaf;   /* if nkids == 0; NULL for ε productions */
   int id;

   // control flow labels
   struct addr first;
   struct addr follow;
   struct addr onTrue;
   struct addr onFalse;

   // usage flags
   bool has_first;
   bool has_follow;
   bool has_onTrue;
   bool has_onFalse;
};

extern int serial; // for tree id

struct token {
   int category;     /* the integer code returned by yylex */
   char *text;     /* the actual string (lexeme) matched */
   int lineno;     /* the line number on which the token occurs */
   char *filename; /* the source file in which the token occurs */
   int ival;       /* for integer constants, store binary value here */
   double dval;    /* for real constants, store binary value here */
   char *sval;     /* for string constants, malloc space, de-escape, store */
                  /*    the string (less quotes and after escapes) here */
};

int alctoken(int category, char *text, int lineno, char *current_file);
struct tree* alctree(int prodrule, char *symbolname, int nkids, ...);
void free_token(struct token *tok);
void free_tree(struct tree *node);
void print_tree(struct tree *node, int depth);

#endif
