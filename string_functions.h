#ifndef STATIC_CODE_ANALYSER_STRING_FUNCTIONS_H
#define STATIC_CODE_ANALYSER_STRING_FUNCTIONS_H

char* Substring(char* source, int start, int end);
void Replace(char* string, char old_char, char new_char);
void ReplaceExcept(char* source, char old_char, char new_char);
int SkipSpaces(const char* source, int start, int end);
int SkipSpacesR(const char* source, int start, int end);
int Find(char symbol, char* source, int start_index);

#endif //STATIC_CODE_ANALYSER_STRING_FUNCTIONS_H
