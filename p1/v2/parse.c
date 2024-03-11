/*
Author: Gavin Witsken
Program: ish - simple shell
File: parse.c
*/
#include "parse.h"

ish_serialized_word_list_t ish_serialize_word_list(const ish_word_list_t wordList) {
    const size_t word_count = wordList.word_count;
    ish_serialized_word_list_t serializedWordList;

    serializedWordList.word_count = word_count;
    serializedWordList.words = (ish_word_t *) calloc(word_count + 1, sizeof(ish_word_t));

    ish_word_list_node_t * curWordNode = wordList.head;
    for (size_t i = 0; i < word_count && curWordNode != NULL; i++) {
        // Shallow copy stuff
        serializedWordList.words[i] = curWordNode->data;

        // Deep copy strings - don't clobber original
        serializedWordList.words[i].wordBuf = calloc(serializedWordList.words[i].bufSize, sizeof(char));
        strncpy(serializedWordList.words[i].wordBuf, curWordNode->data.wordBuf, serializedWordList.words[i].wordLen);

        curWordNode = curWordNode->next;    
    }

    return serializedWordList;
}

void ish_pipeline_component_init(ish_pipeline_component_t * component) {
    component->startWordIdx = -1;
    component->endWordIdx = -1;
    component->type = ISH_PIPELINE_COMPONENT_TYPE_DEFAULT;
}

void ish_pipeline_init(ish_pipeline_t * pipeline, const ish_serialized_word_list_t serializedWordList) {
    // Initial setup
    pipeline->capacity = ISH_DEFAULT_PIPELINE_BUF_SIZE;
    pipeline->componenentCnt = 0;
    pipeline->serializedWordList = serializedWordList;
    pipeline->components = (ish_pipeline_component_t *) calloc(pipeline->capacity, sizeof(ish_pipeline_component_t));
}
void ish_pipeline_append(ish_pipeline_t * pipeline, const ish_pipeline_component_t component) {
    if (pipeline->capacity <= pipeline->componenentCnt + 1) {
        (pipeline->capacity) *= 2;
        ish_pipeline_component_t * temp = (ish_pipeline_component_t *) calloc(pipeline->capacity, sizeof(ish_pipeline_component_t));
        memcpy(temp, pipeline->components, (pipeline->componenentCnt) * sizeof(ish_pipeline_component_t));
        free(pipeline->components);
        pipeline->components = temp;
    }

    pipeline->components[(pipeline->componenentCnt)++] = component;
}
void ish_pipeline_build(ish_pipeline_t * pipeline) {
    if (pipeline == NULL || pipeline->componenentCnt <= 0) {
        return;
    }
    
    ish_pipeline_component_t curComponent;
    ish_pipeline_component_init(&curComponent);

    ish_word_t * curWord = NULL;
    curComponent.startWordIdx = 0;
    for (size_t i = 0; i < pipeline->serializedWordList.word_count; i++) {
        curWord = &(pipeline->serializedWordList.words[i]);
        curComponent.endWordIdx = i;

        if ((i = pipeline->serializedWordList.word_count - 1) || !(curWord->appendNext)) {
            ish_pipeline_append(pipeline, curComponent);
            ish_pipeline_component_init(&curComponent);
        }
    }

    // Iterate over components to identify separators

    // Iterate over components to identify non-separator components
    
}