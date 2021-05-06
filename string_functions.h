#ifndef STATIC_CODE_ANALYSER_STRING_FUNCTIONS_H
#define STATIC_CODE_ANALYSER_STRING_FUNCTIONS_H

#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdbool.h>

char* Substring(char* source, int start, int end);
void Replace(char* string, char old_char, char new_char);
void ReplaceExcept(char* source, char old_char, char new_char);
int Skip(char symbol, const char* source, int start, int end);
int SkipSpaces(const char* source, int start, int end);
int SkipSpacesR(const char* source, int start, int end);
int Find(char symbol, char* source, int start_index);
bool IsNumber(char *string);

#endif //STATIC_CODE_ANALYSER_STRING_FUNCTIONS_H
