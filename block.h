#ifndef STATIC_CODE_ANALYSER_BLOCK_H
#define STATIC_CODE_ANALYSER_BLOCK_H

#include "entity.h"

/**
 * string - contains whole block
 * head - contains key word with statement in braces (like "while (true != false)")
 */
typedef struct Block {
    char* string;
    char* head;
    char* tail;
    VectorEntity children;
    bool is_loop;
}BLOCK;

#endif //STATIC_CODE_ANALYSER_BLOCK_H

char* GetKeyWord(char* block_string);
bool IsLoop(BLOCK* block);
bool HasHead(char* block_string);
void GetBounds(const char *source, int source_start, char opening_brace, char closing_brace,
               int *start_index, int *end_index);