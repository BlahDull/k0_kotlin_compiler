#include "type.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

struct typeinfo any_type = { ANY_TYPE };
struct typeinfo null_type = { NULL_TYPE };
struct typeinfo integer_type = { INT_TYPE };
struct typeinfo double_type = { DOUBLE_TYPE };
struct typeinfo float_type = { FLOAT_TYPE };
struct typeinfo boolean_type = { BOOL_TYPE };
struct typeinfo char_type = { CHAR_TYPE };
struct typeinfo string_type = { STRING_TYPE };
struct typeinfo unit_type = {UNIT_TYPE};
struct typeinfo array_base_type = { ARRAY_BASE_TYPE };
struct typeinfo array_int_type = { ARRAY_INT_TYPE };
struct typeinfo array_string_type = { ARRAY_STRING_TYPE };
struct typeinfo func_type = { FUNC_TYPE };
// nulls
struct typeinfo n_integer_type = { N_INT_TYPE };
struct typeinfo n_double_type = { N_DOUBLE_TYPE };
struct typeinfo n_float_type = { N_FLOAT_TYPE };
struct typeinfo n_boolean_type = { N_BOOL_TYPE };
struct typeinfo n_char_type = { N_CHAR_TYPE };
struct typeinfo n_string_type = { N_STRING_TYPE };

typeptr any_typeptr = &any_type;
typeptr null_typeptr = &null_type;
typeptr integer_typeptr = &integer_type;
typeptr double_typeptr = &double_type;
typeptr float_typeptr = &float_type;
typeptr boolean_typeptr = &boolean_type;
typeptr char_typeptr = &char_type;
typeptr string_typeptr = &string_type;
typeptr unit_typeptr = &unit_type;
typeptr array_base_typeptr = &array_base_type;
typeptr array_int_typeptr = &array_int_type;
typeptr array_string_typeptr = &array_string_type;
typeptr func_typeptr = &func_type;
// nulls
typeptr n_integer_typeptr = &n_integer_type;
typeptr n_double_typeptr = &n_double_type;
typeptr n_float_typeptr = &n_float_type;
typeptr n_boolean_typeptr = &n_boolean_type;
typeptr n_char_typeptr = &n_char_type;
typeptr n_string_typeptr = &n_string_type;

char *typeint_to_name(int type) {
    switch (type) {
        case FUNC_TYPE: return "Fun";
        case NULL_TYPE: return "Null";
        case INT_TYPE: return "Int";
        case N_INT_TYPE: return "Int?";
        case DOUBLE_TYPE: return "Double";
        case N_DOUBLE_TYPE: return "Double?";
        case FLOAT_TYPE: return "Float";
        case N_FLOAT_TYPE: return "Float?";
        case BOOL_TYPE: return "Boolean";
        case N_BOOL_TYPE: return "Boolean?";
        case STRING_TYPE: return "String";
        case N_STRING_TYPE: return "String?";
        case CHAR_TYPE: return "Char";
        case N_CHAR_TYPE: return "Char?";
        case ARRAY_INT_TYPE: return "Array<Int>";
        case ARRAY_STRING_TYPE: return "Array<String>";
        case ARRAY_BASE_TYPE: return "Array";
        case CLASS_TYPE: return "Class";
        case PACKAGE_TYPE: return "Package";
        case UNIT_TYPE: return "Void";
        case ANY_TYPE: return "Any";
        default: {
            printf("debug: typeint_to_name got Other type: %d\n", type);
            return "Other";
        }
    }
}

int name_to_typeint(char *s) {
    if (strcmp(s, "Int") == 0) return INT_TYPE;
    else if (strcmp(s, "Null") == 0) return NULL_TYPE;
    else if (strcmp(s, "Double") == 0) return DOUBLE_TYPE;
    else if (strcmp(s, "Float") == 0) return FLOAT_TYPE;
    else if (strcmp(s, "String") == 0) return STRING_TYPE;
    else if (strcmp(s, "Boolean") == 0) return BOOL_TYPE;
    else if (strstr(s, "Array<Int>")) return ARRAY_INT_TYPE;
    else if (strstr(s, "Array<String>")) return ARRAY_STRING_TYPE;
    else if (strstr(s, "Char")) return CHAR_TYPE;
    else if (strstr(s, "Fun")) return FUNC_TYPE;
    else if (strstr(s, "Any")) return ANY_TYPE;
    // nullable primitives
    else if (strcmp(s, "Int?") == 0) return N_INT_TYPE;
    else if (strcmp(s, "Double?") == 0) return N_DOUBLE_TYPE;
    else if (strcmp(s, "Float?") == 0) return N_FLOAT_TYPE;
    else if (strcmp(s, "String?") == 0) return N_STRING_TYPE;
    else if (strcmp(s, "Boolean?") == 0) return N_BOOL_TYPE;
    else if (strstr(s, "Char?")) return N_CHAR_TYPE;
    else return -1;
}

typeptr alctype(int base) {
    typeptr rv;
    if (base == NULL_TYPE) return null_typeptr;
    else if (base == ANY_TYPE) return any_typeptr;
    else if (base == INT_TYPE) return integer_typeptr;
    else if (base == DOUBLE_TYPE) return double_typeptr;
    else if (base == FLOAT_TYPE) return float_typeptr;
    else if (base == BOOL_TYPE) return boolean_typeptr;
    else if (base == CHAR_TYPE) return char_typeptr;
    else if (base == STRING_TYPE) return string_typeptr;
    else if (base == UNIT_TYPE) return unit_typeptr;
    else if (base == ARRAY_BASE_TYPE) return array_base_typeptr;
    else if (base == ARRAY_INT_TYPE) return array_int_typeptr;
    else if (base == ARRAY_STRING_TYPE) return array_string_typeptr;
    // nulls
    else if (base == N_INT_TYPE) return n_integer_typeptr;
    else if (base == N_DOUBLE_TYPE) return n_double_typeptr;
    else if (base == N_FLOAT_TYPE) return n_float_typeptr;
    else if (base == N_BOOL_TYPE) return n_boolean_typeptr;
    else if (base == N_CHAR_TYPE) return n_char_typeptr;
    else if (base == N_STRING_TYPE) return n_string_typeptr;
    // else if (base == FUNC_TYPE) return func_typeptr;
    else {
        if (base != FUNC_TYPE ) {
            fprintf(stderr, "Debug: Unknown base given in alctype: %s\n", typeint_to_name(base));
            exit(4);
        }
    }
    rv = (typeptr) calloc(1, sizeof(struct typeinfo));
    if (rv == NULL) return rv;
    rv->basetype = base;
    return rv;
}

typeptr clone_type(typeptr t) {
    if (!t) return NULL;
    typeptr newt = malloc(sizeof(struct typeinfo));
    *newt = *t; // shallow copy
    if (t->basetype == FUNC_TYPE) {
        // Deep copy return type
        newt->u.f.returntype = clone_type(t->u.f.returntype);
        // Deep copy param list
        paramlist src = t->u.f.parameters, *dst = &newt->u.f.parameters;
        while (src) {
            *dst = malloc(sizeof(struct param));
            (*dst)->name = src->name;
            (*dst)->type = clone_type(src->type);
            (*dst)->next = NULL;
            dst = &(*dst)->next;
            src = src->next;
        }
    }
    return newt;
}

bool type_is_shared(typeptr t) {
    return t == any_typeptr ||
           t == null_typeptr ||
           t == integer_typeptr ||
           t == double_typeptr ||
           t == float_typeptr ||
           t == boolean_typeptr ||
           t == char_typeptr ||
           t == string_typeptr ||
           t == unit_typeptr ||
           t == array_base_typeptr ||
           t == array_int_typeptr ||
           t == array_string_typeptr ||
           t == n_integer_typeptr ||
           t == n_double_typeptr ||
           t == n_float_typeptr ||
           t == n_boolean_typeptr ||
           t == n_char_typeptr ||
           t == n_string_typeptr;
}


int get_non_null_type(int nullable_type) {
    switch (nullable_type) {
        case N_INT_TYPE:     return INT_TYPE;
        case N_DOUBLE_TYPE:  return DOUBLE_TYPE;
        case N_FLOAT_TYPE:   return FLOAT_TYPE;
        case N_BOOL_TYPE:    return BOOL_TYPE;
        case N_CHAR_TYPE:    return CHAR_TYPE;
        case N_STRING_TYPE:  return STRING_TYPE;
        default:             return -1; // Not a nullable type
    }
}

void free_paramlist(paramlist head) {
    while (head) {
        paramlist next = head->next;
        if (head->type) {
            free_type(head->type);
            head->type = NULL;
        }
        free(head);
        head = next;
    }
}

void free_type(typeptr t) {
    if (!t || type_is_shared(t)) return;

    if (t->basetype == FUNC_TYPE) {
        if (t->u.f.parameters) {
            free_paramlist(t->u.f.parameters);
            t->u.f.parameters = NULL;
        }
        if (t->u.f.returntype) {
            free_type(t->u.f.returntype);
            t->u.f.returntype = NULL;
        }
    }
    free(t);
}

int typeptr_to_size(typeptr t) {
    if (!t) return 0;
    switch (t->basetype) {
        case INT_TYPE:
        case N_INT_TYPE:
            return 4;
        case FLOAT_TYPE:
        case N_FLOAT_TYPE:
            return 4;
        case DOUBLE_TYPE:
        case N_DOUBLE_TYPE:
            return 8;
        case BOOL_TYPE:
        case N_BOOL_TYPE:
            return 1;
        case CHAR_TYPE:
        case N_CHAR_TYPE:
            return 1;
        case STRING_TYPE:
        case N_STRING_TYPE:
        case ARRAY_BASE_TYPE:
        case ARRAY_STRING_TYPE:
        case FUNC_TYPE:
        case CLASS_TYPE:
            return 8; // pointer size on 64-bit systems
        default:
            return 8; // fallback default
    }
}
