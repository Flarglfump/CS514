/*
Author: Gavin Witsken
Program: ish - simple shell
File: lex.h
*/
#pragma once

#include "ish.h"

/* enum type indicating how a word is quoted */
typedef enum ish_quote_type {
    ISH_QUOTE_TYPE_DOUBLE, // "string"
    ISH_QUOTE_TYPE_SINGLE, // 'string'
    ISH_QUOTE_TYPE_BACKTICK, // `string`
    ISH_QUOTE_TYPE_BACKSLASH, // \string
    ISH_QUOTE_TYPE_NONE // string
} ish_quote_type_t;

/* Indicates type to be used as elements within stack */
typedef ish_quote_type_t ish_stack_elem_t;

/* Array-buffered list limited to insertion and removal from a single end */
typedef struct ish_stack {
    ish_stack_elem_t * data;
    size_t capacity;
    size_t len;
} ish_stack_t;

/* Initializes stack with default capacity */
void ish_stack_init(ish_stack_t * stack);
/* Pops the element at top of stack */
void ish_stack_pop(ish_stack_t * stack);
/* Pushes element to top of stack */
void ish_stack_push(ish_stack_t * stack, const ish_stack_elem_t val);
/* Returns element at top of stack */
ish_stack_elem_t ish_stack_top(ish_stack_t * stack);
/* Frees data buffer of stack */
void ish_stack_free(ish_stack_t * stack);

/* Struct used to buffer character strings */
typedef struct ish_word {
    size_t bufSize; // number of allocated characters in buffer
    size_t wordLen; // number of characters actually used
    char * wordBuf; // buffer to hold characters

    int appendNext; // Append next token to this one
    ish_quote_type_t quoteType; // Note if word has been quoted
} ish_word_t;

/* Initializes an ish_word_t buffer using the default word size */
void ish_word_init(ish_word_t * word);
/* Appends a character to an ish_word_t buffer. Will allocate additional space if needed. */
void ish_word_append(ish_word_t * word, const char c);
/* Trims excess allocated memory from the internal word buffer */
void ish_word_trim_allocation(ish_word_t * word);
/* Frees the internal word buffer */
void ish_word_free(ish_word_t * word);

/* Struct used as a node for an ish_word_list */
typedef struct ish_word_list_node {
    ish_word_t data;
    ish_word_list_node_t * next;
} ish_word_list_node_t;

/* Struct used to hold singly-linked-list of ish word nodes */
typedef struct ish_word_list {
    ish_word_list_node_t * head;
    ish_word_list_node_t * tail;
    size_t word_count;
} ish_word_list_t;

/* Initializes ish word list struct */
void ish_word_list_init(ish_word_list_t * list);
/* Appends ish word onto ish word list */
void ish_word_list_append(ish_word_list_t * list, const ish_word_t word);
/* Frees internal char buffers of all words in list. Then also frees nodes in list.*/
void ish_word_list_free(ish_word_list_t * list);

/* Lexically tokenizes string and places tokens into returned ish word list */
ish_word_list_t lex_str(const char * str);
/* Returns character value in str at index position + 1 */
char peek1(const char * str, const size_t len, const int position);
/* Returns character value in str at index position + 2 */
char peek2(const char * str, const size_t len, const int position);

/* Returns 1 if character is a valid word separator blank, 0 if not */
int is_blank(const char c);
/* Returns 1 if character is a special character (start of potential unquoted special string), 0 if not */
int is_special_char(const char c);