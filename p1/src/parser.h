/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.h
*/

#pragma once
#include "lexer.h"

enum CMD_TYPE {
    SEP_REDIRECT_IN,
    SEP_REDIRECT_OUT,
    SEP_REDIRECT_ERROR,
    SEP_APPEND,
    SEP_APPEND_ERROR,
    SEP_LINE,
    COMMAND,
    FILE,
    UNKOWN
};

// Represents a single "command" token unit
typedef struct command_token {
    char * path; // path of command to execute - must be freed
    char ** argv; // argv array for exec - self and internals must be freed
    int argc; // number of arguments in argv array
    enum CMD_TYPE type;
} command_token_t;

typedef struct command_line {
    command_token_t * commands; // must be freed
} command_line_t;

typedef struct command_line_sequence {
    command_line_t * lines;
    size_t line_count;
} command_line_sequence_t;

command_line_sequence_t parse_token_list (const token_buffer_list_t * const t_list);