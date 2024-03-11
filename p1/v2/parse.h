/*
Author: Gavin Witsken
Program: ish - simple shell
File: parse.h
*/
#pragma once

#include "lex.h"
#include "ish.h"

/*
    From csh(1):

    After the input line is aliased and parsed, and before each command is executed, variable substitution is performed keyed by '$' characters. This expansion can be prevented by preceding the '$' with a '\' except within '"'s where it always occurs, and within '''s where it never occurs. Strings quoted by '`' are interpreted later (see Command substitution below) so '$' substitution does not occur there until later, if at all. A '$' is passed unchanged if followed by a blank, tab, or end-of-line.
*/

/*
    Passes:
    1. Lexing (done)
    2. Parse into pipeline components
    3. Variable substitution
    4. Command substitution (Perform)
*/

/*
    The first word in the buffer and the first word following ';', '|', '|&', '&&' or '||' is considered to be a command. A word beginning with '$' is considered to be a variable. Anything else is a filename.
*/

/* Struct used to hold a serialized word list (array-based). Should be constructed from existing ish_word_list struct (linked list). */
typedef struct ish_serialized_word_list {
    ish_word_t * words;
    size_t word_count;
} ish_serialized_word_list_t;

/* Create serialized word list from standard word list. Make deep copies of strings. Do not clobber or modify original word list. */
ish_serialized_word_list_t ish_serialize_word_list(const ish_word_list_t ish_word_list);

/* Describes general type of component */
enum ish_pipeline_component_type {
    ISH_PIPELINE_COMPONENT_TYPE_DEFAULT,
    ISH_PIPELINE_COMPONENT_TYPE_COMMAND,
    ISH_PIPELINE_COMPONENT_TYPE_SEPARATOR,
    ISH_PIPELINE_COMPONENT_TYPE_FILE
};

/* Struct that acts as single element within ish_pipeline struct */
typedef struct ish_pipeline_component {
    enum ish_pipeline_component_type type; // Describes type of component
    int startWordIdx; // Indicates index in serialized word list where component starts
    int endWordIdx; // Indicates index in serialized word list where component ends
} ish_pipeline_component_t;

/* Initialize new ish pipeline component */
void ish_pipeline_component_init(ish_pipeline_component_t * component);

/* Struct used to hold sequence of command pipeline components and separators.*/
typedef struct ish_pipeline {
    ish_serialized_word_list_t serializedWordList;
    size_t capacity;
    size_t componenentCnt;

    ish_pipeline_component_t * components;
} ish_pipeline_t;

/* Initialize new ish pipeline, not yet parsing out components */
void ish_pipeline_init(ish_pipeline_t * pipeline, const ish_serialized_word_list_t serializedWordList);
/* Insert new pipeline component into ish pipeline */
void ish_pipeline_append(ish_pipeline_t * pipeline, const ish_pipeline_component_t component);
/* Build ish pipeline and components from internal serialized word list */
void ish_pipeline_build(ish_pipeline_t * pipeline);