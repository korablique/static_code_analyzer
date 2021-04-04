#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <stdbool.h>
#include "entity.h"
#include "block.h"
#include "statement.h"

#define BIG_NUM 400     // TODO: rename

pcre *compiled_block_regex;
pcre *compiled_statement_regex;

void Print(ENTITY* node) {
    if (node->was) {
        return;
    }
    node->was = true;
    if (node->statement != NULL) {
        // print statement
        printf("%s\n", node->statement->string);
    } else {
        printf("%s {\n", node->block->head);
        for (int i = 0; i < node->block->children.size; ++i) {
//            Print(&node->block->children[i]);
            ENTITY entity = GetFromVectorEntity(&(node->block->children), i);
            Print(&entity);
        }
        printf("\n}\n");
    }
}

/**
 * return substring as [)
 */
char* Substring(char* source, int start, int end) {
    if (start >= end) {
        return "";
    }
    char* dest = (char*) malloc(sizeof(char) * (end - start + 1));
    dest[end] = '\0';
    for (int source_i = start, dest_i = 0; source_i < end && dest_i < end - start; ++source_i, ++dest_i) {
        dest[dest_i] = source[source_i];
    }
    return dest;
}

// regex для for, while, main, if
// [^A-Za-z0-9_] - класс символов, кроме этих (^ - отрицание)
// ' *' - неопределённое кол-во пробелов
// '|' - или
//[^A-Za-z0-9_](for|while|if|int main) *\(
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

        // check if there are statement and block or just 'for'
        int statement_start_index = statement_result[0];
        int statement_end_index = statement_result[1];
        // skip spaces at the start of statement
        if (string[statement_start_index] == ' ') {
            for (int i = statement_start_index; i < statement_end_index; ++i) {
                if (string[i] != ' ') {
                    break;
                }
                ++statement_start_index;
            }
        }
        // если индекс стейтмента < индекса блока, то это стейтмент, и его можно вырезать (записать стейтмент в массив строк (всех))
        if (statement_start_index < block_start_index || block_result_code < 0) {
            char* statement_str = Substring(string, statement_start_index, statement_end_index);
            STATEMENT* statement = (STATEMENT*) malloc(sizeof(STATEMENT));
            statement->string = statement_str; // TODO not sure
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

            char* block_inside_str = Substring(block_str, block_inside_start_i, block_inside_end_i);

            VectorEntity block_entities = GetEntities(block_inside_str);
            block->children = block_entities; // TODO: копировать массив?
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
    const char* error = (char*) malloc(sizeof(char) * BIG_NUM);  // Where to put an error message
    int error_offset;    // Offset in pattern where error was found
    char block_pattern1[] = "[^A-Za-z0-9_](for|while|if|int main) *\\(";
    char block_pattern2[] = "[^A-Za-z0-9_]else *{";
    char block_pattern[BIG_NUM];
    snprintf(block_pattern, sizeof block_pattern, "%s|%s", block_pattern1, block_pattern2);

    char statement_pattern[] = ".+;";

    // regex compilation
    compiled_block_regex = pcre_compile(block_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE compilation failed");
    }

    compiled_statement_regex = pcre_compile(statement_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE compilation failed");
    }

    // print
    int entities_array_size = -1;
    VectorEntity entities = GetEntities(input_string);
    BLOCK root = {"", "", entities};
    ENTITY root_entity = {NULL, &root};
    Print(&root_entity);

    // at the end
    pcre_free(compiled_block_regex);
    return 0;
}


