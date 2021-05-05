#include <pcre.h>
#include "block.h"
#include "string_functions.h"

#define PATTERN_SIZE 400

char* GetKeyWord(char* block_string) {
    char keyword_pattern[] = "^(for|while|if|else if|else|do)";
    char function_pattern[] = "(unsigned |long )?(void|int|float|double|char) +[A-Za-z0-9_]+";
    char pattern[PATTERN_SIZE];
    snprintf(pattern, sizeof pattern, "%s|%s", keyword_pattern, function_pattern);

    const char* error = (char*) malloc(sizeof(char) * 200);
    int error_offset = -1;
    pcre* compiled_keyword_regex = pcre_compile(pattern, 0, &error, &error_offset, NULL);
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
    char* keyword = Substring(block_string, result[0], result[1]);
    return keyword;
}

bool IsLoop(BLOCK* block) {
    char* head = block->head;
    char* keyword = GetKeyWord(head);
    if (keyword == NULL) {
        return false;
    }

    if (strcmp(keyword, "for") == 0 || strcmp(keyword, "while") == 0 || strcmp(keyword, "do") == 0) {
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

/**
 * Set start and end indices of some code block.
 * @param source source string. If you want to pass a substring (not from first index of source),
 * then pass a pointer like that: &source[some_index]
 * @param source_start where to start find start of block
 * @param opening_brace
 * @param closing_brace
 * @param start_index pointer to integer, where to save result
 * @param end_index pointer to integer, where to save result
 */
void GetBounds(const char *source, int source_start, char opening_brace, char closing_brace,
               int *start_index, int *end_index) {
    int unused; // start_index might be not used if we want to get block head, so it is a stub
    if (start_index == NULL) {
        start_index = &unused;
    }
    int opening_brace_counter = 0;
    bool opening_brace_found = false;
    for (int i = source_start; i < strlen(source); ++i) {
        if (source[i] == opening_brace) {
            if (!opening_brace_found) {
                *start_index = i + 1;
                opening_brace_found = true;
            }
            ++opening_brace_counter;
        } else if (source[i] == closing_brace) {
            --opening_brace_counter;
            if (opening_brace_counter == 0) {
                (*end_index) = i + 1; // index of symbol after closing brace
                break;
            }
        }
    }
    // skip spaces at the start of block
    *start_index = SkipSpaces(source, *start_index, *end_index);
    if ((*source)[start_index] == closing_brace) { // need for 'else'
        ++start_index;
    }
}