#include <pcre.h>
#include "block.h"
#include "string_functions.h"

bool IsLoop(BLOCK* block) {
    char* head = block->head;
    char* key_word;
    char loop_pattern[] = "^(for|while)";    // for/while at the start of string
    const char* error = (char*) malloc(sizeof(char) * 200);
    int error_offset = -1;
    pcre* compiled_loop_regex = pcre_compile(loop_pattern, 0, &error, &error_offset, NULL);
    if (compiled_loop_regex == NULL) {
        printf("PCRE compilation failed: %s\n", error);
    }

    int start_i = 0;
    int result_size = 3*3;
    int result[result_size];
    int loop_result_code = pcre_exec(compiled_loop_regex, NULL,
                                          head, strlen(head),
                                          start_i, 0,
                                          result, result_size);
    if (loop_result_code < 0) {
        return false;
    }
    key_word = Substring(head, result[0], result[1]);
    if (strcmp(key_word, "for") == 0 || strcmp(key_word, "while") == 0) {
        return true;
    } else {
        return false;
    }
}
