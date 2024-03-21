/*
Author: Gavin Witsken
Program: ish - interactive shell
File: ish.c
*/
#pragma once

/* Included files */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include "types.h"

/* CONSTANTS */
#ifndef STDIN_FILENO // From unistd.h
#define STDIN_FILENO 0 
#endif
#ifndef STDOUT_FILENO // From unistd.h
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO // From unistd.h
#define STDERR_FILENO 2
#endif

/* Print message to STDERR_FILENO and terminate process. From xv6 sh. */
void ish_panic(const char * msg);

/* Try to fork() process. If failed, call ish_panic function (which exits program). From xv6 sh. */
int ish_fork_assert(void);

/* Execute an ish_cmd given root node of AST. Calling process will always terminate. From xv6 sh. */
void ish_runcmd(ish_cmd_t * cmd);

/* Wrapper for call to execve - will explicitly pass in extern char ** environ */
int ish_exec(const char * cmd, char * const * argv);

/* Constructor for execcmd node */
ish_cmd_t * ish_execcmd_create(void);
/* Constructor for redircmd node */
ish_cmd_t * ish_redircmd_create(ish_cmd_t * subcmd, char * file, char * efile, int mode, int fd1, int fd2);
/* Constructor for pipecmd node */
ish_cmd_t * ish_pipecmd_create(ish_cmd_t * left, ish_cmd_t * right);
/* Constructor for listcmd node */
ish_cmd_t * ish_listcmd_create(ish_cmd_t * left, ish_cmd_t * right);
/* Constructor for backcmd node */
ish_cmd_t * ish_backcmd_create(ish_cmd_t * subcmd);

/* Print prompt, then get command from standard input */
int ish_getcmd(char * buf, int nbuf);