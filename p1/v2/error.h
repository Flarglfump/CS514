/*
Author: Gavin Witsken
Program: ish - simple shell
File: error.h
*/
#pragma once
#include "ish.h"

typedef enum ish_lex_error {
    ISH_LEX_ERR_OPEN_SINGLE_QUOTE,
    ISH_LEX_ERR_OPEN_DOUBLE_QUOTE,
    ISH_LEX_ERR_OPEN_BACK_QUOTE,
    ISH_LEX_ERR_OPEN_BACKSLASH,
    ISH_LEX_ERR_OPEN_CURLY_BRACE,
    ISH_LEX_ERR_OPEN_SQUARE_BRACKET,
    ISH_LEX_ERR_OPEN_PAREN,
    ISH_LEX_ERR_NONE
} ish_lex_error_t;