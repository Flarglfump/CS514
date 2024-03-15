/*
Author: Gavin Witsken
Program: ish - simple shell
File: lex.c
*/

#include "lex.h"
#include <stdio.h>

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

    (list->word_count)++;
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

ish_word_list_t lex_str(const char * str, ish_lex_error_t * lex_error, const int is_interactive) {
    ish_word_list_t wordList;
    ish_word_list_init(&wordList);

    ish_word_t curWord;
    ish_word_init(&curWord);
    
    size_t inputLen = strlen(str);
    size_t curPos = 0;
    
    char curChar = '\0';
    ish_quote_type_t curQuoteType = ISH_QUOTE_TYPE_NONE;

    int hereFileMode = 0;

    while (curPos < inputLen) {
        curChar = str[curPos];

        /* Note on quoted sequences in input: 
            This pass will not handle parsing nested backslash escape sequences within other quoted sequences. 
        */
        if (curQuoteType == ISH_QUOTE_TYPE_DOUBLE) {
            // Inside double quoted "string" - Allows for variable and command substitution; Only escapes a few chars with backlash
            if (curChar == '"') {
                // End of double quote string
                const char nextChar = peek1(str, inputLen, curPos);
                if (!(is_blank(nextChar) || nextChar == '\0' || is_special_char(nextChar))) {
                    curWord.appendNext = 1;
                }
                ish_word_trim_allocation(&curWord);
                ish_word_list_append(&wordList, curWord);
                ish_word_init(&curWord);

                curQuoteType = ISH_QUOTE_TYPE_NONE;
            } else if (curChar == '\\') {
                const char nextChar = peek1(str, inputLen, curPos);
                const char nextNextChar = peek2(str, inputLen, curPos);
                if (nextChar == '$' || nextChar == '"' || nextChar == '\\' || nextChar == '`') {
                    // Valid escape sequence
                    if (curWord.wordLen > 0) {
                        curWord.appendNext = 1;
                        ish_word_trim_allocation(&curWord);
                        ish_word_list_append(&wordList, curWord);
                        ish_word_init(&curWord);
                    }

                    curWord.quoteType = ISH_QUOTE_TYPE_DOUBLE_ESCAPED;
                    if (nextNextChar != '"') {
                        curWord.appendNext = 1;
                    }
                    curPos++;
                    ish_word_append(&curWord, nextChar);
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);

                    ish_word_init(&curWord);
                    if (nextNextChar != '"') {
                        curWord.quoteType = ISH_QUOTE_TYPE_DOUBLE;
                    }   
                } else {
                    // Not a recognized escape sequence - just another character in string
                    ish_word_append(&curWord, curChar);
                }
            } else {
                // Character in string
                ish_word_append(&curWord, curChar);
            }
        } else if (curQuoteType == ISH_QUOTE_TYPE_SINGLE) {
            // Inside single quoted 'string' - no escape sequences or special chars, except backslash escaped newline
            if (curChar == '\'') {
                // Potential end of single quote string
                const char nextChar = peek1(str, inputLen, curPos);
                if (nextChar == '\n') {
                    // Add newline char to string
                    curPos++;
                    ish_word_append(&curWord, nextChar);
                } else {
                    // End of single quote string
                    if (!(is_blank(nextChar) || nextChar != '\0' || is_special_char(nextChar))) {
                        curWord.appendNext = 1;
                    }
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);

                    curQuoteType = ISH_QUOTE_TYPE_NONE;
                }   
            } else {
                // Character in string
                ish_word_append(&curWord, curChar);
            }
        } else if (curQuoteType == ISH_QUOTE_TYPE_BACKTICK) {
            // Inside back tick quoted `string` - Allows for variable and command substitution, backslash escaping works the same as if unquoted
            if (curChar == '`') {
                // End of backtick quoted string
                ish_word_trim_allocation(&curWord);
                ish_word_list_append(&wordList, curWord);
                ish_word_init(&curWord);
                
                curQuoteType = ISH_QUOTE_TYPE_NONE;
            } else if (curChar == '\\') {
                // Inside backslash quoted \string - escape curChar
                const char nextChar = peek1(str, inputLen, curPos);
                const char nextNextChar = peek2(str, inputLen, curPos);

                curWord.appendNext = 1;
                ish_word_trim_allocation(&curWord);
                ish_word_list_append(&wordList, curWord);
                ish_word_init(&curWord);

                curWord.quoteType = ISH_QUOTE_TYPE_BACKTICK_ESCAPED;
                if (nextNextChar != '`') {
                    curWord.appendNext = 1;
                }
                ish_word_append(&curWord, nextChar);
                
                curPos++;
                ish_word_trim_allocation(&curWord);
                ish_word_list_append(&wordList, curWord);

                ish_word_init(&curWord);
                if (nextNextChar != '`') {
                    curWord.quoteType = ISH_QUOTE_TYPE_BACKTICK;
                }
            }
            
        } else if (curQuoteType == ISH_QUOTE_TYPE_BACKSLASH) {
            // Inside backslash quoted \string - escape curChar
            const char nextChar = peek1(str, inputLen, curPos);

            ish_word_append(&curWord, curChar);
            if (!(is_blank(nextChar) || nextChar != '\0' || is_special_char(nextChar))) {
                curWord.appendNext = 1;
            }
            ish_word_trim_allocation(&curWord);
            ish_word_list_append(&wordList, curWord);
            ish_word_init(&curWord);

            curQuoteType = ISH_QUOTE_TYPE_NONE;
        } else {
            // Not inside quoted string
            if (curChar == '"') {
                // Opening double quote
               curQuoteType = ISH_QUOTE_TYPE_DOUBLE;

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_DOUBLE;
            } else if (curChar == '\'') {
                // Opening single quote
                curQuoteType = ISH_QUOTE_TYPE_SINGLE;

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_SINGLE;
            } else if (curChar == '`') {
                // Opening back quote
                curQuoteType = ISH_QUOTE_TYPE_BACKTICK;

                if (curWord.wordLen > 0) {
                    curWord.appendNext = 1;
                    ish_word_trim_allocation(&curWord);
                    ish_word_list_append(&wordList, curWord);
                    ish_word_init(&curWord);
                }

                curWord.quoteType = ISH_QUOTE_TYPE_BACKTICK;
            } else if (curChar == '\\') {
                // Opening back slash
                curQuoteType = ISH_QUOTE_TYPE_BACKSLASH;

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
                        if (curWord.wordLen > 0) {
                            ish_word_trim_allocation(&curWord);
                            ish_word_list_append(&wordList, curWord);
                            ish_word_init(&curWord);
                        }

                        ish_word_append(&curWord, curChar);
                        ish_word_trim_allocation(&curWord);
                        curPos++;
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
                    const char nextChar = peek1(str, inputLen, curPos);
                    const char nextNextChar = peek2(str, inputLen, curPos);
                    const char nextNextNextChar = peek3(str, inputLen, curPos);

                    // Other special character
                    switch(curChar) {
                        case '&': // Background operator. Treat as single char word.   
                        case ';': // Pipeline terminator operator. Treat as single char word. 
                        case '(': // Open paren operator. Treat as single char word. 
                        case ')': // Closing paren operator. Treat as single char word.
                        case '<': // Redirect input operator. Currently, herefile operator '<<' not implemented, so treat '<' as a single char word.
                            if (curWord.wordLen > 0) {
                                ish_word_trim_allocation(&curWord);
                                ish_word_list_append(&wordList, curWord);
                                ish_word_init(&curWord);
                            }
                            ish_word_append(&curWord, curChar);
                            ish_word_trim_allocation(&curWord);
                            ish_word_list_append(&wordList, curWord);

                            ish_word_init(&curWord);
                        break;

                        case '#':
                            // If non-terminal (not interactive) input, rest of line should be treated as a comment and ignored
                            if (!is_interactive) {
                                if (curWord.wordLen > 0) {
                                    ish_word_trim_allocation(&curWord);
                                    ish_word_list_append(&wordList, curWord);
                                    ish_word_init(&curWord);
                                }
                                while (curPos < inputLen && curChar != '\n') {
                                    curPos++;
                                    curChar = str[curPos];
                                }
                            }    
                        break;

                        case '|':
                            // '|', '||', or '|&'
                            if (curWord.wordLen > 0) {
                                ish_word_trim_allocation(&curWord);
                                ish_word_list_append(&wordList, curWord);
                                ish_word_init(&curWord);
                            }

                            ish_word_append(&curWord, curChar);
                            if (nextChar == '|' || nextChar == '&') {
                                // '||' or '|&'
                                ish_word_append(&curWord, nextChar);
                            }

                            ish_word_trim_allocation(&curWord);
                            ish_word_list_append(&wordList, curWord);
                            curPos += curWord.wordLen - 1;
                            ish_word_init(&curWord);
                        break;     

                        case '>':
                            // '>', '>!', '>&', '>&!', '>>', '>>!', '>>&', or '>>&!'
                            if (curWord.wordLen > 0) {
                                ish_word_trim_allocation(&curWord);
                                ish_word_list_append(&wordList, curWord);
                                ish_word_init(&curWord);
                            }

                            ish_word_append(&curWord, curChar);
                            if (nextChar == '>') {
                                // '>>', '>>!', '>>&', or '>>&!'
                                ish_word_append(&curWord, nextChar);
                                if (nextNextChar == '!') {
                                    // '>>!'
                                    ish_word_append(&curWord, nextNextChar);
                                } else if (nextNextChar == '&') {
                                    // '>>&' or '>>&!'
                                    ish_word_append(&curWord, nextNextChar);
                                    if (nextNextNextChar == '!') {
                                        // '>>&!'
                                        ish_word_append(&curWord, nextNextNextChar);
                                    }
                                }
                            } else if (nextChar == '!') {
                                // '>!'
                                ish_word_append(&curWord, nextChar);
                            } else if (nextChar == '&') {
                                // '>&' or '>&!'
                                ish_word_append(&curWord, nextChar);
                                if (nextNextChar == '!') {
                                    // '>&!
                                    ish_word_append(&curWord, nextNextChar);
                                }
                            }

                            ish_word_trim_allocation(&curWord);
                            ish_word_list_append(&wordList, curWord);
                            curPos += curWord.wordLen - 1;
                            ish_word_init(&curWord);
                        break;
                    }
                } else {
                    // Not a special character or other separator - add to word
                    ish_word_append(&curWord, curChar);
                }
            }
        }

        curPos++;
    }

    // If any words left at the end, add them to the word list
    if (curWord.wordLen > 0) {
        ish_word_trim_allocation(&curWord);
        ish_word_list_append(&wordList, curWord);
    }

    // Set lexing error status
    switch(curQuoteType) {
        case ISH_QUOTE_TYPE_BACKSLASH:
            *lex_error = ISH_LEX_ERR_OPEN_BACKSLASH;
        break;
        case ISH_QUOTE_TYPE_DOUBLE:
            *lex_error = ISH_LEX_ERR_OPEN_DOUBLE_QUOTE;
        break;
        case ISH_QUOTE_TYPE_SINGLE:
            *lex_error = ISH_LEX_ERR_OPEN_SINGLE_QUOTE;
        break;
        case ISH_QUOTE_TYPE_BACKTICK:
            *lex_error = ISH_LEX_ERR_OPEN_BACK_QUOTE;
        break;
        default:
            *lex_error = ISH_LEX_ERR_NONE;
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
char peek3(const char * str, const size_t len, const int position) {
    if (position >= len - 3) {
        return '\0';
    }
    return str[position + 3];
}

int is_blank(const char c) {
    return (c == ' ' || c == '\t');
}
int is_special_char(const char c) {
    return (
            c == '&'
        ||  c == '#'
        ||  c == '|'
        ||  c == ';' 
        ||  c == '<' 
        ||  c == '>' 
        ||  c == '('
        ||  c == ')'
    );
}

void ish_word_list_print(const ish_word_list_t wordList) {
    printf("Word list (%lu words):\n", wordList.word_count);
    int printIdx = 1;
    ish_word_list_node_t * curWord = wordList.head;

    for (size_t i = 0; i < wordList.word_count && curWord != NULL; i++) {
        // Unquoted = default
        // Double Quoted = RED
        // Single Quoted = BLUE
        // Backtick Quoted = GREEN
        // Backslash Quoted = BOLD

        printf("\e[0m");
        if (printIdx) {
            printf("%lu:\t", i);
        }

        switch(curWord->data.quoteType) {
            case ISH_QUOTE_TYPE_DOUBLE:
                printf("\e[0;31m");
            break;
            case ISH_QUOTE_TYPE_DOUBLE_ESCAPED:
                printf("\e[1;31m");
            break;
            case ISH_QUOTE_TYPE_SINGLE:
                printf("\e[0;34m");
            break;
            case ISH_QUOTE_TYPE_BACKTICK:
                printf("\e[0;32m");
            break;
            case ISH_QUOTE_TYPE_BACKTICK_ESCAPED:
                printf("\e[1;32m");
            break;
            case ISH_QUOTE_TYPE_BACKSLASH:
                printf("\e[1;0m");
            break;
            case ISH_QUOTE_TYPE_NONE:
            default:
                printf("\e[0m");
        }
        
        printf("%s", curWord->data.wordBuf);

        if (!(curWord->data.appendNext)) {
            printIdx = 1;
            printf("\n");
        } else {
            printIdx = 0;
        }

        curWord = curWord->next;
    }

    printf("\e[0m\n");
}