#ifndef TYPE_H
#define TYPE_H

#include "tree.h"
#include <stdbool.h>

typedef struct param
{
    char *name;
    struct typeinfo *type;
    struct param *next;
} *paramlist;

struct field
{ /* members (fields) of structs */
    char *name;
    struct type *elemtype;
};

/* base types. How many more base types do we need? */
enum base_types {
    FIRST_TYPE = 1000000,
    NULL_TYPE,
    INT_TYPE,
    DOUBLE_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    CHAR_TYPE,
    STRING_TYPE,
    ARRAY_BASE_TYPE,
    ARRAY_INT_TYPE,
    ARRAY_STRING_TYPE,
    FUNC_TYPE,
    CLASS_TYPE,
    PACKAGE_TYPE,
    ANY_TYPE,
    UNIT_TYPE,
    LAST_TYPE,
    // nullable primities
    N_INT_TYPE,
    N_DOUBLE_TYPE,
    N_FLOAT_TYPE,
    N_BOOL_TYPE,
    N_CHAR_TYPE,
    N_STRING_TYPE,
};

typedef struct typeinfo
{
    int basetype;
    union
    {
        struct funcinfo
        {
            char *name;  /* ? */
            int defined; /* 0 == prototype, 1 == not prototype */
            struct sym_table *st;
            struct typeinfo *returntype;
            int nparams;
            struct param *parameters;
        } f;
        struct arrayinfo
        {
            int size; /* -1 == unspecified/unknown/dontcare */
            struct typeinfo *elemtype;
        } a;
    } u;
} *typeptr;

char *typeint_to_name(int type);
int typeptr_to_size(typeptr t);
int name_to_typeint(char *s);
typeptr alctype(int base);
int get_non_null_type(int nullable_type);
void free_type(typeptr t);
void free_paramlist(paramlist head);
typeptr clone_type(typeptr t);
bool type_is_shared(typeptr t);

#endif
