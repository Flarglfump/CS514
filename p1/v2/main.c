/*
Author: Gavin Witsken
Program: ish - simple shell
File: main.c
*/
#include "ish.h"
#include "lex.h"
#include <stdio.h>

int main(int argc, char * argv[]) {

    char * inputStr = NULL;
    size_t inputBufSize;
    size_t inputStrLen = 0;
    ish_word_list_t lexedWordList;
    ish_lex_error_t lex_error = ISH_LEX_ERR_NONE;

    while (1) {
        inputStrLen = getline(&inputStr, &inputBufSize, stdin);
        printf("Characters read: %lu\n", inputStrLen);
        printf("Buffer size: %lu\n", inputBufSize);

        lexedWordList = lex_str(inputStr, &lex_error, 1);

        ish_word_list_print(lexedWordList);

        ish_word_list_free(&lexedWordList);
    }

    return 0;
}