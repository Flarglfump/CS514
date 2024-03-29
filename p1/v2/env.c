/*
Author: Gavin Witsken
Program: ish - simple shell
File: env.c
*/

#include "env.h"
#include <regex.h>

void ish_var_init(ish_var_t * var) {
    if (var == NULL) {
        return;
    }

    var->key = NULL;
    var->next = NULL;

    var->val_cap = ISH_DEFAULT_VAL_BUF_SIZE;
    var->val_count = 0;
    var->values = (char **) calloc(var->val_cap, sizeof(char **));
}
void ish_var_clear_vals(ish_var_t * var) {
    if (var == NULL) {
        return;
    }
    var->val_count = 0;
    if (var->values == NULL) {
        var->val_cap = 0;
        return;
    }
    for (size_t i = 0; i < var->val_cap; i++) {
        if ((var->values[i]) != NULL) {
            free((var->values)[i]);
        }
    }
}
void ish_var_free(ish_var_t * var) {
    if (var == NULL) {
        return;
    }
    if (var->key != NULL) {
        free(var->key);
    }
    if (var->values != NULL) {
        ish_var_clear_vals(var); // Free internal value strings
        free(var->values); // Free array of value strings
    }
    var->val_count = 0;
    var->val_cap = 0;
}
void ish_var_update_val(ish_var_t * var, const char * val) {
    ish_var_clear_vals(var);

    if (var->val_cap <= 1) {
        var->val_cap *= 2;
        char ** newValArr = (char **) calloc(var->val_cap, sizeof(char **));
        for (size_t i = 0; i < var->val_count ; i++) {
            newValArr[i] = (var->values)[i];
        }
        free(var->values);
        var->values = newValArr;
    }

    const size_t newValLen = strlen(val);
    *(var->values) = (char *) calloc(newValLen + 1, sizeof(char));
    strncpy(*(var->values), val, (newValLen + 1) * sizeof(char));
    var->val_count = 1;
}
void ish_var_update_vals(ish_var_t * var, const char ** vals, const size_t numVals) {
    ish_var_clear_vals(var);

    var->val_count = 0;
    if (var->val_cap <= numVals + 1) {
        var->val_cap = (numVals * 2) + 1;
        char ** newValArr = (char **) calloc(var->val_cap, sizeof(char **));
        for (size_t i = 0; i < var->val_count ; i++) {
            newValArr[i] = (var->values)[i];
        }
        free(var->values);
        var->values = newValArr;
    }

    for (size_t i = 0; i < numVals || vals == NULL || vals[i] != NULL; i++) {
        const size_t newValLen = strlen(vals[i]);
        (var->values)[i] = (char *) calloc(newValLen + 1, sizeof(char));
        strncpy((var->values)[i], vals[i], (newValLen + 1) * sizeof(char));
        var->val_count++;
    }
}
void ish_var_add_val(ish_var_t * var, const char * val) {
   
    if (var->val_cap <= var->val_count + 1) {
        var->val_cap = (var->val_count * 2) + 1;
        char ** newValArr = (char **) calloc(var->val_cap, sizeof(char **));
        for (size_t i = 0; i < var->val_count ; i++) {
            newValArr[i] = (var->values)[i];
        }
        free(var->values);
        var->values = newValArr;
    }

    const size_t newValLen = strlen(val);
    (var->values[var->val_count]) = (char *) calloc(newValLen + 1, sizeof(char));
    strncpy((var->values[var->val_count]), val, (newValLen + 1) * sizeof(char));
    (var->val_count)++;
}
void ish_var_add_vals(ish_var_t * var, const char ** vals, const size_t numVals) {
    
    if (var->val_cap <= var->val_count + numVals + 1) {
        var->val_cap = ((var->val_count + numVals) * 2) + 1;
        char ** newValArr = (char **) calloc(var->val_cap, sizeof(char **));
        for (size_t i = 0; i < var->val_count ; i++) {
            newValArr[i] = (var->values)[i];
        }
        free(var->values);
        var->values = newValArr;
    }

    for (size_t i = var->val_count; i < numVals || vals == NULL || vals[i] != NULL; i++) {
        const size_t newValLen = strlen(vals[i - var->val_count]);
        (var->values)[i] = (char *) calloc(newValLen + 1, sizeof(char));
        strncpy((var->values)[i], vals[i], (newValLen + 1) * sizeof(char));
    }
    var->val_count += numVals;
}

void ish_var_list_init(ish_var_list_t * varList) {
    if (varList == NULL) {
        return;
    }

    varList->head = NULL;
    varList->var_count = 0;
}

void ish_var_list_free(ish_var_list_t * varList) {
    if (varList == NULL) {
        return;
    }

    ish_var_t * prev = NULL;
    ish_var_t * cur = varList->head;

    while(cur != NULL) {
        if (prev != NULL) {
            free(prev);
        }

        ish_var_free(cur);

        prev = cur;
        cur = cur->next;
    }
    if (prev != NULL) {
        free(prev);
    }
    varList->var_count = 0;
    varList->head = NULL;
}
ish_var_t * ish_var_list_find(ish_var_list_t * varList, const char * key) {
    if (varList == NULL || key == NULL) {
        return NULL;
    }
    
    ish_var_t * cur = varList->head;
    while (cur != NULL) {
        if (strcmp(key, cur->key) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}
int ish_var_list_remove(ish_var_list_t * varList, const char * key) {
    if (varList == NULL || key == NULL) {
        return 1;
    }

    ish_var_t * prev = NULL;
    ish_var_t * cur = varList->head;

    while(cur != NULL) {
        if (strcmp(key, cur->key) == 0) {
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL) {
        return 1;
    }

    if (cur == varList->head) {
        // Remove head
        if (cur->next != NULL) {
            varList->head = cur->next;
        } else {
            varList->head = NULL;
        }
        ish_var_free(cur);
        free(cur);
        (varList->var_count)--;
    } else {
        // Remove somewhere other than head
        if (prev != NULL) {
            prev->next = cur->next;
            ish_var_free(cur);
            free(cur);
            (varList->var_count)--;
        } else {
            return 1;
        }
    }
    return 0;
}
void ish_var_list_append_val(ish_var_list_t * varList, const char * key, const char * val) {
    if (varList == NULL || key == NULL) {
        return;
    }

    ish_var_t * var = (ish_var_t *) calloc(1, sizeof(ish_var_t));
    ish_var_init(var);
    
    // Copy key
    if (var->key != NULL) {
        free(var->key);
    }
    const size_t keyLen = strlen(key);
    var->key = (char*) calloc(keyLen + 1, sizeof(char));
    strncpy(var->key, key, keyLen * sizeof(char));

    // Copy val
    ish_var_add_val(var, val);

    // Insert new var in list
    if (varList->head == NULL) {
        varList->head = var;
        varList->var_count = 1;
    } else {
        ish_var_t * cur = varList->head;
        size_t cnt = 1;
        while (cur->next != NULL) {
            cur = cur->next;
            cnt++;
        }
        cur->next = var;
    }
}
void ish_var_list_append_vals(ish_var_list_t * varList, const char * key, const char ** vals, const size_t numVals) {
    if (varList == NULL || key == NULL) {
        return;
    }

    ish_var_t * var = (ish_var_t *) calloc(1, sizeof(ish_var_t));
    ish_var_init(var);
    
    // Copy key
    if (var->key != NULL) {
        free(var->key);
    }
    const size_t keyLen = strlen(key);
    var->key = (char*) calloc(keyLen + 1, sizeof(char));
    strncpy(var->key, key, keyLen * sizeof(char));

    // Copy val
    ish_var_add_vals(var, vals, numVals);

    // Insert new var in list
    if (varList->head == NULL) {
        varList->head = var;
        varList->var_count = 1;
    } else {
        ish_var_t * cur = varList->head;
        size_t cnt = 1;
        while (cur->next != NULL) {
            cur = cur->next;
            cnt++;
        }
        cur->next = var;
    }
}
void ish_var_list_update_var_val(ish_var_list_t * varList, const char * key, const char * val) {
    if (varList == NULL || key == NULL) {
        return;
    }
    ish_var_t * var = ish_var_list_find(varList, key);
    
    // Allocate and copy key or clear vals as needed
    if (var == NULL) {
        ish_var_t * var = (ish_var_t *) calloc(1, sizeof(ish_var_t));
        ish_var_init(var);

        if (var->key != NULL) {
            free(var->key);
        }
        const size_t keyLen = strlen(key);
        var->key = (char*) calloc(keyLen + 1, sizeof(char));
        strncpy(var->key, key, keyLen * sizeof(char));
    } else {
        ish_var_clear_vals(var);
    }

    // Copy val
    ish_var_add_val(var, val);

    if (varList->head == NULL) {
        varList->head = var;
        varList->var_count = 1;
    } else {
        ish_var_t * cur = varList->head;
        size_t cnt = 1;
        while (cur->next != NULL) {
            cur = cur->next;
            cnt++;
        }
        cur->next = var;
    }
}
void ish_var_list_update_var_vals(ish_var_list_t * varList, const char * key, const char ** vals, const size_t numVals) {
    if (varList == NULL || key == NULL) {
        return;
    }

    ish_var_t * var = ish_var_list_find(varList, key);
    
    // Allocate and copy key or clear vals as needed
    if (var == NULL) {
        ish_var_t * var = (ish_var_t *) calloc(1, sizeof(ish_var_t));
        ish_var_init(var);

        if (var->key != NULL) {
            free(var->key);
        }
        const size_t keyLen = strlen(key);
        var->key = (char*) calloc(keyLen + 1, sizeof(char));
        strncpy(var->key, key, keyLen * sizeof(char));
    } else {
        ish_var_clear_vals(var);
    }

    // Copy val
    ish_var_add_vals(var, vals, numVals);

    if (varList->head == NULL) {
        varList->head = var;
        varList->var_count = 1;
    } else {
        ish_var_t * cur = varList->head;
        size_t cnt = 1;
        while (cur->next != NULL) {
            cur = cur->next;
            cnt++;
        }
        cur->next = var;
    }
}

const char ** ish_var_list_get_val(ish_var_list_t * varList, const char * key, size_t * numVals) {
    if (varList == NULL || key == NULL || numVals == NULL) {
        return NULL;
    }

    ish_var_t * var = ish_var_list_find(varList, key);

    if (var == NULL) {
        return NULL;
    }

    *numVals = var->val_count;
    return (const char **) (var->values);
}
int ish_var_key_is_valid(const char * key) {
    regex_t regex;
    char matchPattern[] = "^[a-zA-Z_][a-zA-Z0-9_]*$";
    int res;

    res = regcomp(&regex, matchPattern, REG_EXTENDED);
    if (res) {
        return 0; // Could not compile regex
    }

    res = regexec(&regex, key, 0, NULL, 0);

    regfree(&regex);
    
    if (res) {
        return 0;
    }

    return 1;
}