#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "k0gram.h"

extern YYSTYPE yylval;
extern const char *token_name(int t);
int serial = 0;

char *escape(char *s)
{
    size_t len = strlen(s);
    char *ns = malloc(len * 4 + 1);
    char *ns_ptr = ns;
    const char *start = s;
    while (*start)
    {
        if (strncmp(start, "\\t", 2) == 0)
        {
            strcpy(ns_ptr, "    ");
            ns_ptr += 4;
            start += 2;
        }
        else if (strncmp(start, "\\\"", 2) == 0)
        {
            strcpy(ns_ptr, "\"");
            ns_ptr += 1;
            start += 2;
        }
        else
        {
            *ns_ptr++ = *start++;
        }
    }
    *ns_ptr = '\0';
    return ns;
}

// create leaf/token
int alctoken(int category, char *text, int lineno, char *current_file)
{
    // for lexer mode
    if (current_file == NULL) {
        return category;
    }

    // NL tokens are needed for grammar but not the tree
    else if (category == NL) {
        return category;
    }
    
    yylval.treeptr = malloc(sizeof(struct tree));
    if (!yylval.treeptr)
    {
        fprintf(stderr, "Memory allocation failed for tree node (alctoken).\n");
        exit(4);
    }
    yylval.treeptr->prodrule = category;
    yylval.treeptr->symbolname = NULL; // not needed for terminals
    yylval.treeptr->nkids = 0;         // no children since it's a token
    yylval.treeptr->id = serial;
    serial++;
    yylval.treeptr->leaf = malloc(sizeof(struct token));
    if (!yylval.treeptr->leaf)
    {
        fprintf(stderr, "Memory allocation failed for leaf (alctoken).\n");
        free(yylval.treeptr);
        exit(4);
    }
    yylval.treeptr->leaf->category = category;
    yylval.treeptr->leaf->text = strdup(text);
    yylval.treeptr->leaf->lineno = lineno;
    yylval.treeptr->leaf->filename = strdup(current_file);
    switch (category)
    {
    case IntegerLiteral:
        yylval.treeptr->leaf->ival = atoi(text);
        break;
    case DoubleLiteral:
    case FloatLiteral:
        yylval.treeptr->leaf->dval = atof(text);
        break;
    case StringLiteral:
    {
        char *temp_string = malloc(strlen(text));
        strncpy(temp_string, text + 1, strlen(text) - 2);
        temp_string[strlen(text) - 2] = '\0';
        yylval.treeptr->leaf->sval = escape(temp_string);
        free(temp_string);
        break;
    }
    case MultilineStringLiteral:
    {
        char *temp_string = malloc(strlen(text));
        strncpy(temp_string, text + 1, strlen(text) - 2);
        temp_string[strlen(text) - 2] = '\0';
        yylval.treeptr->leaf->sval = escape(temp_string);
        free(temp_string);
        break;
    }
    }
    return category;
}

// create tree
struct tree *alctree(int prodrule, char *symbolname, int nkids, ...)
{
    // struct tree *node = (struct tree *)malloc(sizeof(struct tree));
    // struct tree *node = malloc(sizeof(struct tree));
    struct tree *node = calloc(1, sizeof(struct tree));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed for tree node\n");
        exit(4);
    }
    node->prodrule = prodrule;
    node->symbolname = strdup(symbolname);
    node->nkids = nkids;
    node->leaf = NULL; // initialize as NULL (only used for leaves)
    node->id = serial;
    serial++;
    // handle variable argument list for child nodes
    va_list args;
    va_start(args, nkids);
    for (int i = 0; i < nkids; i++) {
        if (i >= 10) {
            fprintf(stderr, "Too many children in alctree: max 10 allowed.\n");
            exit(4);
        }
        node->kids[i] = va_arg(args, struct tree *);
    }    
    va_end(args);
    return node;
}

// free leaf
void free_token(struct token *leaf)
{
    if (!leaf)
    {
        return;
    }
    free(leaf->text);
    free(leaf->filename);
    if (leaf->category == StringLiteral || leaf->category == MultilineStringLiteral)
    {
        free(leaf->sval); // Free the allocated sval for string literals
    }
    free(leaf);
}

// free the syntax tree
void free_tree(struct tree *node)
{
    if (!node)
    {
        return;
    }
    // recursively free child nodes
    for (int i = 0; i < node->nkids; i++)
    {
        free_tree(node->kids[i]);
    }
    free(node->symbolname);
    // if (node->leaf->category == StringLiteral || node->leaf->category == MultilineStringLiteral) {
    //     free(node->leaf->sval);
    // }
    free_token(node->leaf);
    free(node);
}

// print the syntax tree
void print_tree(struct tree *node, int depth)
{
    if (node == NULL)
    {
        return;
    }
    // indentation based on depth
    for (int i = 0; i < depth; i++)
    {
        printf("  ");
    }
    if (node->leaf)
    {
        printf("[Leaf] ID: %d, Text: \"%s\", Category: %d\n", node->id, node->leaf->text, node->leaf->category);
    }
    else
    {
        printf("[Node] ID: %d, Symbolname: %s (Prodrule: %d, Children: %d)\n",
               node->id,
               node->symbolname ? node->symbolname : "(null)",
               node->prodrule,
               node->nkids);
    }
    // recursively print child nodes
    for (int i = 0; i < node->nkids; i++)
    {
        print_tree(node->kids[i], depth + 1);
    }
}

char *pretty_print_name(struct tree *t)
{
    char *s2 = malloc(40);
    if (t->leaf == NULL)
    {
        sprintf(s2, "%s#%d", t->symbolname, t->prodrule % 10);
        return s2;
    }
    else
    {
        char *text = escape(t->leaf->text);
        sprintf(s2, "%s:%d", text, t->leaf->category);
        free(text);
        return s2;
    }
}

void print_branch(struct tree *t, FILE *f)
{
    fprintf(f, "N%d [shape=box label=\"%s\"];\n", t->id, pretty_print_name(t));
}

void print_leaf(struct tree *t, FILE *f)
{
    // char * s = yyname(t->leaf->category);
    // const char *s = yysymbol_name(t->leaf->category);
    const char *s = token_name(t->leaf->category);
    fprintf(f, "N%d [shape=box style=dotted label=\" %s \\n ", t->id, s);
    if (t->leaf->category == StringLiteral || t->leaf->category == MultilineStringLiteral)
    {
        char *text = escape(t->leaf->sval);
        fprintf(f, "text = \\\"%s\\\" \\l lineno = %d \\l\"];\n", text,
                t->leaf->lineno);
        free(text);
    }
    else
    {
        char *text = escape(t->leaf->text);
        fprintf(f, "text = %s \\l lineno = %d \\l\"];\n", text,

                t->leaf->lineno);
        free(text);
    }
}

void write_graph(struct tree *t, FILE *f)
{
    if (t->leaf != NULL)
    {
        print_leaf(t, f);
        return;
    }
    print_branch(t, f);
    for (int i = 0; i < t->nkids; i++)
    {
        if (t->kids[i] != NULL)
        {
            fprintf(f, "N%d -> N%d;\n", t->id, t->kids[i]->id);
            write_graph(t->kids[i], f);
        }
        else
        {
            fprintf(f, "N%d -> N%d%d;\n", t->id, t->id, serial);
            fprintf(f, "N%d%d [label=\"%s\"];\n", t->id, serial, "Empty rule");
            serial++;
        }
    }
}

void print_graph(struct tree *t, char *file_name)
{
    FILE *fptr = fopen(file_name, "w");
    if (fptr == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        exit(4);
    }
    fprintf(fptr, "digraph {\n");
    write_graph(t, fptr);
    fprintf(fptr, "}\n");
    fclose(fptr);
}
