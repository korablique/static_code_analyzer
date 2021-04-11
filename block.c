#include <pcre.h>
#include "block.h"
#include "string_functions.h"

char* GetKeyWord(char* block_string) {
    char keyword_pattern[] = "^(for|while|if|else if|else|do|int main)";
    const char* error = (char*) malloc(sizeof(char) * 200);
    int error_offset = -1;
    pcre* compiled_keyword_regex = pcre_compile(keyword_pattern, 0, &error, &error_offset, NULL);
    if (compiled_keyword_regex == NULL) {
        printf("PCRE compilation failed: %s\n", error);
    }

    int start_i = 0;
    int result_size = 3*3;
    int result[result_size];
    int keyword_result_code = pcre_exec(compiled_keyword_regex, NULL,
                                     block_string, strlen(block_string),
                                     start_i, 0,
                                     result, result_size);
    if (keyword_result_code < 0) {
        return NULL;
    }
    char* keyword = Substring(block_string, result[0], result[1]); // TODO надо выделить память?
    return keyword;
}

bool IsLoop(BLOCK* block) {
    char* head = block->head;
    char* keyword = GetKeyWord(head);
    if (keyword == NULL) {
        return false;
    }

    if (strcmp(keyword, "for") == 0 || strcmp(keyword, "while") == 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Check if block has something in round braces
 * @param block_string string starting with start of block (should not contain spaces)
 * @return
 */
bool HasHead(char* block_string) {
    char* keyword = GetKeyWord(block_string);
    if (strcmp(keyword, "else") == 0 || strcmp(keyword, "do") == 0) {
        return false;
    } else {
        return true;
    }
}
