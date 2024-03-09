/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.c
*/
#include "parser.h"

command_line_sequence_t parse_token_list (const token_buffer_list_t * const t_list) {
    unsigned char * line_sep_mask = (unsigned char *) calloc(t_list->token_count, sizeof(unsigned char));
    unsigned char * other_sep_mask = (unsigned char *) calloc(t_list->token_count, sizeof(unsigned char));

    command_line_sequence_t command_line_seq;
    command_token_t cur_command_token;
    command_line_t cur_command_line;

    command_line_sequence_init(&command_line_seq);
    command_token_init(&cur_command_token);
    command_line_init(&cur_command_line);
    
    if (!t_list) {
        return command_line_seq;
    }

    // Pass 1: Separate into commands and command lines
    token_buffer_list_node_t * cur_tbuf_node = t_list->head;
    token_buffer_t * cur_tbuf;
    size_t i, j;

    for (i = 0; cur_tbuf_node != NULL; i++, cur_tbuf_node = cur_tbuf_node->next) {
        cur_tbuf = &(cur_tbuf_node->token_buf);

        if (cur_tbuf->is_special) {
            // Could be command separator or line separator
            if (cur_tbuf->token_len == 1 && (*(cur_tbuf->buffer) == '\n' || *(cur_tbuf->buffer) == ';' || *(cur_tbuf->buffer) == '&')) {
                // Line separator - if command line size > 0, add current command line to command line sequence and reinitialize command line
                if (cur_command_line.command_tokens > 0) {
                    if (*(cur_tbuf->buffer) == '&') {
                        cur_command_line.execution_mode = ISH_MODE_BG;
                    }
                    command_line_sequence_add_line(&command_line_seq, cur_command_line);
                    command_line_init(&cur_command_line);
                }
            } else {
                // Command token separator - if command token size > 0, add current command token to command line. Add separator token to command line and reinitialize
                if (cur_command_token.argc > 0) {
                    command_line_add_token(&cur_command_line, cur_command_token); 
                    command_token_init(&cur_command_token);
                }

                command_token_add_arg(&cur_command_token, cur_tbuf->buffer, cur_tbuf->token_len, get_cmd_type(cur_tbuf->buffer, cur_tbuf->token_len));
                command_line_add_token(&cur_command_line, cur_command_token);
                command_token_init(&cur_command_token);
            }
        } else {
            // Not separator - Add arg to current command token
            command_token_add_arg(&cur_command_token, cur_tbuf->buffer, cur_tbuf->token_len, ISH_TYPE_UNKNOWN);
        }
    }
    // If anything left in current buffers, add them.
    if (cur_command_token.argc > 0) {
        command_line_add_token(&cur_command_line, cur_command_token);
    }
    command_token_init(&cur_command_token);

    if (cur_command_line.command_token_count > 0) {
        command_line_sequence_add_line(&command_line_seq, cur_command_line);
    }
    command_line_init(&cur_command_line);

    // Now everything should be stored in command line sequence struct

    // Pass 2: Classify tokens in command line sequence
    command_line_t * cur_line;
    command_token_t * cur_tok;
    enum CMD_TYPE last_type = ISH_TYPE_UNKNOWN;
    for (i = 0; i < command_line_seq.line_count; i++) {
        cur_line = &(command_line_seq.lines[i]);
        for (j = 0; j < cur_line->command_token_count; j++) {
            cur_tok = &(cur_line->command_tokens[j]);
            if (cur_tok->type == ISH_TYPE_UNKNOWN) {
                // Not separator
                if (j == 0) {
                    // First token in line - Expecting builtin or external command.
                    if (is_builtin_command(cur_tok->argv[0], strnlen(cur_tok->argv[0], BUILTIN_COMMAND_MAX_LEN))) {
                        // Token is a builtin command
                        cur_tok->type = ISH_TYPE_BUILTIN_COMMAND;
                    } else {
                        cur_tok->type = ISH_TYPE_EXTERN_COMMAND;
                    }
                } else if (j < cur_line->command_token_count - 1) {
                    // Token in middle of line - Expecting builtin command, external command, or file destination.
                    switch(last_type) {
                        case ISH_TYPE_SEP_REDIRECT_OUT:
                        case ISH_TYPE_SEP_APPEND:
                        case ISH_TYPE_SEP_APPEND_ERROR:

                        break;

                        case ISH_TYPE_SEP_REDIRECT_IN:
                        // Expecting
                        break;

                        case ISH_TYPE_SEP_PIPE:
                        // Expecting external command
                        break;
                    }
                } else {
                    // Last token in line (not first) - Expecting builtin command, external command, or file destination. Last token should be set.

                }
            }
            last_type = cur_tok->type;
        }
    }

    return command_line_seq;
}

void command_token_init(command_token_t * ct) {
    ct->argc = 0;
    ct->argv = NULL;
    ct->type = ISH_TYPE_UNKNOWN;
}

void command_token_add_arg(command_token_t * ct, char * arg, size_t arg_len, enum CMD_TYPE type) {
    // Allocate space for new string
    ct->argv = realloc(ct->argv, sizeof(char*) * ((ct->argc) + 1));
    char * newStr = ct->argv[(ct->argc)];

    // Copy new arg into null-terminated command_token arg
    size_t newStrLen = arg_len;
    newStr = (char *) calloc(newStrLen + 1, sizeof(char));
    strncpy(newStr, arg, newStrLen);

    // Increase arg count
    (ct->argv)++;
}
void command_token_free(command_token_t * ct) {
    // Free individual arg strings
    for (size_t i = 0; i < ct->argc; i++) {
        if (ct->argv[i]) {
            free(ct->argv[i]);
        }
    }

    // Free argv array
    free(ct->argv);
}

void command_line_init(command_line_t * cl) {
    cl->command_token_count = 0;
    cl->command_tokens = NULL;
    cl->execution_mode = ISH_MODE_SEQ;
}
void command_line_add_token(command_line_t * cl, command_token_t ct) {
    // Reallocate space for new command_token
    cl->command_tokens = (command_token_t *) realloc(cl->command_tokens, (cl->command_token_count + 1) * sizeof(command_token_t));
    
    // Shallow copy command_token to end of command_tokens array
    cl->command_tokens[cl->command_token_count] = ct;

    // Increase command_token_count
    (cl->command_token_count)++;
}
void command_line_free(command_line_t * cl) {
    // Free command tokens
    for (size_t i = 0; i < cl->command_token_count; i++) {
        command_token_free(&(cl->command_tokens[i]));
    }

    // Free command token array
    free(cl->command_tokens);
}

void command_line_sequence_init(command_line_sequence_t * cls) {
    cls->line_count = 0;
    cls->lines = NULL;
}

void command_line_sequence_add_line(command_line_sequence_t * cls, command_line_t cl) {
    // Reallocate space for new command_line
    cls->lines = (command_line_t *) realloc(cls->lines, (cls->line_count + 1) * sizeof(command_line_t));
    
    // Shallow copy command_line to end of command_lines array
    cls->lines[cls->line_count] = cl;

    // Increase command_token_count
    (cls->line_count)++;
}

void command_line_sequence_free(command_line_sequence_t * cls) {
    // Free command lines
    for (size_t i = 0; i < cls->line_count; i++) {
        command_line_free(&(cls->lines[i]));
    }

    // Free command line array
    free(cls->lines);
}

enum CMD_TYPE get_sep_type(char * s, size_t len) {
    enum CMD_TYPE ret = ISH_TYPE_UNKNOWN;
    switch(len) {
        case 1:
            if (*s == '>') {
                ret = ISH_TYPE_SEP_REDIRECT_OUT;
            } else if (*s == '<') {
                ret = ISH_TYPE_SEP_REDIRECT_IN;
            } else if (*s == '|') {
                ret = ISH_TYPE_SEP_PIPE;
            } else if (*s == '&') {
                ret = ISH_TYPE_SEP_BACKGROUND;
            }
        break;
        case 2:
            if (strncmp(s, ">>", 2) == 0) {
                ret = ISH_TYPE_SEP_REDIRECT_OUT;
            } else if (strncmp(s, ">&", 2) == 0) {
                ret = ISH_TYPE_SEP_REDIRECT_ERROR;
            }
        break;
        case 3:
            if (strncmp(s, ">>&", 3)) {
                ret = ISH_TYPE_SEP_APPEND_ERROR;
            }
        break;
        default:
            ret = ISH_TYPE_UNKNOWN;
    }

    return ret;
}

bool is_builtin_command(char * s, size_t len) {
    for (size_t i = 0; i < BUILTIN_COMMAND_COUNT; i++) {
        if (strncmp(s, BUILTIN_COMMANDS[i], len) == 0) {
            return true;
        }
    }
    return false;
}