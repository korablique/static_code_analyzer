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

/**
 * @param node pointer to block to start with
 * @return block vector with endless loops
 */
VectorEntity HasEndlessLoops(ENTITY* node) {
    VectorEntity endless_blocks = CreateVectorEntity(&EntityComparator);
    // DFS
    if (node->statement != NULL) {
        return endless_blocks;
    } else {
        // if is block and loop
        BLOCK* current_block = node->block;
        if (current_block->is_loop) {
            // if head contains "while(const)", where const != 0 and there is no "break" inside => endless loop
            if (strcmp(GetKeyWord(current_block->head), "while") == 0) {
                // get what is in brackets
                int condition_start_i = -1;
                int condition_end_i = -1;
                GetBounds(current_block->head, 0, '(', ')', &condition_start_i, &condition_end_i); // get indices of condition
                --condition_end_i; // to skip ')'
                char* condition = Substring(current_block->head, condition_start_i, condition_end_i);
                if (IsNumber(condition) && fabs(atof(condition)) > 1e-6) {
                    // find break
                    char break_pattern[] = "[^A-Za-z0-9_]break;";
                    const char* error = (char*) malloc(sizeof(char) * 200);
                    int error_offset = -1;
                    pcre* compiled_break_regex = pcre_compile(break_pattern, 0, &error, &error_offset, NULL);
                    if (compiled_break_regex == NULL) {
                        printf("PCRE compilation failed: %s\n", error);
                        abort();
                    }
                    free(error);

                    int start_i = 0;
                    int result_size = 3*3;
                    int result[result_size];
                    int break_result_code = pcre_exec(compiled_break_regex, NULL,
                                                        current_block->string, strlen(current_block->string),
                                                        start_i, 0,
                                                        result, result_size);
                    if (break_result_code < 0) {
                        // there is no break => endless loop
                        PushBackVectorEntity(&endless_blocks, *node);
                    }
                }
            }
        }
        // run for nesting entities
        for (int i = 0; i < node->block->children.size; ++i) {
            ENTITY entity = GetFromVectorEntity(&(node->block->children), i);
            VectorEntity endless_children = HasEndlessLoops(&entity);
            for (int j = 0; j < endless_children.size; ++j) {
                PushBackVectorEntity(&endless_blocks, GetFromVectorEntity(&endless_children, j));
            }
        }
    }

    return endless_blocks;
}
