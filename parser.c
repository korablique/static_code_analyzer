#include "parser.h"

#define BIG_NUM 400     // TODO: rename

pcre *compiled_block_regex = NULL;
pcre *compiled_statement_regex = NULL;
pcre *compiled_directive_regex = NULL;

bool CompileRegexes() {
    if (compiled_block_regex != NULL) {
        return false;
    }

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
    char directive_pattern[] = "^( |\\t)*#[A-Za-z]";

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

    return true;
}

void FreeRegexes() {
    pcre_free(compiled_block_regex);
    pcre_free(compiled_statement_regex);
    pcre_free(compiled_directive_regex);
    compiled_block_regex = NULL;
    compiled_statement_regex = NULL;
    compiled_directive_regex = NULL;
}

VectorEntity GetEntities(char* string) {
    bool compiled = CompileRegexes();
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
        statement_start_index = Skip('}', string, statement_start_index, statement_end_index); // TODO костыль невероятный
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
            char *head;
            if (HasHead(&string[block_start_index])) {
                GetBounds(string, block_start_index, '(', ')', NULL, &block_head_end_index);
                block_head_end_index = SkipSpacesR(string, block_start_index, block_head_end_index);
                ++block_head_end_index; // index of symbol after head
                head = Substring(string, block_start_index, block_head_end_index);
            } else {
                // head will contain only "else"
                head = GetKeyWord(&string[block_start_index]);
                block_head_end_index = block_start_index + strlen(head);
            }
            block->head = (char *) malloc(sizeof(char) * strlen(head));
            strcpy(block->head, head);

            // find end of block
            // смотрим, если все пробелы игнорировать, что первое - открывающаяся фигурная скобка или другой символ
            // если открывающаяся фигурная скобка, то конец блока - это парная закрывающая фигурная скобка
            // если другой символ, то тело блока - это всё до первой точки с запятой
            int after_head_index = SkipSpaces(string, block_head_end_index, strlen(string));
            char next_after_head = string[after_head_index];
            // find inside of block
            int block_inside_start_i = -1;
            int block_inside_end_i = -1;
            if (next_after_head == '{') {                       // block_inside_end_i - index after brace
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

            if (next_after_head == '{') {
                --block_inside_end_i; // make index of '}', which will be cut when substring
            }
            char *block_inside_str = Substring(string, block_inside_start_i, block_inside_end_i);

            VectorEntity block_entities = GetEntities(block_inside_str);
            block->children = block_entities;
            ENTITY *entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->block = block;
            entity->statement = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
        }
    }

    if (compiled) {
        FreeRegexes();
    }
    return vector_entity;
}