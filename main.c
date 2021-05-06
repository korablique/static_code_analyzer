#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "entity.h"
#include "block.h"
#include "string_functions.h"
#include "parser.h"
#include "print.h"
#include "test.h"
#include "code_analyzer.h"

int GetMaxNestingOfLoops(ENTITY* node) {
    if (node->statement != NULL) {
        return 0;
    }
    int answer = 0;
    for (int i = 0; i < node->block->children.size; ++i) {
        answer = fmax(answer, IsLoop(node->block) + GetMaxNestingOfLoops(&node->block->children.array[i]));
    }
    return answer;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("File path should be in command line\n");
        abort();
    }

    if (argc > 3 && strcmp(argv[3], "test") == 0) {
        Test();
        return 0;
    }

    // read file to string
    FILE* input_file = fopen(argv[1], "rb");
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char* input_string = malloc(sizeof(char) * (file_size + 1));
    fread(input_string, sizeof(char), file_size, input_file);
    fclose(input_file);

    input_string[file_size] = 0; // make null terminated C string
    ReplaceExcept(input_string, '\n', ' ');

    // output
    VectorEntity entities = GetEntities(input_string, INT_MAX);
    BLOCK root = {"", "", NULL, entities};
    ENTITY root_entity = {NULL, &root};

    VectorEntity blocks_with_endless_loop = HasEndlessLoops(&root_entity);
    int endless_blocks_number = blocks_with_endless_loop.size;
    if (endless_blocks_number > 0) {
        printf("Probably there are %d endless loops:\n", endless_blocks_number);
        for (int i = 0; i < endless_blocks_number; ++i) {
            ENTITY entity = GetFromVectorEntity(&blocks_with_endless_loop, i);
            PrintToLog(&entity, 0);
        }
    }

    FILE* output_file = fopen(argv[2], "w");
    PrintToFile(&root_entity, -1, output_file);
    printf("Max loops nesting: %d", GetMaxNestingOfLoops(&root_entity));
    fclose(output_file);
    return 0;
}
