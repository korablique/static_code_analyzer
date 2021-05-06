#include "print.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "block.h"
#include "statement.h"

void PrintTabs(int number_of_tabs, FILE* file, bool print_to_log, bool print_to_file) {
    for (int i = 0; i < number_of_tabs; ++i) {
        if (print_to_file) fprintf(file, "\t");
        if (print_to_log) printf("\t");
    }
}

void PrintToFile(ENTITY* node, int depth, FILE* file) {
    PrintImpl(node, depth, file, false, true);
}

void PrintToLog(ENTITY* node, int depth) {
    PrintImpl(node, depth, NULL, true, false);
}

void PrintImpl(ENTITY* node, int depth, FILE* file, bool print_to_log, bool print_to_file) {
    if (node->statement != NULL) {
        // print statement
        if (strcmp(node->statement->string, "") != 0) {
            PrintTabs(depth, file, print_to_log, print_to_file);
            if (print_to_file) fprintf(file, "%s\n", node->statement->string);
            if (print_to_log) printf("%s\n", node->statement->string);
        }
    } else {
        PrintTabs(depth, file, print_to_log, print_to_file);
        if (depth != -1) {
            if (print_to_file) fprintf(file, "%s\n", node->block->head);
            if (print_to_log) printf("%s\n", node->block->head);
            PrintTabs(depth, file, print_to_log, print_to_file);
            if (print_to_file) fprintf(file, "{\n");
            if (print_to_log) printf("{\n");
        }
        for (int i = 0; i < node->block->children.size; ++i) {
            ENTITY entity = GetFromVectorEntity(&(node->block->children), i);
            PrintImpl(&entity, depth + 1, file, print_to_log, print_to_file);
        }
        PrintTabs(depth, file, print_to_log, print_to_file);
        if (depth != -1) {
            if (print_to_file) fprintf(file, "}\n");
            if (print_to_log) printf("}\n");
        }

        if (node->block->tail != NULL) {
            PrintTabs(depth, file, print_to_log, print_to_file);
            if (print_to_file) fprintf(file, "%s\n", node->block->tail);
            if (print_to_log) printf("%s\n", node->block->tail);
        }
    }
}