/*
Author: Gavin Witsken
Program: ish - simple shell
File: main.c
*/
#include "ish.h"
#include "lex.h"
#include "parse.h"
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

        /* 
            Stage 1: Lexical analysis (Lexing) 
            - Split string into word tokens
            - Create serialized word list
            - Pass serialized word list to parsing stage
        */

        // Perform lexical analysis to split input into word tokens
        lexedWordList = lex_str(inputStr, &lex_error, 1);
        ish_word_list_print(lexedWordList);

        // Serialize generated word list from lexing stage
        ish_serialized_word_list_t serializedWordList = ish_serialize_word_list(lexedWordList);
        // Free memory initially allocated in lexedWordList
        ish_word_list_free(&lexedWordList);

        /* 
            Stage 2: Parsing
            - Create initial pipeline from serialized word list
            - Create pipeline staging queue
            - Split initial pipeline into sub-pipelines based on pipeline terminators
            - Enqueue sub-pipelines onto staging queue to prepare for execution
        */

        ish_pipeline_t initialPipeline;
        ish_staging_queue_t stagingQueue;
        
        ish_pipeline_init(&initialPipeline, serializedWordList);
        ish_staging_queue_build_from_pipeline(&stagingQueue, &initialPipeline);

        /*
            Stage 3: Execution
            - While staging queue is not empty:
                - Fetch and dequeue first pipeline in queue
                - If pipeline is to be executed in background, create subshell, set process group to background, and continue.
                - If pipeline fully enclosed in parentheses (first component is open paren and last component is close paren), handle in pure subshell
                - For each component in pipeline:
                    - Perform variable and filename substitution
                    - For each back-quoted `subcommand` in component:
                        - Create subshell
                        - Redirect stdout from subshell into buffer in main shell
                        - Lex, parse, perform variable & command substitution, and execute command in subshell
                        - Back in main shell, replace original subcommand with redirected output from subcommand, broken into separate words at blanks, tabs, and newlines
                    - After substitution is done, validate syntax
                - Validate syntax
                - Run pipeline:
                    - If builtin command, run as builtin
                    - Else, run with execve()
                - When done, if not a background process, signal to run next pipeline in queue
                - Free shallow memory in pipeline
            - After queue is empty, free deeper memory associated with pipeline stuff (word buffers and such)
                  
        */
        
        int isBGChild = 0;
        while (stagingQueue.size != 0) {
            ish_pipeline_t * curPipeline = ish_staging_queue_fetch_and_dequeue(&stagingQueue);
            if (curPipeline == NULL) {
                break;
            }

            if (curPipeline->execMode == ISH_PIPELINE_EXECUTION_MODE_BG) {
                if (fork() == 0) {
                    // Child
                    
                } else {
                    // Parent

                }
            }
        }

        
    }

    return 0;
}