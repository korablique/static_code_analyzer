#ifndef STATIC_CODE_ANALYSER_PRINT_H
#define STATIC_CODE_ANALYSER_PRINT_H

#include "entity.h"

void Print(ENTITY* node, int depth, FILE* file);
void PrintImpl(ENTITY* node, int depth, FILE* file, bool print_to_log);

#endif //STATIC_CODE_ANALYSER_PRINT_H
