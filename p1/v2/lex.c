/*
Author: Gavin Witsken
Program: ish - simple shell
File: lex.c
*/

#include "lex.h"

/* ish stack stuff*/

void ish_stack_init(ish_stack_t * stack) {
    stack->capacity = ISH_DEFAULT_BUF_SIZE;
    stack->data = calloc(stack->capacity, sizeof(ish_stack_elem_t));
    stack->len = 0;
}
void ish_stack_pop(ish_stack_t * stack) {
    if (stack->len > 0) {
        (stack->len)--;
    }
}
void ish_stack_push(ish_stack_t * stack, const ish_stack_elem_t val) {
    if (stack->len >= stack->capacity - 1) {
        (stack->capacity) *= 2;
        ish_stack_elem_t * newArr = calloc(stack->capacity, sizeof(ish_stack_elem_t));
        for (size_t i = 0; i < stack->len; i++) {
            newArr[i] = stack->data[i];
        }
        free(stack->data);
        stack->data = newArr;
    }
    stack->data[(stack->len)] = val;
    (stack->len)++;
}
ish_stack_elem_t ish_stack_top(ish_stack_t * stack) {
    if (stack->len > 0) {
        return (stack->data[stack->len - 1]);
    }

    return -1;
}
void ish_stack_free(ish_stack_t * stack) {
    if (stack->data != NULL) {
        free(stack->data);
    }
}

/* ish word stuff */

void ish_word_init(ish_word_t * word) {
    word->bufSize = ISH_DEFAULT_BUF_SIZE;
    word->wordLen = 0;
    word->wordBuf = (char *) calloc(ISH_DEFAULT_BUF_SIZE, sizeof(char));

    word->appendNext = 0;
    word->quoteType = ISH_QUOTE_TYPE_NONE;
}
void ish_word_append(ish_word_t * word, const char c) {
    if (word->wordLen >= word->bufSize - 1) {
        char * temp = (char *) calloc(word->bufSize * 2, sizeof(char));
        strncpy(temp, word->wordBuf, word->bufSize);
        free(word->wordBuf);
        word->wordBuf = temp;
    }
    word->wordBuf[word->wordLen++] = c;
    word->wordBuf[word->wordLen] = '\0';
}
void ish_word_trim_allocation(ish_word_t * word) {
    if (word->wordLen > 0 && word->wordBuf != NULL) {
        char * temp = (char *) calloc(word->wordLen + 1, sizeof(char));
        strncpy(temp, word->wordBuf, word->wordLen + 1);
        free (word->wordBuf);
        word->wordBuf = temp;
    }
}
void ish_word_free(ish_word_t * word) {
    if (word->wordBuf != NULL) {
        free(word->wordBuf);
    }
}

/* ish word list stuff */

void ish_word_list_init(ish_word_list_t * list) {
    list->word_count = 0;
    list->head = NULL;
    list->tail = NULL;
}
void ish_word_list_append(ish_word_list_t * list, const ish_word_t word) {
    ish_word_list_node_t * newNode = calloc(1, sizeof(ish_word_list_node_t));
    newNode->data = word;
    newNode->next = NULL;
    if (list->tail == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}
void ish_word_list_free(ish_word_list_t * list) {
    ish_word_list_node_t * cur = list->head;
    ish_word_list_node_t * prev = NULL;
    while (cur != NULL) {
        ish_word_free(&(cur->data));
        if (prev != NULL) {
            free(prev);
        }
        prev = cur;
        cur = cur->next;
    }
    if (prev != NULL) {
        free(prev);
    }
}

/* Actual lexing stuff */

ish_word_list_t lex_str(const char * str) {
    ish_word_list_t wordList;
    ish_word_list_init(&wordList);

    ish_word_t curWord;
    ish_word_init(&curWord);

    ish_stack_t stack;
    ish_stack_init(&stack);
    
    size_t inputLen = strlen(str);
    size_t curPos = 0;
    
    char curChar = '\0';
    ish_quote_type_t curQuoteType = ISH_QUOTE_TYPE_NONE;

    while (curPos < inputLen) {
        curChar = str[curPos];
        if (stack.len > 0) {
            curQuoteType = ish_stack_top(&stack);
        }

        if (curQuoteType == ISH_QUOTE_TYPE_DOUBLE) {
            // Inside double quote "string"

        } else if (curQuoteType == ISH_QUOTE_TYPE_SINGLE) {
            // Inside single quote 'string'

        } else if (curQuoteType == ISH_QUOTE_TYPE_BACKTICK) {
            // Inside back quote `string`

        } else if (curQuoteType == ISH_QUOTE_TYPE_BACKSLASH) {
            // Inside backslash quoted \string

        } else {
            // Not inside quoted string
            if (curChar == '"') {
                // Opening double quote
                ish_stack_push(&stack, ISH_QUOTE_TYPE_DOUBLE);

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_DOUBLE;
            } else if (curChar == '\'') {
                // Opening single quote
                ish_stack_push(&stack, ISH_QUOTE_TYPE_SINGLE);

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_SINGLE;
            } else if (curChar == '`') {
                // Opening back quote
                ish_stack_push(&stack, ISH_QUOTE_TYPE_BACKTICK);

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_BACKTICK;
            } else if (curChar == '\\') {
                // Opening back slash
                ish_stack_push(&stack, ISH_QUOTE_TYPE_BACKSLASH);

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_BACKSLASH;
            } else {
                // Not opening a quoted string
                if (is_blank(curChar)) {
                    // Current character is a blank - indicates word separator
                    if (curWord.wordLen > 0) {
                        ish_word_trim_allocation(&curWord);
                        ish_word_list_append(&wordList, curWord);
                        ish_word_init(&curWord);
                    }
                } else if (curChar == '\r') {
                    const char nextChar = peek1(str, inputLen, curPos);
                    if (nextChar == '\n') {
                        // Windows style newline - skip a character forward in curPos
                        curPos++;  
                        if (curWord.wordLen > 0) {
                            ish_word_trim_allocation(&curWord);
                            ish_word_list_append(&wordList, curWord);
                            ish_word_init(&curWord);
                        }

                        ish_word_append(&curWord, curChar);
                        ish_word_trim_allocation(&curWord);
                        ish_word_list_append(&wordList, curWord);

                        ish_word_init(&curWord);
                    } else {
                        // Treat as blank - csh man page does not specify behavior
                        if (curWord.wordLen > 0) {
                            ish_word_trim_allocation(&curWord);
                            ish_word_list_append(&wordList, curWord);
                            ish_word_init(&curWord);
                        }
                    }
                } else if (curChar == '\n') {
                    // Unix style newline
                    if (curWord.wordLen > 0) {
                        ish_word_trim_allocation(&curWord);
                        ish_word_list_append(&wordList, curWord);
                        ish_word_init(&curWord);
                    }
                    ish_word_append(&curWord, curChar);
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);

                    ish_word_init(&curWord);
                } else if (is_special_char(curChar)) {
                    // 


                }
            }
        }


        curPos++;
    }

    return wordList;
}
char peek1(const char * str, const size_t len, const int position) {
    if (position >= len - 1) {
        return '\0';
    }
    return str[position + 1];
}
char peek2(const char * str, const size_t len, const int position) {
    if (position >= len - 2) {
        return '\0';
    }
    return str[position + 2];
}

int is_blank(const char c) {
    return (c == ' ' || c == '\t');
}
int is_special_char(const char c) {
    return (c == '|' 
        ||  c == '<' 
        ||  c == '>' 
        ||  c == '<'
    );
}
/*
Special chars:
    '&',
    '|',
    ';',
    '<',
    '>',
    '(',
    ')',
    '&&',
    '||',
    '<<'
    '>>'
*/