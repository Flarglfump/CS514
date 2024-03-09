/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.h
*/

#pragma once
#include "lexer.h"

enum CMD_TYPE {
    ISH_TYPE_SEP_REDIRECT_IN, // "<"
    ISH_TYPE_SEP_REDIRECT_OUT, // ">"
    ISH_TYPE_SEP_REDIRECT_ERROR, // ">&"
    ISH_TYPE_SEP_APPEND, // ">>"
    ISH_TYPE_SEP_APPEND_ERROR, // ">>&"
    ISH_TYPE_SEP_PIPE, // "|"
    ISH_TYPE_SEP_BACKGROUND, // "&"
    ISH_TYPE_SEP_LINE, // "\n" or ";"
    ISH_TYPE_BUILTIN_COMMAND, // One of predefined built-ins
    ISH_TYPE_EXTERN_COMMAND, // First arg should be path to executable file
    ISH_TYPE_DEST_FILE, // Destination file - right side of redirect out
    ISH_TYPE_SRC_FILE, // Source file - right side of redirect in
    ISH_TYPE_UNKNOWN // Other
};

enum EXECUTION_MODE {
    ISH_MODE_BG,
    ISH_MODE_SEQ
};

#define BUILTIN_COMMAND_COUNT 8
#define BUILTIN_COMMAND_MAX_LEN 9
const char BUILTIN_COMMANDS[BUILTIN_COMMAND_COUNT][BUILTIN_COMMAND_MAX_LEN] = {
    "bg",
    "cd",
    "exit",
    "fg",
    "jobs",
    "kill",
    "setenv",
    "unsetenv"
};

// Represents a single "command" token unit
typedef struct command_token {
    char ** argv; // argv array for exec - self and internals must be freed
    int argc; // number of arguments in argv array
    enum CMD_TYPE type;
} command_token_t;

typedef struct command_line {
    command_token_t * command_tokens; // must be freed
    enum EXECUTION_MODE execution_mode;
    size_t command_token_count;
} command_line_t;

typedef struct command_line_sequence {
    command_line_t * lines;
    size_t line_count;
} command_line_sequence_t;

command_line_sequence_t parse_token_list (const token_buffer_list_t * const t_list);

void command_token_init(command_token_t * ct);
void command_token_add_arg(command_token_t * ct, char * arg, size_t arg_len, enum CMD_TYPE type);
void command_token_free(command_token_t * ct);

void command_line_init(command_line_t * cl);
void command_line_add_token(command_line_t * cl, command_token_t ct);
void command_line_free(command_line_t * cl);

void command_line_sequence_init(command_line_sequence_t * cls);
void command_line_sequence_add_line(command_line_sequence_t * cls, command_line_t cl);
void command_line_sequence_free(command_line_sequence_t * cls);

enum CMD_TYPE get_sep_type(char * s, size_t len);
bool is_builtin_command(char * s, size_t len);