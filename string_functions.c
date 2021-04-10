#include "string_functions.h"
#include <stdlib.h>
#include <memory.h>

/**
 * return substring as [)
 */
char* Substring(char* source, int start, int end) {
    if (start >= end) {
        return "";
    }
    int distance = end - start;
    char* dest = (char*) malloc(sizeof(char) * (distance + 1));
    for (int i = 0; i < distance; ++i) {
        dest[i] = source[i + start];
    }
    dest[distance] = '\0';
    return dest;
}

void Replace(char* string, char old_char, char new_char) {
    for (int i = 0; i < strlen(string); ++i) {
        if (string[i] == old_char) {
            string[i] = new_char;
        }
    }
}