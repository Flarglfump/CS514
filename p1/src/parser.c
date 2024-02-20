/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.c
*/
#include "parser.h"
#include <string.h>

command_line_sequence_t parse_token_list (const token_buffer_list_t * const t_list) {
    unsigned char * line_sep_mask = (unsigned char *) calloc(t_list->token_count, sizeof(unsigned char)); 
    unsigned char * other_sep_mask = (unsigned char *) calloc(t_list->token_count, sizeof(unsigned char));

    command_line_sequence_t result;
    
    if (!t_list) {
        return result;
    }

    // Pass 1: identify separators
    size_t num_lines = 0;
    token_buffer_list_node_t * cur_tbuf = t_list->head;
    for (size_t i = 0; cur_tbuf != NULL; i++, cur_tbuf = cur_tbuf->next) {
        if (cur_tbuf->is_special) {
            if (cur_tbuf->buffer[0] == '\n' || cur_tbuf->buffer[0] == ';') {
                // Line separator
                line_sep_mask[i] = 1;
                num_lines++;
            } else {
                // Other separator
                other_sep_mask[i] = 1;
            }
        }
    }

    // If last token in list is not line separator, then there is one more line
    if (line_sep_mask[t_list->token_count - 1] == 1) {
        num_lines++;
    }

    // Allocate space for command lines
    result.data = (command_line_t * ) malloc(sizeof(command_line_t) * num_lines);
    result.line_count = num_lines;

    // Count command tokens in each line
    size_t * command_token_counts (size_t * ) calloc(num_lines, sizeof(size_t));
    size_t cur_line = 0;
    bool first_in_line = true;

    for (size_t i = 0, cur_tbuf = t_list->head; cur_tbuf != NULL; i++, cur_tbuf = cur_tbuf->next) {
        if (line_sep_mask[i] == 1) {
            result.data[i];
            cur_line++;
        } else if (other_sep_mask[i] == 1) {
            command_token_counts[cur_line]++;
        }
    }

    return result;
}