/*
Author: Gavin Witsken
Program: ish - interactive shell
File: env.h
*/
#pragma once

#include "ish.h"

typedef struct ish_env_var {
    char * key;
    char * val;
    struct ish_env_var * next;
} ish_env_var_t;

/* Initialize ish environment upon shell startup */
ish_env_var_t * ish_env_init(ish_env_var_t * env);
/* Set environment variable value. Return pointer to first node in env list */
ish_env_var_t * ish_env_var_set(ish_env_var_t * env, const char * key, const char * val);
/* Remove environment variable. Return pointer to first node in env list if exists. If it does not exist, return NULL. */
ish_env_var_t * ish_env_var_unset(ish_env_var_t * env, const char * key, const char * val);
/* Searches for variable in env list with given key. If found, dynamically allocates copy of value. Returns created copy. If not found, returns NULL. */
char * ish_env_var_get_val(ish_env_var_t * env, const char * key);
/* Searches for variable in env list with given key. If found, returns non-zero value. If not found, returns 0. */
int ish_env_var_is_set(ish_env_var_t * env, const char * key);