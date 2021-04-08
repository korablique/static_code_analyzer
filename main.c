#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <stdbool.h>
#include <math.h>
#include "entity.h"
#include "block.h"
#include "statement.h"
#include "string_functions.h"

#define BIG_NUM 400     // TODO: rename

pcre *compiled_block_regex;
pcre *compiled_statement_regex;

int GetMaxNestingOfLoops(ENTITY* node) {
    if (node->statement != NULL) {
        return 0;
    }
    int answer = 0;
    for (int i = 0; i < node->block->children.size; ++i) {
        answer = fmax(answer, IsLoop(node->block) + GetMaxNestingOfLoops(&node->block->children.array[i]));
    }
    return answer;
}

void PrintTabs(int number_of_tabs, FILE* file) {
    for (int i = 0; i < number_of_tabs; ++i) {
        fprintf(file, "\t");
    }
}

void Print(ENTITY* node, int depth, FILE* file) {
    if (node->was) {
        return;
    }
    node->was = true;
    if (node->statement != NULL) {
        // print statement
        if (strcmp(node->statement->string, "") != 0) {
            PrintTabs(depth, file);
            fprintf(file, "%s\n", node->statement->string);
        }
    } else {
        PrintTabs(depth, file);
        if (depth != -1) {
            fprintf(file, "%s\n", node->block->head);
            PrintTabs(depth, file);
            fprintf(file, "{\n");
        }
        for (int i = 0; i < node->block->children.size; ++i) {
            ENTITY entity = GetFromVectorEntity(&(node->block->children), i);
            Print(&entity, depth + 1, file);
        }
        PrintTabs(depth, file);
        if (depth != -1) {
            fprintf(file, "}\n");
        }
    }
}

int GetNotSpaceCharIndex(const char* source, int start, int end) {
    if (source[start] == ' ' || source[start] == '\n' || source[start] == '\t') {
        for (int i = start; i < end; ++i) {
            if (source[i] != ' ' && source[i] != '\n' && source[i] != '\t') {
                break;
            }
            ++start;
        }
    }
    return start;
}

VectorEntity GetEntities(char* string) {
    VectorEntity vector_entity = CreateVectorEntity(&EntityComparator);
    int start_i = 0;
    while (start_i < strlen(string)) {
        // compare with regex string
        int result_size = 33; // a multiple of 3
        int block_result[result_size];
        int statement_result[result_size];
        int statement_result_code = pcre_exec(compiled_statement_regex, NULL,
                                              string, strlen(string),
                                              start_i, 0,
                                              statement_result, result_size);

        int block_result_code = pcre_exec(compiled_block_regex, NULL,
                                          string, strlen(string),
                                          start_i, 0,
                                          block_result, result_size);

        // find end of block
        int block_start_index = block_result[0];
        int opening_brace_counter = 0;
        int block_end_index = -1;
        // if block found, find its end
        if (block_result > 0) {
            for (int i = block_start_index; i < strlen(string); ++i) {
                // find '}'
                if (string[i] == '{') {
                    ++opening_brace_counter;
                } else if (string[i] == '}') {
                    --opening_brace_counter;
                    if (opening_brace_counter == 0) {
                        block_end_index = i + 1; // чтоб он указывал на позицию ЗА последним символом блока
                        break;
                    }
                }
            }
        }

        int statement_start_index = statement_result[0];
        int statement_end_index = statement_result[1];
        // skip spaces at the start of statement
        statement_start_index = GetNotSpaceCharIndex(string, statement_start_index, statement_end_index);
        // skip spaces at the start of block
        block_start_index = GetNotSpaceCharIndex(string, block_start_index, block_end_index);
        if (string[block_start_index] == '}') {
            ++block_start_index;
        }

        // check if there are statement and block or just 'for'
        // если индекс стейтмента < индекса блока, то это стейтмент, и его можно положить в вектор
        if (statement_start_index < block_start_index || block_result_code < 0) {
            char* statement_str = Substring(string, statement_start_index, statement_end_index);
            STATEMENT* statement = (STATEMENT*) malloc(sizeof(STATEMENT));
            statement->string = statement_str;
            ENTITY* entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->statement = statement;
            entity->block = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
            start_i = statement_end_index;
        } else if (statement_start_index >= block_start_index || statement_result_code < 0) {
            // если индекс блока меньше, то это for - тоже записать в массив строк
            char* block_str = Substring(string, block_start_index, block_end_index);

            BLOCK* block = (BLOCK*) malloc(sizeof(BLOCK));
            block->string = (char*) malloc(sizeof(char) * strlen(block_str));
            strcpy(block->string, block_str);
            start_i = block_end_index;

            // find inside of block
            int block_inside_start_i = -1;
            bool found_block_inside_start = false;
            int block_inside_end_i = -1;
            for (int block_i = 0; block_i < strlen(block_str); ++block_i) {
                if (block_str[block_i] == '{') {
                    if (!found_block_inside_start) {
                        block_inside_start_i = block_i + 1;
                        found_block_inside_start = true;
                    }
                } else if (block_str[block_i] == '}') {
                    block_inside_end_i = block_i;
                }
            }

            char* head = Substring(block_str, 0, block_inside_start_i - 1);
            block->head = (char*) malloc(sizeof(char) * strlen(head));
            strcpy(block->head, head);

            // check block type to set is_loop field
            bool is_loop = IsLoop(block);
            block->is_loop = is_loop;

            char* block_inside_str = Substring(block_str, block_inside_start_i, block_inside_end_i);

            VectorEntity block_entities = GetEntities(block_inside_str);
            block->children = block_entities;
            ENTITY* entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->block = block;
            entity->statement = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
        }
    }
    return vector_entity;
}

int main(int argc, char **argv) {
    if (argc < 1) {
        printf("File path should be in command line\n");
        abort();
    }

    // read file to string
    FILE* input_file = fopen(argv[1], "rb");
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char* input_string = malloc(sizeof(char) * (file_size + 1));
    fread(input_string, sizeof(char), file_size, input_file);
    fclose(input_file);

    input_string[file_size] = 0; // make null terminated C string

    // compile regex statements
    // [^A-Za-z0-9_] - class of symbols except these ('^' - not)
    // ' *' - zero or more occurrences of spaces
    // '|' - or
    const char* error = (char*) malloc(sizeof(char) * BIG_NUM);  // Where to put an error message
    int error_offset;    // Offset in pattern where error was found
    char block_pattern1[] = "[^A-Za-z0-9_]?(for|while|else if|if|int main) *\\(";
    char block_pattern2[] = "[^A-Za-z0-9_]else *{";
    char block_pattern[BIG_NUM];
    snprintf(block_pattern, sizeof block_pattern, "%s|%s", block_pattern1, block_pattern2);

    char statement_pattern[] = ".+;";

    // regex compilation
    compiled_block_regex = pcre_compile(block_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE compilation failed: %s\n", error);
    }

    compiled_statement_regex = pcre_compile(statement_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE compilation failed: %s\n", error);
    }

    // print
    VectorEntity entities = GetEntities(input_string);
    BLOCK root = {"", "", entities};
    ENTITY root_entity = {NULL, &root};

    FILE* output_file = fopen(argv[1], "w");
    Print(&root_entity, -1, output_file);
    printf("max nesting: %d", GetMaxNestingOfLoops(&root_entity));
    fclose(output_file);
    
    // at the end
    pcre_free(compiled_block_regex);
    return 0;
}
