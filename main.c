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
    if (argc < 1) {
        printf("File path should be in command line\n");
        abort();
    }

    if (argc > 2 && strcmp(argv[2], "test") == 0) {
        printf("Run tests\n");
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

    // print
    VectorEntity entities = GetEntities(input_string, INT_MAX);
    BLOCK root = {"", "", NULL, entities};
    ENTITY root_entity = {NULL, &root};

    FILE* output_file = fopen(argv[2], "w");
    Print(&root_entity, -1, output_file);
    printf("max nesting: %d", GetMaxNestingOfLoops(&root_entity));
    fclose(output_file);
    return 0;
}
