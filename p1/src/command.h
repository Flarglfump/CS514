#pragma once
/*
Author: Gavin Witsken
Program: ish - simple shell
File: command.h
Created: 10:25 AM, 31 Jan 2024
Modified: 12:55 PM, 31 Jan 2024
*/

enum TOKEN_TYPE {
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
    OPTION,
    STRING,
    LOGICAL_AND,
    LOGICAL_OR,
    NONE
};

// Single token from command line
typedef struct {
    enum TOKEN_TYPE type;
    char* str;
} command_token;

// Single command between separators
typedef struct {
    command_token* tokens;
    enum TOKEN_TYPE l_sep, r_sep;
} command;

// Line of command input - end of line indicated by "\n" or "\r\n"
typedef struct {
    int arg_count;
    command* commands;
} command_line;

// Buffered list of lines to be processed - will be processed sequentially
typedef struct {
    command_line* lines;
} command_line_list;