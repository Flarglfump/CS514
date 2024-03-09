/*
Author: Gavin Witsken
Program: ish - simple shell
File: env.h
*/
#pragma once

extern char ** ISH_ENV_P; // Created in env.c

typedef enum ISH_ENV_RESULT {
    SUCCESS,
    INVALID_NAME,
    NOT_FOUND
} ISH_ENV_RESULT_T;

ISH_ENV_RESULT_T ish_setenv(const char* name, const char* val);
ISH_ENV_RESULT_T ish_unsetenv(const char* name, const char* val);
int env_name_is_valid(const char* name);
int get_env_list_size();
void free_ish_env();