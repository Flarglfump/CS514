#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct command_struct {
    char* path; //path of command to execute
    char** args; //argv array for exec
    char* left_separator; //find separator (|, <, >, >>)
    char* right_separator; //find separator (|, <, >, >>)
    int arg_count; //number of arguments in args array
    struct command_struct * next;
} command_struct;

void command_struct_init(command_struct* command) {
    command->arg_count = 0;
    command->next = NULL;
    command->path = NULL;
    command->args = NULL;
    command->left_separator = NULL;
    command->right_separator = NULL;
}

void command_struct_delete(command_struct* command) {
    free(command->args);
    // printf("Freed args\n");
    // if (command->right_separator != NULL) {
    //     free(command->right_separator);
    //     command->right_separator = NULL;
    // }
    // if (command->left_separator != NULL) {
    //     free(command->left_separator);
    //     command->left_separator = NULL;
    // }
}

void command_struct_arg_insert(command_struct* command, char* arg) {
    if (command->arg_count == 0) { //No args
        command->args = malloc(sizeof(char*));
        command->args[command->arg_count] = arg;
        ++command->arg_count;
    } else {
        command->args = realloc(command->args, (command->arg_count+1) * sizeof(char*));
        command->args[command->arg_count] = arg;
        if (arg != NULL) {
            ++command->arg_count;
        }
    }
}

typedef struct command_list { //linked list of command structs
    command_struct* root;
    int cmd_count;
} command_list;

void command_list_init(command_list* cmd_list) {
    cmd_list->cmd_count = 0;
    cmd_list->root = NULL;
}

void command_list_insert(command_list* cmd_list, command_struct* command) {
    if (cmd_list->root == NULL) { //List is empty
        cmd_list->root = command;
        ++cmd_list->cmd_count;
    } else {
        command_struct* node;
        node = cmd_list->root;

        while (node->next != NULL) {
            node = node->next;
        }

        node->next = command;
        ++cmd_list->cmd_count;
    }
}

void command_list_delete(command_list* cmd_list) {
    command_struct* node = cmd_list->root;
    command_struct* temp;
    
    int counter = 0;
    while (node != NULL) {
        temp = node->next;
        // printf("Attempting to delete command struct args at %d for command %s\n", counter, node->path);
        command_struct_delete(node);
        node = temp;
        counter++;
    }
}

void command_list_print(command_list* cmd_list) {

    command_struct* command = cmd_list->root;

    for (int i = 0; i < cmd_list->cmd_count; ++i) {
        
        fprintf(stderr, "Command %d:\n", i);

        printf("\tLeft separator: %s\n", command->left_separator);
        for (int j = 0; j < command->arg_count; j++) {
            fprintf(stderr, "\tArg %d: %s\n", j, command->args[j]);
        }

        fprintf(stderr, "\tRight separator: %s\n", command->right_separator);

        command=command->next;
    } 
}