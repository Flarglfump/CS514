#pragma once
/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.h
Created: 12:30 PM, 31 Jan 2024
Modified: 12:45 PM, 31 Jan 2024
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>

// Used as buffer when parsing out tokens
typedef struct token_buffer {
    size_t token_len;
    size_t buf_size;
    char* buff; // Dynamically allocated - must be freed
    bool is_special;
    bool is_valid;
} token_buffer_t;
// Initialize token buffer with some token size
void token_buffer_init(token_buffer_t * buf, const size_t buf_size);
// Will allocate a new token buffer on the heap, initialize it, and return a pointer to it.
token_buffer_t * token_buffer_create_and_init(const size_t buf_size);
// Free allocated memory from internal buffer
void token_buffer_free(token_buffer_t * buf);
// Append character to end of current constructed string. Will reallocate for more size if needed.
void token_buffer_append(token_buffer_t * buf, const char ch);

// Structure to serve as node in token buffer list
typedef struct token_buffer_list_node {
    token_buffer_t token_buf;
    struct token_buffer_list_node * next;
} token_buffer_list_node_t;
// Holds list of token buffers to be processed later
typedef struct token_buffer_list {
    token_buffer_list_node_t * head; // Dynamically allocated - must be freed after freeing child struct allocations
    size_t token_count;
} token_buffer_list_t;

// Initializes token buffer list
void token_buffer_list_init(token_buffer_list_t * list);
// Will allocate a new token buffer list on the heap, initialize it, and return a pointer to it.
token_buffer_list_t * token_buffer_list_create_and_init();
// Frees memory stored in token buffers internally. Then frees the nodes themselves.
void token_buffer_list_free(token_buffer_list_t * list);
// Append shallow copy of token buffer to end of list.
void token_buffer_list_append(token_buffer_list_t * list, const token_buffer_t token_buf);

// Will check is character is a special character
bool is_special_char(const char ch);
// Will check is character is a whitespace separator character
bool is_whitespace_sep(const char ch);

// Peek 1 characters ahead in string. Return -1 if out of bounds.
char peek1(const char * const str, const int cur_pos, const size_t max_size);
// Peek 2 characters ahead in string. Return -1 if out of bounds.
char peek2(const char* const str, const int cur_pos, const size_t max_size);

// Will try to read line-separated text file at given path and hierarchically fill structs: command_line_list -> command_line -> command -> command_token
token_buffer_list_t lex_file(const char* fname);
// First pass; will parse string given as a single line of command input and hierarchically fill structs: command_line -> command -> command_token
token_buffer_list_t lex_str(const char* line);

// Print out list of nodes in lexed string
void token_buffer_list_print(const token_buffer_list_t list);