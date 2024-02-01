/*
Author: Gavin Witsken
Program: ish - simple shell
File: parser.c
Created: 12:45 PM, 31 Jan 2024
Modified: 12:45 PM, 31 Jan 2024
*/

#include "parser.h"

void token_buffer_init(token_buffer * buf, const size_t buf_size) {
    if (!buf) {
        return;
    }
    buf->buf_size = buf_size;
    buf->token_len = 0;
    buf->buff = (char*) calloc(buf_size, sizeof(char));
}
token_buffer * token_buffer_create_and_init(const size_t buf_size) {
    token_buffer * new_token_buf = (token_buffer *) malloc(sizeof(token_buffer));
    token_buffer_init(new_token_buf, buf_size * sizeof(char));
    return new_token_buf;
}
void token_buffer_free(token_buffer * buf) {
    if (buf->buff) {
        free(buf->buff);
    }   
}
void token_buffer_append(token_buffer * buf, const char ch) {
    if (buf->token_len >= buf->buf_size) {
        // Resize if needed
        size_t new_buf_size = buf->buf_size * 2;
        char* new_buf = (char*) calloc(new_buf_size, sizeof(char));
        memccpy(new_buf, buf->buff, buf->token_len, sizeof(char));
        free(buf->buff);
        buf->buff = new_buf;
    }
    buf->buff[(buf->token_len)++] = ch;
}

token_buffer_list_init(token_buffer_list * list) {
    list->head = NULL;
    list->token_count = 0;
}
token_buffer_list * token_buffer_list_create_and_init() {
    token_buffer_list * list = malloc(sizeof(token_buffer_list));
    token_buffer_list_init(list);
    return list;
}
token_buffer_list_free(token_buffer_list * list) {
    token_buffer_list_node * prev = NULL;
    token_buffer_list_node * cur = list->head;
    while (cur != NULL) {
        token_buffer_free(&(cur->token_buf));

        prev = cur;
        cur = cur->next;
        if (prev) {
            free(prev);
        }    
    }
}
void token_buffer_list_append(token_buffer_list * list, const token_buffer token_buf) {
    token_buffer_list_node * cur_node = list->head, * new_node = (token_buffer_list_node *) malloc(sizeof(token_buffer_list_node));
    new_node->token_buf = token_buf;
    while (cur_node->next) {
        cur_node = cur_node->next;
    }

    cur_node->next = new_node;
}


bool is_special_char(const char ch) {
    return (ch == '<' || ch == '>' || ch == '&' || ch == ';');
}

command_line_list parse_file(const char * fname) {
    command_line_list list;
    list.lines == NULL;  
    FILE * in;
    if (!fname || !fname[0]) {
        fprintf(stderr, "Error: ish: parse_file: file name provided is not valid", fname);
        return list;
    }

    fopen(fname, "r");
    if (!in) {
        fprintf(stderr, "Error: ish: parse_file: could not open file \"%s\" for reading\n", fname);
        return list;
    }

    return list;
}

command_line parse_line(const char * str) {
    command_line cmd_line;
    cmd_line.commands = NULL;
    cmd_line.arg_count = 0;
    if (!str || !str[0]) {
        fprintf(stderr, "Error: ish: parse_line: line text provided is not valid");
        return cmd_line;
    }

    size_t char_count = strlen(str);
    if (char_count <= 0) {
        fprintf(stderr, "Error: ish: parse_line: line text provided is not valid");
        return cmd_line;
    }

    // First pass: parse line into sequential list of tokens
    int cur_pos = 0;
    bool open_double_quote = false, open_single_quote = false, in_string = false, in_special_string = false;
    char cur_char = 0;

    token_buffer_list buffer_list;
    token_buffer_list_init(&buffer_list);

    token_buffer buff;
    const size_t TOKEN_BUF_SIZE = 2048;
    token_buffer_init(&buff, TOKEN_BUF_SIZE);

    for (; cur_pos < char_count; ++cur_pos) {
        cur_char = str[cur_pos];

        if (!in_string) { 
            // Token not currently being constructed
            if (cur_char == ' ' || cur_char == '\t') { 
                // whitespace separator - ignore
                continue;
            }

            if (is_special_char(cur_char)) { 
                if (open_double_quote || open_single_quote) {
                    // Special char is part of token - add character to buffer
                    token_buffer_append(&buff, cur_char);
                } else {
                    // Special char is not part of token - end of current token; start of special string
                    token_buffer_list_append(&buffer_list, buff);

                    in_special_string = true;
                    token_buffer_init(&buff, TOKEN_BUF_SIZE);
                }
            }
        } else { 
            // Token currently being constructed

        }
    }

    return cmd_line;
}