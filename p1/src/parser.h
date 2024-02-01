#pragma once
/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.h
Created: 12:30 PM, 31 Jan 2024
Modified: 12:45 PM, 31 Jan 2024
*/

#include "command.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Used as buffer when parsing out tokens
typedef struct {
    size_t token_len;
    size_t buf_size;
    char* buff;
} token_buffer;
// Initialize token buffer with some token size
void token_buffer_init(token_buffer * buf, const size_t buf_size);
// Will allocate a new token buffer on the heap, initialize it, and return a pointer to it.
token_buffer * token_buffer_create_and_init(const size_t buf_size);
// Free allocated memory from internal buffer
void token_buffer_free(token_buffer * buf);
// Append character to end of current constructed string. Will reallocate for more size if needed.
void token_buffer_append(token_buffer * buf, const char ch);

// Structure to serve as node in token buffer list
typedef struct {
    token_buffer token_buf;
    struct token_buffer_list_node * next;
} token_buffer_list_node;
// Holds list of token buffers to be processed later
typedef struct {
    
    
    token_buffer_list_node * head;
    size_t token_count;
    
} token_buffer_list;

// Initializes token buffer list
token_buffer_list_init(token_buffer_list * list);
// Will allocate a new token buffer list on the heap, initialize it, and return a pointer to it.
token_buffer_list * token_buffer_list_create_and_init();
// Frees memory stored in token buffers internally. Then frees the nodes themselves.
token_buffer_list_free(token_buffer_list * list);
// Append shallow copy of token buffer to end of list.
void token_buffer_list_append(token_buffer_list * list, const token_buffer token_buf);

// Will check is character is a special character
bool is_special_char(const char ch);

// Will try to read line-separated text file at given path and hierarchically fill structs: command_line_list -> command_line -> command -> command_token
command_line_list parse_file(const char* fname);

// First pass; will parse string given as a single line of command input and hierarchically fill structs: command_line -> command -> command_token
command_line parse_line(const char* line);


