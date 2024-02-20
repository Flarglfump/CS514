#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct env_var{    //environment variable as a struct
    char* env_var_name;
    char* env_var_value;
    struct env_var* next;
}env_var;

env_var* create_env_var(char* name, char* value){
    env_var* new_env_var = malloc(sizeof(env_var));
    if (NULL != new_env_var){
            new_env_var->env_var_name = name;
            new_env_var->env_var_value = value;
            new_env_var->next = NULL;
    }
    return new_env_var;
}

env_var* delete_env_var(env_var* root, env_var* previous, char* word){
    env_var* head = root;
    env_var* prev = previous;
    if(strcmp(root->env_var_name, word) == 0){
        prev = head;
        head = head->next;
        root = root->next;
        free(prev);
        return (root);
    }
    else if(strcmp(head->env_var_name, word) != 0){
            while(strcmp(head->env_var_name, word) != 0){
            prev = head;
            head = head->next;
            }
            prev->next = head->next;
            free(head);
            return (root);
    }
    else{
    prev->next = head->next;
    free(head);
    return (root);
    }
}

char* search_env_var(env_var* root, char* word){
    env_var* head = root; 
    //printf("entering search function");
    while(strcmp(head->env_var_name, word) != 0 && head->next != NULL){            
            head = head->next;
            //search_env_var(head, word);
    }
    if(strcmp(head->env_var_name, word) == 0){
    return(head->env_var_value);
    }
    else {
        //printf("returning null from search function");
        return NULL;
    }
}

env_var* update_env_var(env_var* env_list, char* name, char* value){
    env_var* head = env_list;
    while(strcmp(head->env_var_name, name) != 0 && head != NULL){            
            head = head->next;
            //search_env_var(head, word);
    }
    head->env_var_value = value;
    return head;
}

env_var* add_env_var(env_var* env_list, char* name, char* value){
    env_var* head = env_list;
    if(search_env_var(head, name) == NULL ){
        env_var* new_env_var = create_env_var(name, value);
        if (NULL != new_env_var){
               new_env_var->next = env_list;
        }
        return new_env_var;
    }
    else{
        update_env_var(head, name, value);
        return head;
    }
}

void print_env(env_var* list){
    env_var* node = list;

    printf("-----ENVIRONMENT VARIABLES-----\n");
    while(node != NULL){
        printf("%s=%s\n", node->env_var_name, node->env_var_value);
        node = node->next;
    }
    printf("-----END OF ENVIRONMENT VARIABLES-----\n");
}




