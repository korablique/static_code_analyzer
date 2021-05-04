#include "string_functions.h"
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

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

/**
 * Replace all newlines except after strings starting with #
 * @param source
 * @param old_char
 * @param new_char
 */
void ReplaceExcept(char* source, char old_char, char new_char) {
    bool need_replace = true;
    for (int i = 0; i < strlen(source); ++i) {
        if (source[i] == '#') {
            need_replace = false;
        }
        if (source[i] == old_char) {
            if (need_replace) {
                source[i] = new_char;
            } else {
                need_replace = true;
            }
        }
    }
}

int Skip(char symbol, const char* source, int start, int end) {
    if (source[start] == symbol) {
        for (int i = start; i < end; ++i) {
            if (source[i] != symbol) {
                break;
            }
            ++start;
        }
    }
    return start;
}

/**
 * @param source
 * @param start
 * @param end points after the requiring char
 * @return first not space char index
 */
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
