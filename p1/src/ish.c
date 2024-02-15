/*
Author: Gavin Witsken
Program: ish - simple shell
File: command.h
Created: 10:25 AM, 31 Jan 2024
Modified: 1:29 PM, 8 Feb 2024
*/

#include "ish.h"

int main() {

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