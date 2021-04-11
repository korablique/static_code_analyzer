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

int SkipSpaces(const char* source, int start, int end) {
    if (source[start] == ' ' || source[start] == '\n' || source[start] == '\t') {
        for (int i = start; i < end; ++i) {
            if (source[i] != ' ' && source[i] != '\n' && source[i] != '\t') {
                break;
            }
            ++start;
        }
    }
    return start;
}

int SkipSpacesR(const char* source, int start, int end) {
    --end;
    if (source[end] == ' ' || source[end] == '\n' || source[end] == '\t') {
        for (int i = end; i >= start; --i) {
            if (source[i] != ' ' && source[i] != '\n' && source[i] != '\t') {
                break;
            }
            --end;
        }
    }
    return end;
}

int Find(char symbol, char* source, int start_index) {
    for (int i = start_index; i < strlen(source); ++i) {
        if (source[i] == symbol) {
            return i;
        }
    }
    return -1;
}
