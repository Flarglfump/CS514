/*
Author: Gavin Witsken
Program: ish - simple shell
File: lexer.c
*/

#include "lexer.h"

void token_buffer_init(token_buffer_t * buf, const size_t buf_size) {
    if (!buf) {
        return;
    }
    buf->buf_size = buf_size;
    buf->token_len = 0;
    buf->buffer = (char*) calloc(buf_size, sizeof(char));
    buf->is_special = false;
    buf->is_valid = true;
}
token_buffer_t * token_buffer_create_and_init(const size_t buf_size) {
    token_buffer_t * new_token_buf = (token_buffer_t *) malloc(sizeof(token_buffer_t));
    token_buffer_init(new_token_buf, buf_size * sizeof(char));
    return new_token_buf;
}
void token_buffer_free(token_buffer_t * buf) {
    if (buf->buffer) {
        free(buf->buffer);
    }   
}
void token_buffer_append(token_buffer_t * buf, const char ch) {
    if (buf->token_len >= buf->buf_size) {
        // Resize if needed
        size_t new_buf_size = buf->buf_size * 2;
        char* new_buf = (char*) calloc(new_buf_size, sizeof(char));
        memccpy(new_buf, buf->buffer, buf->token_len, sizeof(char));
        free(buf->buffer);
        buf->buffer = new_buf;
    }
    buf->buffer[(buf->token_len)++] = ch;
}

void token_buffer_list_init(token_buffer_list_t * list) {
    list->head = NULL;
    list->token_count = 0;
}
token_buffer_list_t * token_buffer_list_create_and_init() {
    token_buffer_list_t * list = malloc(sizeof(token_buffer_list_t));
    token_buffer_list_init(list);
    return list;
}
void token_buffer_list_free(token_buffer_list_t * list) {
    token_buffer_list_node_t * prev = NULL;
    token_buffer_list_node_t * cur = list->head;
    while (cur != NULL) {
        token_buffer_free(&(cur->token_buf));

        prev = cur;
        cur = cur->next;
        if (prev) {
            free(prev);
        }    
    }
}
void token_buffer_list_append(token_buffer_list_t * list, const token_buffer_t token_buf) {
    token_buffer_list_node_t * new_node = (token_buffer_list_node_t *) malloc(sizeof(token_buffer_list_node_t));
    new_node->token_buf = token_buf;

    if (!list->head) {
        list->head = new_node;
    } else {
        token_buffer_list_node_t * cur_node = list->head;
        while (cur_node->next) {
            cur_node = cur_node->next;
        }
        cur_node->next = new_node;
    }
    (list->token_count)++;
}


bool is_special_char(const char ch) {
    return (ch == '<' || ch == '>' || ch == '&' || ch == '|'|| ch == ';' || ch == '\n');
}

bool is_whitespace_sep(const char ch) {
    return (ch == ' ' || ch == '\t');
}

char peek1(const char * const str, const int cur_pos, const size_t max_size) {
    int idx = cur_pos + 1;
    if (idx >= max_size) {
        return -1;
    }
    return str[idx];
}
char peek2(const char* const str, const int cur_pos, const size_t max_size) {
    int idx = cur_pos + 2;
    if (idx >= max_size) {
        return -1;
    }
    return str[idx];
}

token_buffer_list_t lex_file(const char * fname) {
    token_buffer_list_t t_buf_list;
    return t_buf_list;
}

token_buffer_list_t lex_str(const char * const str) {
    // First pass: lex line into sequential list of initial tokens; Only classify by special/non-special string
    int cur_pos;
    bool open_double_quote = false, open_single_quote = false;
    char cur_char = 0;

    token_buffer_list_t t_buf_list;
    token_buffer_list_init(&t_buf_list);

    token_buffer_t t_buf;
    const size_t TOKEN_BUF_SIZE = 2048;
    token_buffer_init(&t_buf, TOKEN_BUF_SIZE);

    if (!str || !str[0]) {
        fprintf(stderr, "Error: ish: parse_line: line text provided is not valid");
        return t_buf_list;
    }
    size_t char_count = strlen(str);
    if (char_count <= 0) {
        fprintf(stderr, "Error: ish: parse_line: line text provided is not valid");
        return t_buf_list;
    }

    for (cur_pos = 0; cur_pos < char_count; cur_pos++) {
        cur_char = str[cur_pos];
        // printf("\tCurrent char: %c\n", cur_char);

        // Handle strings within single or double quotes
        if (open_single_quote) {
            // Inside single quote string - No escaped characters
            if (cur_char == '\'') {
                // End quote string
                open_single_quote = false;
            } else {
                // Add character as literal
                token_buffer_append(&t_buf, cur_char);
            }
        } else if (open_double_quote) {
            // Inside double quote string - Have to account for escaped double quote  
            if (cur_char == '\\') {
                char next_char = peek1(str, cur_pos, char_count);
                if (next_char == '"') {
                    // Escaped double quote char - add to token and advance extra position
                    token_buffer_append(&t_buf, next_char);
                    cur_pos++;
                } else {
                    // Add backslash char to token
                    token_buffer_append(&t_buf, cur_char);
                }
            } else if (cur_char == '"') {
                // Unescaped double quote char - end double quote string
                open_double_quote = false;
            }
            else {
                // Add current char to token
                token_buffer_append(&t_buf, cur_char);
            }
        } else {
            // Handle other cases
            if (cur_char == '\\') {
                // Next character should be escaped
                char next_char = peek1(str, cur_pos, char_count);
                if (next_char != -1) {
                    // Character is valid, add to token
                    token_buffer_append(&t_buf, next_char);
                    cur_pos++;
                } // Else escape char is at end of command line - just ignore it
            } else if (cur_char == '\'') {
                // Should open single quote string
                open_single_quote = true;
            } else if (cur_char == '"') {
                // Should open double quote string
                open_double_quote = true;
            } else if (is_whitespace_sep(cur_char)) {
                // Add current token to list and initialize new token if size is > 0.
                if (t_buf.token_len > 0) {
                    token_buffer_list_append(&t_buf_list, t_buf);
                    token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                }
            } else if (is_special_char(cur_char)) {
                if (cur_char == ';' || cur_char == '\n' || cur_char == '<' || cur_char == '|' || cur_char == '&') {
                    // Treat as single-char separator token
                    // Add existing token if size is > 0
                    if (t_buf.token_len > 0) {
                        token_buffer_list_append(&t_buf_list, t_buf);
                        token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                    }

                    // Create new token and fill with current char
                    token_buffer_append(&t_buf, cur_char);
                    t_buf.is_special = true;
                    token_buffer_list_append(&t_buf_list, t_buf);

                    // Init new token buff
                    token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                } else if (cur_char == '>') {
                    // Could be 1, 2 or 3 character special string
                    char next_char = peek1(str, cur_pos, char_count);

                    if (next_char == '&') {
                        // 2 char string: >&
                        if (t_buf.token_len > 0) {
                            token_buffer_list_append(&t_buf_list, t_buf);
                            token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                        }

                        // Create new token and fill with current and next
                        token_buffer_append(&t_buf, cur_char);
                        token_buffer_append(&t_buf, next_char);
                        t_buf.is_special = true;
                        token_buffer_list_append(&t_buf_list, t_buf);

                        // Init new token buff
                        token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                        cur_pos++;
                    } else if (next_char == '>') {
                        // Could be >> or >>&
                        char next_next_char = peek2(str, cur_pos, char_count);

                        if (next_next_char == '&') {
                            // 3 char string: >>&
                            if (t_buf.token_len > 0) {
                                token_buffer_list_append(&t_buf_list, t_buf);
                                token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                            }

                            // Create new token and fill with current and next 2 chars
                            token_buffer_append(&t_buf, cur_char);
                            token_buffer_append(&t_buf, next_char);
                            token_buffer_append(&t_buf, next_next_char);
                            t_buf.is_special = true;
                            token_buffer_list_append(&t_buf_list, t_buf);

                            // Init new token buff
                            token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                            cur_pos += 2;
                        } else {
                            // 2 char string: >>
                            if (t_buf.token_len > 0) {
                                token_buffer_list_append(&t_buf_list, t_buf);
                                token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                            }

                            // Create new token and fill with current char and next char
                            token_buffer_append(&t_buf, cur_char);
                            token_buffer_append(&t_buf, next_char);
                            t_buf.is_special = true;
                            token_buffer_list_append(&t_buf_list, t_buf);

                            // Init new token buff
                            token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                            cur_pos++;
                        }
                    } else {
                        // Single character special string
                        if (t_buf.token_len > 0) {
                            token_buffer_list_append(&t_buf_list, t_buf);
                            token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                        }

                        // Create new token and fill with current char
                        token_buffer_append(&t_buf, cur_char);
                        t_buf.is_special = true;
                        token_buffer_list_append(&t_buf_list, t_buf);

                        // Init new token buff
                        token_buffer_init(&t_buf, TOKEN_BUF_SIZE);
                    }
                }
            } else {
                // Read as simple nonspecial character
                token_buffer_append(&t_buf, cur_char);
            }
        }

        // printf("\tCurrent token: %s\n", t_buf.buff);
    }

    // If open single or double quote, then token is invalid; Don't want to invalidate whole list, just command in which argument lives
    if (open_double_quote || open_single_quote) {
        t_buf.is_valid = false;
    }

    // End of input - if current token size greater than 0, add to list, else just free data allocated in buffer
    if (t_buf.token_len > 0) {
        token_buffer_list_append(&t_buf_list, t_buf);
    } else {
        token_buffer_free(&t_buf);
    }

    return t_buf_list;
}

void token_buffer_list_print(const token_buffer_list_t list) {
    printf("Number of tokens: %lu\n", list.token_count);
    token_buffer_list_node_t * cur_node = list.head;

    unsigned int i = 0;
    while (cur_node != NULL) {
        printf("\t%d (%s): %s\n", i, cur_node->token_buf.is_special ? "special" : "not special", cur_node->token_buf.buffer);
        cur_node = cur_node->next;
        i++;
    }
}