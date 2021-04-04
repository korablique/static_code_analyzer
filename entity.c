#include "entity.h"
#include "statement.h"
#include "block.h"

int EntityComparator(ENTITY* lhs, ENTITY* rhs) {
    if (lhs->statement != NULL && rhs->statement != NULL) {
        if (strcmp(lhs->statement->string, rhs->statement->string) == 0) {
            return 1;
        } else {
            return 0;
        }
    } else if (lhs->block != NULL && rhs->block != NULL) {
        if (strcmp(lhs->block->string, rhs->block->string) == 0) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

DEFINE_VECTOR(VectorEntity, ENTITY)