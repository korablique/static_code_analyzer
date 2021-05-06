#ifndef STATIC_CODE_ANALYSER_ENTITY_H
#define STATIC_CODE_ANALYSER_ENTITY_H

#include <stdbool.h>
#include "cvector.h"

struct Statement;
struct Block;

/**
 * Both statement and block cannot be defined at the same time.
 * Either the first is NULL or the second.
 */
typedef struct Entity {
    struct Statement* statement;
    struct Block* block;
}ENTITY;

int EntityComparator(ENTITY* lhs, ENTITY* rhs);

DECLARE_VECTOR(VectorEntity, ENTITY)

#endif //STATIC_CODE_ANALYSER_ENTITY_H
