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
#include "test.h"

#define BIG_NUM 400     // TODO: rename

pcre *compiled_block_regex;
pcre *compiled_statement_regex;
pcre *compiled_directive_regex;

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
        printf("\t");

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
            printf("%s\n", node->statement->string);
        }
    } else {
        PrintTabs(depth, file);
        if (depth != -1) {
            fprintf(file, "%s\n", node->block->head);
            printf("%s\n", node->block->head);
            PrintTabs(depth, file);
            fprintf(file, "{\n");
            printf("{\n");
        }
        for (int i = 0; i < node->block->children.size; ++i) {
            ENTITY entity = GetFromVectorEntity(&(node->block->children), i);
            Print(&entity, depth + 1, file);
        }
        PrintTabs(depth, file);
        if (depth != -1) {
            fprintf(file, "}\n");
            printf("}\n");
        }
    }
}

/**
 * Set start and end indices of some code block.
 * @param source source source string. If you want to pass a substring (not from first index of source,
 * then pass a pointer like that: &source[some_index]
 * @param source_start where to start find start of block
 * @param opening_brace
 * @param closing_brace
 * @param start_index pointer to integer, where to save result
 * @param end_index
 */
void GetBounds(const char *source, int source_start, char opening_brace, char closing_brace,
               int *start_index, int *end_index) {
    int unused; // start_index might be not used if we want to get block head, so it is a stub
    if (start_index == NULL) {
        start_index = &unused;
    }
    int opening_brace_counter = 0;
    bool opening_brace_found = false;
    for (int i = source_start; i < strlen(source); ++i) {
        if (source[i] == opening_brace) {
            if (!opening_brace_found) {
                *start_index = i + 1;
                opening_brace_found = true;
            }
            ++opening_brace_counter;
        } else if (source[i] == closing_brace) {
            --opening_brace_counter;
            if (opening_brace_counter == 0) {
                (*end_index) = i + 1; // чтоб он указывал на позицию ЗА последним символом блока // TODO он и так указывает на скобку
                break;
            }
        }
    }
    // skip spaces at the start of block
    *start_index = SkipSpaces(source, *start_index, *end_index);
    if ((*source)[start_index] == closing_brace) { // need for 'else'
        ++start_index;
    }
}

VectorEntity GetEntities(char* string) {
    VectorEntity vector_entity = CreateVectorEntity(&EntityComparator);
    int start_i = 0;
    while (start_i < strlen(string)) {
        // compare with regex string
        int result_size = 33; // a multiple of 3
        int block_result[result_size];
        int statement_result[result_size];
        int directive_result[result_size];
        int statement_result_code = pcre_exec(compiled_statement_regex, NULL,
                                              string, strlen(string),
                                              start_i, 0,
                                              statement_result, result_size);

        int block_result_code = pcre_exec(compiled_block_regex, NULL,
                                          string, strlen(string),
                                          start_i, 0,
                                          block_result, result_size);

        int directive_result_code = pcre_exec(compiled_directive_regex, NULL,
                                          &string[start_i], strlen(&string[start_i]),
                                          0, 0,
                                          directive_result, result_size);

        int block_start_index = block_result[0]; // TODO если блок не найден, можно задать бесконечность
        block_start_index = SkipSpaces(string, block_start_index, strlen(string));

        int statement_start_index = statement_result[0];
        int statement_end_index = statement_result[1];
        // skip spaces at the start of statement
        statement_start_index = SkipSpaces(string, statement_start_index, statement_end_index);

        int directive_start_index = -1;
        int directive_end_index = -1;
        if (directive_result_code > 0) {
            directive_start_index = directive_result[0] + start_i;
            directive_start_index = SkipSpaces(string, directive_start_index, strlen(string));
            directive_end_index = Find('\n', string, directive_start_index);
        }

        // check if there are statement and block or just 'for'
        bool is_directive = directive_result_code > 0 && directive_start_index < block_start_index;
        bool is_statement = statement_start_index < block_start_index || block_result_code < 0;
        bool is_block = statement_start_index >= block_start_index || statement_result_code < 0;
        if (is_directive) {
            // обрабатывается как statement TODO убрать дублирование
            char* directive_str = Substring(string, directive_start_index, directive_end_index);
            STATEMENT* directive = (STATEMENT*) malloc(sizeof(STATEMENT));
            directive->string = directive_str;
            ENTITY* entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->statement = directive;
            entity->block = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
            start_i = directive_end_index + 1; // after '\n'
        } else if (is_statement) {
            char* statement_str = Substring(string, statement_start_index, statement_end_index);
            STATEMENT* statement = (STATEMENT*) malloc(sizeof(STATEMENT));
            statement->string = statement_str;
            ENTITY* entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->statement = statement;
            entity->block = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
            start_i = statement_end_index;
        } else if (is_block && block_result > 0) {
            BLOCK *block = (BLOCK *) malloc(sizeof(BLOCK));

            int block_head_end_index = -1;
            // get block head
            if (HasHead(&string[block_start_index])) {
                GetBounds(string, block_start_index, '(', ')', NULL, &block_head_end_index);
                block_head_end_index = SkipSpacesR(string, block_start_index, block_head_end_index);
                ++block_head_end_index;
            } else {
                // head will contain only "else"
                block_head_end_index = SkipSpacesR(string, block_start_index, block_result[1]);
            }
            char *head = Substring(string, block_start_index, block_head_end_index);
            block->head = (char *) malloc(sizeof(char) * strlen(head));
            strcpy(block->head, head);

            // find end of block
            // смотрим, если все пробелы игнорировать, что первое - открывающаяся фигурная скобка или другой символ.
            // если открывающаяся фигурная скобка, то конец блока - это парная закрывающая фигурная скобка
            // если другой символ, то тело блока - это всё до первой точки с запятой
            int after_head_index = SkipSpaces(string, block_head_end_index, strlen(string));
            char next_after_head = string[after_head_index];
            // find inside of block
            int block_inside_start_i = -1;
            int block_inside_end_i = -1;
            if (next_after_head == '{') {
                GetBounds(string, block_start_index, '{', '}', &block_inside_start_i, &block_inside_end_i);
            } else {
                // there is a one-line block inside
                block_inside_start_i = after_head_index;
                block_inside_end_i = Find(';', string, block_inside_start_i) + 1;
            }

            char *block_str = Substring(string, block_start_index, block_inside_end_i); // todo: probably don't need

            block->string = (char *) malloc(sizeof(char) * strlen(block_str));
            strcpy(block->string, block_str);
            start_i = block_inside_end_i;

            block->is_loop = IsLoop(block);

            char *block_inside_str = Substring(string, block_inside_start_i, block_inside_end_i);

            VectorEntity block_entities = GetEntities(block_inside_str);
            block->children = block_entities;
            ENTITY *entity = (ENTITY*) malloc(sizeof(ENTITY));
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

    if (argc > 2 && strcmp(argv[2], "test") == 0) {
        printf("Run tests\n");
        Test();
        return 0;
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
    ReplaceExcept(input_string, '\n', ' ');

    // compile regex statements
    // [^A-Za-z0-9_] - class of symbols except these ('^' - not)
    // ' *' - zero or more occurrences of spaces
    // '|' - or
    const char* error = (char*) malloc(sizeof(char) * BIG_NUM);  // Where to put an error message
    int error_offset;    // Offset in pattern where error was found
    char block_pattern1[] = "[^A-Za-z0-9_]?(for|while|else if|if|int main) *\\(";
    char block_pattern2[] = "[^A-Za-z0-9_]else *{?";
    char block_pattern[BIG_NUM];
    snprintf(block_pattern, sizeof block_pattern, "%s|%s", block_pattern1, block_pattern2);

    char statement_pattern[] = "[^;]+;";
//    char directive_pattern[] = "^#[A-Za-z]|[^\"]#[A-Za-z]"; // # в начале строки либо не в кавычках
    char directive_pattern[] = "^( |\\t)*#[A-Za-z]";

    // regex compilation
    compiled_block_regex = pcre_compile(block_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE block compilation failed: %s\n", error);
    }

    compiled_statement_regex = pcre_compile(statement_pattern, 0, &error, &error_offset, NULL);
    if (compiled_statement_regex == NULL) {
        printf("PCRE statement compilation failed: %s\n", error);
    }

    compiled_directive_regex = pcre_compile(directive_pattern, 0, &error, &error_offset, NULL);
    if (compiled_directive_regex == NULL) {
        printf("PCRE directive compilation failed: %s\n", error);
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
    pcre_free(compiled_statement_regex);
    pcre_free(compiled_directive_regex);
    return 0;
}
