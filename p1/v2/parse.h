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
    2. Parse into pipeline components (Perform)
    3. Variable substitution (Perform)
    4. Filename substitution (Perform)
    5. Command substitution (Perform)
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

enum ish_sep_type {
    ISH_SEP_TYPE_NONE, // Not a separator
    ISH_SEP_TYPE_REDIR_OUT, // '>'
    ISH_SEP_TYPE_REDIR_OUT_FORCE, // '>!'
    ISH_SEP_TYPE_REDIR_OUT_ERR, // '>&'
    ISH_SEP_TYPE_REDIR_OUT_ERR_FORCE, // '>&!'
    ISH_SEP_TYPE_APPEND_OUT, // '>>'
    ISH_SEP_TYPE_APPEND_OUT_FORCE, // '>>!'
    ISH_SEP_TYPE_APPEND_OUT_ERR, // '>>&'
    ISH_SEP_TYPE_APPEND_OUT_ERR_FORCE, // '>>&!'
    ISH_SEP_TYPE_PIPE_OUT,  // '|'
    ISH_SEP_TYPE_PIPE_OUT_ERR, // '|&'
    ISH_SEP_TYPE_PIPELINE_TERM, // ';' or '\n'
    ISH_SEP_TYPE_LOGICAL_AND, // '&&'
    ISH_SEP_TYPE_LOGICAL_OR, // '||'
    ISH_SEP_TYPE_BG, // '&'
    ISH_SEP_TYPE_OPEN_PAREN, // '('
    ISH_SEP_TYPE_CLOSE_PAREN, // ')'
    ISH_SEP_TYPE_REDIR_IN, // '<'
    ISH_SEP_TYPE_HEREDOC // '<<'
};

/* Struct that acts as single element within ish_pipeline struct */
typedef struct ish_pipeline_component {
    enum ish_pipeline_component_type componentType; // Describes type of component
    enum ish_sep_type sepType; // Describes type of separator
    int startWordIdx; // Indicates index in serialized word list where component starts
    int endWordIdx; // Indicates index in serialized word list where component ends
} ish_pipeline_component_t;

/* Initialize new ish pipeline component */
void ish_pipeline_component_init(ish_pipeline_component_t * component);

/* Field to specify execution mode of pipeline */
enum ish_pipeline_execution_mode {
    ISH_PIPELINE_EXECUTION_MODE_SEQ,
    ISH_PIPELINE_EXECUTION_MODE_BG
};

/* Struct used to hold sequence of command pipeline components and separators.*/
typedef struct ish_pipeline {
    ish_serialized_word_list_t serializedWordList;
    size_t capacity;
    size_t componentCnt;
    enum ish_pipeline_execution_mode execMode;

    ish_pipeline_component_t * components;
} ish_pipeline_t;

/* Initialize new ish pipeline, not yet parsing out components */
void ish_pipeline_init(ish_pipeline_t * pipeline, const ish_serialized_word_list_t serializedWordList);
/* Insert new pipeline component into ish pipeline */
void ish_pipeline_append(ish_pipeline_t * pipeline, const ish_pipeline_component_t component);
/* Classify string as ish separator type */
enum ish_sep_type get_ish_sep_type(const char * str, const size_t len);
/* Create sub-pipeline from subset of source pipeline. Makes shallow copy of memory. */
ish_pipeline_t ish_pipeline_create_sub_pipeline(ish_pipeline_t * srcPipeline, const int componentStartIdx, const int componenetEndIdx);

/* Circular FIFO queue of command pipelines used for staging before substitutions are performed */
typedef struct ish_staging_queue {
    size_t head;
    size_t tail;

    size_t capacity;
    size_t size;
    ish_pipeline_t * data;
} ish_staging_queue_t;

/* Initializes staging queue values and data buffer */
void ish_staging_queue_init(ish_staging_queue_t * stagingQueue);
/* Push element onto the back of queue - does not make deep copy */
void ish_staging_queue_enqueue(ish_staging_queue_t * stagingQueue, const ish_pipeline_t pipeline);
/* Get a pointer to the element at the front of the queue */
ish_pipeline_t * ish_staging_queue_front(ish_staging_queue_t * stagingQueue);
/* Pop  element at front of queue (does not free memory) */
void ish_staging_queue_dequeue(ish_staging_queue_t * stagingQueue);
/* Fetches element at front of queue, and then pops it (does not free memory) */
ish_pipeline_t * ish_staging_queue_fetch_and_dequeue(ish_staging_queue_t * stagingQueue);
/* Frees data buffer inside struct. Does not free any memory nested within elements themselves */
void ish_staging_queue_free(ish_staging_queue_t * stagingQueue);
/* Build ish pipeline queue and components from unlabeled pipeline object (may contain more than one pipeline internally). */
void ish_staging_queue_build_from_pipeline(ish_staging_queue_t * stagingQueue, ish_pipeline_t * pipeline);