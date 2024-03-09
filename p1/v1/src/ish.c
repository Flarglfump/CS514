/*
Author: Gavin Witsken
Program: ish - simple shell
File: ish.c
*/

#include "ish.h"

int main(int argc, char * argv[], char* envp[]) {
    if (argc >= 2) {
        // Start shell in different directory
        
    }   

    while (1) {
        printf("Prompt: $ ");
        char* line;
        size_t size;
        ssize_t str_size;
        str_size = getline(&line, &size, stdin);

        token_buffer_list_t lexed_list = lex_str(line);

        token_buffer_list_print(lexed_list);    
        token_buffer_list_free(&lexed_list);
    }
    
    return 0;
}