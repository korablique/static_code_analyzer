#ifndef STATIC_CODE_ANALYSER_PARSER_H
#define STATIC_CODE_ANALYSER_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <stdbool.h>
#include "entity.h"
#include "block.h"
#include "statement.h"
#include "string_functions.h"

VectorEntity GetEntities(char* string, int max_entities);

#endif //STATIC_CODE_ANALYSER_PARSER_H
