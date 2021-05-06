#ifndef STATIC_CODE_ANALYSER_PRINT_H
#define STATIC_CODE_ANALYSER_PRINT_H

#include "entity.h"

void PrintToFile(ENTITY* node, int depth, FILE* file);
void PrintToLog(ENTITY* node, int depth);
void PrintImpl(ENTITY* node, int depth, FILE* file, bool print_to_log, bool print_to_file);

#endif //STATIC_CODE_ANALYSER_PRINT_H
