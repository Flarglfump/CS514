/*
Author: Gavin Witsken
Program: ish - simple shell
File: ish.h
*/
#pragma once

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef ISH_DEFAULT_BUF_SIZE
#define ISH_DEFAULT_BUF_SIZE 256
#endif

#ifndef ISH_DEFAULT_VAL_BUF_SIZE
#define ISH_DEFAULT_VAL_BUF_SIZE 2
#endif

#ifndef ISH_DEFAULT_PIPELINE_BUF_SIZE
#define ISH_DEFAULT_PIPELINE_BUF_SIZE 5
#endif

// Extern types and globals
extern struct ish_var_list ish_env;

// Globals
