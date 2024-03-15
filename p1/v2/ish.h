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

#ifndef ISH_STAGING_QUEUE_DEFAULT_BUF_SIZE
#define ISH_STAGING_QUEUE_DEFAULT_BUF_SIZE 100
#endif

/*
    Processing order:
    1) Read input, lexing into word tokens
    2) Parse word tokens to indentify separators
    3) Split into pipelines based on pipeline separators
    4) Push each pipeline into staging queue in order received

    5) For each pipeline in staging queue:
        5a) Do variable and filename substitution
        5b) Do command substitution. Interpret subcommand within subshell using rules of raw input (recursively start again from step 1 again). Redirect output from subcommand into buffer on main thread. WAIT on main thread until subcommand is executed. Replace subcommand from initial pipeline with redirected subcommand output.
        5c) Once ALL substitution is completed, build components in pipeline, packing together words that should be and labeling components according to intended function.
        5d) If pipeline should be run in sequential mode (not background), do not work on next pipeline in queue until previous pipeline has finished in the execution stage

    6) Execution stage:
        Try running command component by component, handling redirections, setting job groups, signal control, etc. according to specified functionality. Print errors to main shell stderr. If necessary, notify next process in staging queue once execution has finished.
*/

// Extern types and globals
extern struct ish_var_list ish_env;

// Globals
