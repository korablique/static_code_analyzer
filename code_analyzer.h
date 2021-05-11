#ifndef STATIC_CODE_ANALYSER_CODE_ANALYZER_H
#define STATIC_CODE_ANALYSER_CODE_ANALYZER_H

#include "entity.h"
#include "block.h"
#include "string_functions.h"
#include <pcre.h>
#include <math.h>

int GetMaxNestingOfLoops(ENTITY* node);
VectorEntity HasEndlessLoops(ENTITY* root_entity);

#endif //STATIC_CODE_ANALYSER_CODE_ANALYZER_H
