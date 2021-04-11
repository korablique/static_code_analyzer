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
    VectorEntity children;
    bool is_loop;
}BLOCK;

#endif //STATIC_CODE_ANALYSER_BLOCK_H

bool IsLoop(BLOCK* block);
bool HasHead(char* block_string);
