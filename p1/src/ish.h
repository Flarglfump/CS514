/*
Author: Gavin Witsken
Program: ish - simple shell
File: ish.h
*/
#pragma once

#include "parser.h"
#include "lexer.h"
#include "env.h"

#include <unistd.h>
#include <limits.h>
#include <signal.h>

char PROMPT[256] = {0};

void ish_init(char ** prev_envp);