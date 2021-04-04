#ifndef STATIC_CODE_ANALYSER_CVECTOR_H
#define STATIC_CODE_ANALYSER_CVECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define DECLARE_VECTOR(VECTOR_TYPE_NAME, ITEM_TYPE)              \
                                                                 \
typedef struct CVector##ITEM_TYPE {                              \
    ITEM_TYPE* array;                                            \
    int size;                                                    \
    int capacity;                                                \
    int (*comparator)(ITEM_TYPE*, ITEM_TYPE*);                   \
}VECTOR_TYPE_NAME;                                               \
                                                                 \
VECTOR_TYPE_NAME Create##VECTOR_TYPE_NAME(                       \
        int (*comparator)(ITEM_TYPE*, ITEM_TYPE*));              \
void PushBack##VECTOR_TYPE_NAME(                                 \
        VECTOR_TYPE_NAME* cv, ITEM_TYPE element);                \
int Find##VECTOR_TYPE_NAME(                                      \
        VECTOR_TYPE_NAME* cv, ITEM_TYPE element);                \
int EraseFirst##VECTOR_TYPE_NAME(                                \
        VECTOR_TYPE_NAME* cv, ITEM_TYPE element);                \
ITEM_TYPE GetFrom##VECTOR_TYPE_NAME(                             \
        VECTOR_TYPE_NAME* cv, int index);                        \

#define DEFINE_VECTOR(VECTOR_TYPE_NAME, ITEM_TYPE)               \
VECTOR_TYPE_NAME Create##VECTOR_TYPE_NAME(                       \
        int (*comparator)(ITEM_TYPE*, ITEM_TYPE*)) {             \
    VECTOR_TYPE_NAME cv = {NULL, 0, 0, comparator};              \
    return cv;                                                   \
}                                                                \
                                                                 \
void Reallocate##VECTOR_TYPE_NAME(VECTOR_TYPE_NAME* cv) {        \
    int new_capacity = cv->capacity * 2 + 10;                    \
    ITEM_TYPE* new_array = (ITEM_TYPE*) malloc(                  \
        sizeof (ITEM_TYPE) * new_capacity);                      \
                                                                 \
    /* move data to new array */                                 \
    memcpy(new_array, cv->array, sizeof (ITEM_TYPE) * cv->size); \
                                                                 \
    free(cv->array);                                             \
    cv->array = new_array;                                       \
    cv->capacity = new_capacity;                                 \
}                                                                \
                                                                 \
void PushBack##VECTOR_TYPE_NAME(                                 \
        VECTOR_TYPE_NAME* cv, ITEM_TYPE element) {               \
    if (cv->size == cv->capacity) {                              \
        Reallocate##VECTOR_TYPE_NAME(cv);                        \
    }                                                            \
    cv->array[cv->size] = element;                               \
    cv->size += 1;                                               \
}                                                                \
                                                                 \
int Find##VECTOR_TYPE_NAME(                                      \
        VECTOR_TYPE_NAME* cv, ITEM_TYPE element) {               \
    int index = -1;                                              \
    for (int i = 0; i < cv->size; ++i) {                         \
        if (cv->comparator(&(cv->array[i]), &element)) {         \
            index = i;                                           \
            break;                                               \
        }                                                        \
    }                                                            \
    return index;                                                \
}                                                                \
                                                                 \
int EraseFirst##VECTOR_TYPE_NAME(                                \
        VECTOR_TYPE_NAME* cv, ITEM_TYPE element) {               \
    int index = Find##VECTOR_TYPE_NAME(cv, element);             \
    if (index == -1) {                                           \
        return 0;                                                \
    }                                                            \
                                                                 \
    for (int i = index + 1; i < cv->size; ++i) {                 \
        cv->array[i-1] = cv->array[i];                           \
    }                                                            \
    cv->size--;                                                  \
    return 1;                                                    \
}                                                                \
                                                                 \
ITEM_TYPE GetFrom##VECTOR_TYPE_NAME(                             \
        VECTOR_TYPE_NAME* cv, int index) {                       \
    return cv->array[index];                                     \
}                                                                \

#endif //STATIC_CODE_ANALYSER_CVECTOR_H
