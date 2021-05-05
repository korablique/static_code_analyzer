#include "print.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "block.h"
#include "statement.h"

void PrintTabs(int number_of_tabs, FILE* file, bool print_to_log) {
    for (int i = 0; i < number_of_tabs; ++i) {
        fprintf(file, "\t");
        if (print_to_log) printf("\t");
    }
}

void Print(ENTITY* node, int depth, FILE* file) {
    PrintImpl(node, depth, file, true);
}

void PrintImpl(ENTITY* node, int depth, FILE* file, bool print_to_log) {
    if (node->was) {
        return;
    }
    node->was = true;
    if (node->statement != NULL) {
        // print statement
        if (strcmp(node->statement->string, "") != 0) {
            PrintTabs(depth, file, print_to_log);
            fprintf(file, "%s\n", node->statement->string);
            if (print_to_log) printf("%s\n", node->statement->string);
        }
    } else {
        PrintTabs(depth, file, print_to_log);
        if (depth != -1) {
            fprintf(file, "%s\n", node->block->head);
            if (print_to_log) printf("%s\n", node->block->head);
            PrintTabs(depth, file, print_to_log);
            fprintf(file, "{\n");
            if (print_to_log) printf("{\n");
        }
        for (int i = 0; i < node->block->children.size; ++i) {
            ENTITY entity = GetFromVectorEntity(&(node->block->children), i);
            PrintImpl(&entity, depth + 1, file, print_to_log);
        }
        PrintTabs(depth, file, print_to_log);
        if (depth != -1) {
            fprintf(file, "}\n");
            if (print_to_log) printf("}\n");
        }

        if (node->block->tail != NULL) {
            PrintTabs(depth, file, print_to_log);
            fprintf(file, "%s\n", node->block->tail);
            if (print_to_log) printf("%s\n", node->block->tail);
        }
    }
}