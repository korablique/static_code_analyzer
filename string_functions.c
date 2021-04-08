#include "string_functions.h"
#include <stdlib.h>

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