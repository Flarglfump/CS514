/*
Author: Gavin Witsken
Program: ish - interactive shell
File: env.c
*/
#include "env.h"

extern char ** environ;

ish_env_var_t * ish_env_init(ish_env_var_t * env) {
    if (!environ) {
        dprintf(STDERR_FILENO, "ish (warning): could not load PATH from system environment.\n");
    }

    return env;
}

ish_env_var_t * ish_env_var_set(ish_env_var_t * env, const char * key, const char * val) {
    ish_env_var_t * cur = env;
    ish_env_var_t * tail = env;
    
    if (!env) {
        cur = (ish_env_var_t *) malloc(sizeof(ish_env_var_t));
        cur->next = NULL;
        cur->key = strdup(key);
        cur->val = strdup(val);

        env = cur;

        return env;
    }
    
    while (cur != NULL) {
        if (cur->key && (strcmp(key, cur->key) == 0)) {
            break;
        }
        if (cur->next == NULL) {
            tail = cur;
        }
        cur = cur->next;
    }

    if (cur != NULL) {
        // Found key
        if (cur->val) {
            free(cur->val);
        }

        cur->val = strdup(val);
    } else if (tail) {
        cur = (ish_env_var_t *) malloc(sizeof(ish_env_var_t));
        cur->next = NULL;
        cur->key = strdup(key);
        cur->val = strdup(val);

        tail->next = cur;
    }

    return env;
}

ish_env_var_t * ish_env_var_unset(ish_env_var_t * env, const char * key, const char * val) {
    ish_env_var_t * prev = NULL;
    ish_env_var_t * cur = env;

    while (cur) {
        if (cur->key && strcmp(cur->key, key) == 0) {
            break;
        }

        prev = cur;
        cur = cur->next;
    }

    if (!cur) {
        // Not found
        return NULL;
    } else if (prev) {

    }
}

char * ish_env_var_get_val(ish_env_var_t * env, const char * key) {

}

int ish_env_var_is_set(ish_env_var_t * env, const char * key) {

}