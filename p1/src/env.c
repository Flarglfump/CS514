/*
Author: Gavin Witsken
Program: ish - simple shell
File: env.c
*/
#include "env.h"
#include <stdlib.h>
#include <string.h>

char ** ISH_ENV_P;

ISH_ENV_RESULT_T ish_setenv(const char* name, const char* val) {
    char * cur;
    int found = -1, i;
    size_t name_len = strlen(name);
    size_t val_len = strlen(val);
    size_t new_size;
    ISH_ENV_RESULT_T result = SUCCESS;

    if (!ISH_ENV_P) {
        ISH_ENV_P = (char **) malloc(sizeof(char*));
    }

    for (i = 0, cur = ISH_ENV_P[i]; cur != NULL; cur++, i++) {
        if (strncmp(name, cur, name_len + 1) == 0) {
            found = i;
            break;
        }
    }

    new_size = name_len + val_len + 2;
    char * temp;

    if (found != -1) {
        // Found at existing location
        cur = realloc(cur, new_size);
    } else {
        ISH_ENV_P = realloc(ISH_ENV_P, i * sizeof(char **));
        cur = ISH_ENV_P[i];
    }

    temp = stpncpy(cur, name, name_len);
    *(temp++) = '=';
    temp = stpncpy(temp, val, val_len);
    *temp = '\0';

    return result;
}

ISH_ENV_RESULT_T ish_unsetenv(const char* name, const char* val) {
    char * cur;
    int found = -1, i;
    size_t name_len = strlen(name);
    ISH_ENV_RESULT_T result = SUCCESS;

    if (!ISH_ENV_P) {
        result = NOT_FOUND;
        return result;
    }

    for (i = 0, cur = *ISH_ENV_P; cur != NULL; cur++, i++) {
        if (strncmp(name, cur, name_len + 1) == 0) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        result = NOT_FOUND;
        return result;
    }

    int list_size = get_env_list_size();
    free(cur);

    // Shift other stuff over
    for (i = found; i < list_size - 1; i++) {
        ISH_ENV_P[i] = ISH_ENV_P[i+1];
    }

    // Resize list
    ISH_ENV_P = realloc(ISH_ENV_P, sizeof(char **) * (list_size - 1));

    result = SUCCESS;
    return result;
}

int env_name_is_valid(const char* name) {
    for (int i = 0; i < strlen(name); ++i) {
        if (name[i] == '=') {
            return 0;
        }
    }
    return 1;
}

void free_ish_env() {
    char * cur;

    if (!ISH_ENV_P) {
        return;
    }

    for (cur = *ISH_ENV_P; cur != NULL; cur++) {
        free(cur);
    }

    free(ISH_ENV_P);
}

int get_env_list_size() {
    if (!ISH_ENV_P) {
        return 0;
    }

    int i = 0;
    for (char* cur = *ISH_ENV_P; cur != NULL; cur++, i++) {;}
    return i;
}