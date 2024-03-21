/*
Author: Gavin Witsken
Program: ish - interactive shell
File: types.h
*/
#pragma once

#include "const.h"

/* Simple enum type used to identify types of parsed tokens. EXEC, REDIR, PIPE, LIST, and BACK are pulled from xv6 sh - see Credits.txt for more details */
typedef enum ish_token {
    ISH_TOKEN_TYPE_NONE,
    ISH_TOKEN_TYPE_EXEC,
    ISH_TOKEN_TYPE_REDIR,
    ISH_TOKEN_TYPE_PIPE,
    ISH_TOKEN_TYPE_LIST,
    ISH_TOKEN_TYPE_BACK
} ish_token_t;

/* Generic node to represent a parsed token. From xv6 sh. */
typedef struct ish_cmd {
    ish_token_t type;
} ish_cmd_t;

typedef struct ish_execcmd {
  ish_token_t type;
  char * argv[ISH_MAX_ARG_COUNT];
  char * eargv[ISH_MAX_ARG_COUNT];
} ish_execcmd_t;

/* Generic node to represent a parsed redirect command token. From xv6 sh. */
typedef struct ish_redircmd {
  ish_token_t type;
  ish_cmd_t * cmd;
  char * file;
  char * efile;
  int mode;
  int fd1;
  int fd2;
} ish_redircmd_t;

/* Generic node to represent a parsed pipe command token. From xv6 sh. */
typedef struct ish_pipecmd {
  ish_token_t type;
  ish_cmd_t * left;
  ish_cmd_t * right;
} ish_pipecmd_t;

/* Generic node to represent a parsed command list token. From xv6 sh. */
typedef struct ish_listcmd {
  ish_token_t type;
  ish_cmd_t * left;
  ish_cmd_t * right;
} ish_listcmd_t;

/* Generic node to represent a parsed pipe command token. From xv6 sh. */
typedef struct ish_backcmd {
  ish_token_t type;
  ish_cmd_t * cmd;
} ish_backcmd_t;