/*
Author: Gavin Witsken
Program: ish - interactive shell
File: parse.h
*/
#pragma once

#include "ish.h"

/* Much of this code is is taken from or has been adapted from xv6 sh. */

/* Parse command to build abstract syntax tree. From xv6 sh. */
ish_cmd_t * ish_parsecmd(char * s);
/* Parse line cmd. From xv6 sh. */
ish_cmd_t * ish_parseline(char ** ps, char * es);
/* Parse pipe cmd. From xv6 sh. */
ish_cmd_t * ish_parsepipe(char ** ps, char * es);
/* Parse exec cmd. From xv6 sh. */
ish_cmd_t * ish_parseexec(char ** ps, char * es);
/* Parse redir cmd. From xv6 sh. */
ish_cmd_t * ish_parseredirs(ish_cmd_t * cmd, char ** ps, char * es);
/* Parse input block. From xv6 sh. */
ish_cmd_t * ish_parseblock(char ** ps, char * es);

/* Null-terminate strings that were counted. From xv6 sh. */
ish_cmd_t * ish_nulterminate(ish_cmd_t * cmd);

/* Get token from input. From xv6 sh. */
int ish_gettoken(char ** ps, char * es, char ** q, char ** eq);

/* Peek ahead to find next non-whitespace token. From xv6 sh. */
int ish_peek(char ** ps, char * es, char * toks);