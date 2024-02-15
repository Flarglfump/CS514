#pragma once
/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.h
Created: 10:25 AM, 31 Jan 2024
Modified: 12:55 PM, 31 Jan 2024
*/

#include "lexer.h"

enum CMD_TOKEN_TYPE {
    WORD,
    COMMAND,
    FILENAME,
    BACKGROUND,
    PIPE,
    PIPE_ERROR,
    SEMICOLON,
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_ERROR,
    APPEND,
    APPEND_ERROR,
    NONE
};

typedef struct command {
    
    enum CMD_TOKEN_TYPE type;
    bool is_valid;
} command_t;