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
    component->componentType = ISH_PIPELINE_COMPONENT_TYPE_DEFAULT;
    component->sepType = ISH_SEP_TYPE_NONE;
}

void ish_pipeline_init(ish_pipeline_t * pipeline, const ish_serialized_word_list_t serializedWordList) {
    // Initial setup
    pipeline->capacity = ISH_DEFAULT_PIPELINE_BUF_SIZE;
    pipeline->componentCnt = 0;
    pipeline->serializedWordList = serializedWordList;
    pipeline->execMode = ISH_PIPELINE_EXECUTION_MODE_SEQ;
    pipeline->components = (ish_pipeline_component_t *) calloc(pipeline->capacity, sizeof(ish_pipeline_component_t));
}
void ish_pipeline_append(ish_pipeline_t * pipeline, const ish_pipeline_component_t component) {
    if (pipeline->capacity <= pipeline->componentCnt + 1) {
        (pipeline->capacity) *= 2;
        ish_pipeline_component_t * temp = (ish_pipeline_component_t *) calloc(pipeline->capacity, sizeof(ish_pipeline_component_t));
        memcpy(temp, pipeline->components, (pipeline->componentCnt) * sizeof(ish_pipeline_component_t));
        free(pipeline->components);
        pipeline->components = temp;
    }

    pipeline->components[(pipeline->componentCnt)++] = component;
}

void ish_staging_queue_build_from_pipeline(ish_staging_queue_t * stagingQueue, ish_pipeline_t * pipeline) {
    if (pipeline == NULL || pipeline->componentCnt <= 0) {
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

    // Iterate over components to label separators and split into different pipelines
    int componentStartIdx = 0;
    int componentEndIdx = -1;
    ish_pipeline_t curPipeline;

    for (size_t i = 0; i < pipeline->componentCnt; i++) {
        ish_pipeline_component_t * componentPtr = &(pipeline->components[i]);
        if (componentPtr->componentType == ISH_PIPELINE_COMPONENT_TYPE_DEFAULT &&
           (componentPtr->startWordIdx == componentPtr->endWordIdx) &&
           (pipeline->serializedWordList.words[componentPtr->startWordIdx].quoteType == ISH_QUOTE_TYPE_NONE)) {
            // Unquoted, single-word token. Should be separator.
            ish_word_t * wordPtr = &((pipeline->serializedWordList.words)[componentPtr->startWordIdx]);
            componentPtr->sepType = get_ish_sep_type(wordPtr->wordBuf, wordPtr->wordLen);
            if (componentPtr->sepType != ISH_SEP_TYPE_NONE) {
                componentPtr->componentType = ISH_PIPELINE_COMPONENT_TYPE_SEPARATOR;
            } else if (componentPtr->sepType == ISH_SEP_TYPE_BG) {
                componentEndIdx = i - 1; // Last component is 1 before terminating separator

                curPipeline = ish_pipeline_create_sub_pipeline(pipeline, componentStartIdx, componentEndIdx);
                curPipeline.execMode = ISH_PIPELINE_EXECUTION_MODE_BG;

                ish_staging_queue_enqueue(stagingQueue, curPipeline);

                componentStartIdx = i + 1;
            } else if (componentPtr->sepType == ISH_SEP_TYPE_PIPELINE_TERM) {
                componentEndIdx = i - 1; // Last component is 1 before terminating separator

                curPipeline = ish_pipeline_create_sub_pipeline(pipeline, componentStartIdx, componentEndIdx);
                curPipeline.execMode = ISH_PIPELINE_EXECUTION_MODE_SEQ;

                ish_staging_queue_enqueue(stagingQueue, curPipeline);

                componentStartIdx = i + 1;
            }
        }
    }
    // If command line not properly terminated, assume line to terminate at last component in sequential mode
    if (componentStartIdx < pipeline->componentCnt) {
        componentEndIdx = pipeline->componentCnt - 1;

        curPipeline = ish_pipeline_create_sub_pipeline(pipeline, componentStartIdx, componentEndIdx);

        ish_staging_queue_enqueue(stagingQueue, curPipeline);
    }

}

enum ish_sep_type get_ish_sep_type(const char * str, const size_t len) {
    switch(len) {
        case 1:
            return strncmp(str, ">", len) == 0 ? ISH_SEP_TYPE_REDIR_OUT :
                strncmp(str, "<", len) == 0 ? ISH_SEP_TYPE_REDIR_IN :
                strncmp(str, "(", len) == 0 ? ISH_SEP_TYPE_OPEN_PAREN :
                strncmp(str, ")", len) == 0 ? ISH_SEP_TYPE_CLOSE_PAREN :
                strncmp(str, "&", len) == 0 ? ISH_SEP_TYPE_BG :
                strncmp(str, "|", len) == 0 ? ISH_SEP_TYPE_PIPE_OUT :
                strncmp(str, ";", len) == 0
                || strncmp(str, "\n", len) == 0 ? ISH_SEP_TYPE_PIPELINE_TERM :
                ISH_SEP_TYPE_NONE;
        break;
        case 2:
            return strncmp(str, ">!", len) == 0 ? ISH_SEP_TYPE_REDIR_OUT_FORCE :
            strncmp(str, ">&", len) == 0 ? ISH_SEP_TYPE_REDIR_OUT_ERR :
            strncmp(str, ">>", len) == 0 ? ISH_SEP_TYPE_APPEND_OUT :
            strncmp(str, "|&", len) == 0 ? ISH_SEP_TYPE_PIPE_OUT_ERR :
            strncmp(str, "&&", len) == 0 ? ISH_SEP_TYPE_LOGICAL_AND :
            strncmp(str, "||", len) == 0 ? ISH_SEP_TYPE_LOGICAL_OR :
            ISH_SEP_TYPE_NONE;
        break;

        case 3:
            return strncmp(str, ">&!", len) == 0 ? ISH_SEP_TYPE_REDIR_OUT_ERR_FORCE :
            strncmp(str, ">>!", len) == 0 ? ISH_SEP_TYPE_APPEND_OUT_FORCE :
            strncmp(str, ">>&", len) == 0 ? ISH_SEP_TYPE_APPEND_OUT_ERR :
            ISH_SEP_TYPE_NONE;
        break;

        case 4:
            return strncmp(str, ">>&!", len) == 0 ? ISH_SEP_TYPE_APPEND_OUT_ERR_FORCE :
            ISH_SEP_TYPE_NONE;
        break;

        default:
        return ISH_SEP_TYPE_NONE;
    }

    return ISH_SEP_TYPE_NONE;
}

void ish_staging_queue_init(ish_staging_queue_t * stagingQueue) {
    if (stagingQueue == NULL)  {
        return;
    }
    stagingQueue->head = 0;
    stagingQueue->tail = 0;
    stagingQueue->size = 0;
    stagingQueue->capacity = ISH_STAGING_QUEUE_DEFAULT_BUF_SIZE;

    stagingQueue->data = (ish_pipeline_t *) calloc(ISH_STAGING_QUEUE_DEFAULT_BUF_SIZE, sizeof(ish_pipeline_t));
}
void ish_staging_queue_enqueue(ish_staging_queue_t * stagingQueue, const ish_pipeline_t pipeline) {
    if (stagingQueue == NULL) {
        return;
    }
    if (stagingQueue->data == NULL) {
        ish_staging_queue_init(stagingQueue);
    }

    if (stagingQueue->size == 0) {
        stagingQueue->head = 0;
        stagingQueue->tail = 0;

        (stagingQueue->data)[0] = pipeline;
        (stagingQueue->size++);
    } else if (stagingQueue->size >= stagingQueue->capacity - 1) {
        (stagingQueue->capacity) *= 2;
        ish_pipeline_t * temp = (ish_pipeline_t * ) calloc(stagingQueue->capacity, sizeof(ish_pipeline_t));
        memcpy(temp, stagingQueue->data, (stagingQueue->capacity / 2) * sizeof(ish_pipeline_t));
        free(stagingQueue->data);
        stagingQueue->data = temp;

        const size_t insertPos = ( (stagingQueue->tail + 1) % stagingQueue->capacity);
        (stagingQueue->data)[insertPos] = pipeline;

        stagingQueue->tail = insertPos;
        (stagingQueue->size)++;
    } else {
        const size_t insertPos = ( (stagingQueue->tail + 1) % stagingQueue->capacity);
        (stagingQueue->data)[insertPos] = pipeline;

        stagingQueue->tail = insertPos;
        (stagingQueue->size)++;
    }
}
ish_pipeline_t * ish_staging_queue_front(ish_staging_queue_t * stagingQueue) {
    if (stagingQueue == NULL || stagingQueue->size == 0) {
        return NULL;
    }

    return &((stagingQueue->data)[stagingQueue->head]);
}
void ish_staging_queue_dequeue(ish_staging_queue_t * stagingQueue) {
    if (stagingQueue == NULL || stagingQueue->size == 0) {
        return;
    }

    (stagingQueue->head) = ((stagingQueue->head) + 1) % (stagingQueue->capacity);
    (stagingQueue->size)--;
}

ish_pipeline_t * ish_staging_queue_fetch_and_dequeue(ish_staging_queue_t * stagingQueue) {
    if (stagingQueue == NULL || stagingQueue->size == 0) {
        return NULL;
    }

    ish_pipeline_t * pipelinePtr = &((stagingQueue->data)[stagingQueue->head]);

    (stagingQueue->head) = ((stagingQueue->head) + 1) % (stagingQueue->capacity);
    (stagingQueue->size)--;

    return pipelinePtr;
}

void ish_staging_queue_free(ish_staging_queue_t * stagingQueue) {
    if (stagingQueue == NULL) {
        return;
    }

    if (stagingQueue->data != NULL) {
        free(stagingQueue->data);
    }
}

ish_pipeline_t ish_pipeline_create_sub_pipeline(ish_pipeline_t * srcPipeline, const int componentStartIdx, const int componentEndIdx) {
    ish_pipeline_t newPipeline;
    if (componentStartIdx <= -1 || componentEndIdx <= -1 || componentEndIdx > componentStartIdx) {
        return newPipeline;
    }

    newPipeline.serializedWordList = srcPipeline->serializedWordList;
    newPipeline.componentCnt = componentEndIdx - componentStartIdx + 1;

    if (newPipeline.componentCnt > 0) {
        newPipeline.capacity = newPipeline.componentCnt;
        newPipeline.components = (ish_pipeline_component_t *) calloc(newPipeline.capacity, sizeof(ish_pipeline_component_t));
    }

    for (int i = componentStartIdx, j = 0; i <= componentEndIdx; i++, j++) {
        newPipeline.components[j] = srcPipeline->components[i];
    }

    return newPipeline;
}