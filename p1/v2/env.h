/*
Author: Gavin Witsken
Program: ish - simple shell
File: env.h
*/

#include "ish.h"

/* Declared externally - holds system environment variables */
extern char ** environ;

/* Used as an entry in the ish variable list */
typedef struct ish_var {
    char * key;
    char ** values;
    size_t val_count;
    size_t val_cap;
    struct ish_var * next;
} ish_var_t;
/* Initialize new ish variable struct */
void ish_var_init(ish_var_t * var);
/* Clear values of ish var */
void ish_var_clear_vals(ish_var_t * var);
/* Free all memory internal to ish variable */
void ish_var_free(ish_var_t * var);
/* Overwrite values of existing ish variable */
void ish_var_update_val(ish_var_t * var, const char * val);
/* Overwrite values of existing ish variable*/
void ish_var_update_vals(ish_var_t * var, const char ** vals, const size_t numVals);
/* Add to existing values of ish variable */
void ish_var_add_val(ish_var_t * var, const char * val);
/* Add to existing values of ish variable */
void ish_var_add_vals(ish_var_t * var, const char ** vals, const size_t numVals);

/* Used to store list of ish variables */
typedef struct ish_var_list {
    struct ish_var * head;
    size_t var_count;
} ish_var_list_t;
/* Initialize new ish variable list */
void ish_var_list_init(ish_var_list_t * varList);
/* Free words in ish variable list */
void ish_var_list_free(ish_var_list_t * varList);
/* Find ish variable in list with name key. If found, return pointer to var; else NULL. */
ish_var_t * ish_var_list_find(ish_var_list_t * varList, const char * key);
/* Remove ish variable in list with name key. If found, return 0; else, 1. */
int ish_var_list_remove(ish_var_list_t * varList, const char * key);

