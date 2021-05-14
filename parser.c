#include "parser.h"

#define ERROR_MSG_SIZE 200
#define PATTERN_SIZE 400

pcre *compiled_block_regex = NULL;
pcre *compiled_statement_regex = NULL;
pcre *compiled_directive_or_comment_regex = NULL;

bool CompileRegexes() {
    if (compiled_block_regex != NULL) {
        return false;
    }

    // compile regex statements
    const char* error = (char*) malloc(sizeof(char) * ERROR_MSG_SIZE);  // Where to put an error message
    int error_offset;    // Offset in pattern where error was found

    char block_pattern1[] = "[^A-Za-z0-9_]?(for|while|else if|if|int main) *\\(";
    // pattern for "else" and do-while
    char block_pattern2[] = "[^A-Za-z0-9_]?(else|do)[^A-Za-z0-9_]|^(else|do)[^A-Za-z0-9_]|}else";
    char function_pattern[] = "(unsigned |long )?(void|int|float|double|char|bool)\\*? +[A-Za-z0-9_]+ *\\(";
    char block_pattern[PATTERN_SIZE];
    snprintf(block_pattern, sizeof block_pattern, "%s|%s|%s", block_pattern1, block_pattern2, function_pattern);

    char statement_pattern[] = "[^;]+;";

    char directive_or_comment_pattern[] = "^( |\\t)*#[A-Za-z]|^( |\\t)*//";

    compiled_block_regex = pcre_compile(block_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE block compilation failed: %s\n", error);
        abort();
    }

    compiled_statement_regex = pcre_compile(statement_pattern, 0, &error, &error_offset, NULL);
    if (compiled_statement_regex == NULL) {
        printf("PCRE statement compilation failed: %s\n", error);
        abort();
    }

    compiled_directive_or_comment_regex = pcre_compile(directive_or_comment_pattern, 0, &error, &error_offset, NULL);
    if (compiled_directive_or_comment_regex == NULL) {
        printf("PCRE directive compilation failed: %s\n", error);
        abort();
    }

    return true;
}

void FreeRegexes() {
    pcre_free(compiled_block_regex);
    pcre_free(compiled_statement_regex);
    pcre_free(compiled_directive_or_comment_regex);
    compiled_block_regex = NULL;
    compiled_statement_regex = NULL;
    compiled_directive_or_comment_regex = NULL;
}

VectorEntity GetEntities(char* string, int max_entities) {
    bool compiled = CompileRegexes();
    VectorEntity vector_entity = CreateVectorEntity(&EntityComparator);
    int start_i = 0;
    int entity_number = 0;
    while (start_i < strlen(string) && entity_number < max_entities) {
        // compare with regex string
        int result_size = 33; // a multiple of 3
        int block_result[result_size];
        int statement_result[result_size];
        int directive_or_comment_result[result_size];
        int statement_result_code = pcre_exec(compiled_statement_regex, NULL,
                                              string, strlen(string),
                                              start_i, 0,
                                              statement_result, result_size);

        int block_result_code = pcre_exec(compiled_block_regex, NULL,
                                          string, strlen(string),
                                          start_i, 0,
                                          block_result, result_size);

        int directive_or_comment_result_code = pcre_exec(compiled_directive_or_comment_regex, NULL,
                                                         &string[start_i], strlen(&string[start_i]),
                                                         0, 0,
                                                         directive_or_comment_result, result_size);

        int block_start_index;
        if (block_result < 0) {
            block_start_index = INT_MAX;
        } else {
            block_start_index = block_result[0];
            block_start_index = SkipSpaces(string, block_start_index, strlen(string));
        }

        int statement_start_index = statement_result[0];
        int statement_end_index = statement_result[1];
        // skip spaces at the start of statement
        statement_start_index = SkipSpaces(string, statement_start_index, statement_end_index);

        int directive_or_comment_start_index = -1;
        int directive_or_comment_end_index = -1;
        if (directive_or_comment_result_code > 0) {
            directive_or_comment_start_index = directive_or_comment_result[0] + start_i;
            directive_or_comment_start_index = SkipSpaces(string, directive_or_comment_start_index, strlen(string));
            directive_or_comment_end_index = Find('\n', string, directive_or_comment_start_index);
        }

        // check if there are statement and block or just 'for'
        bool is_directive_or_comment = directive_or_comment_result_code > 0 && directive_or_comment_start_index < block_start_index;
        bool is_statement = statement_start_index < block_start_index || block_result_code < 0;
        bool is_block = statement_start_index >= block_start_index || statement_result_code < 0;
        if (is_directive_or_comment) {
            // handle as statement
            char* statement_str = Substring(string, directive_or_comment_start_index, directive_or_comment_end_index);
            STATEMENT* statement = (STATEMENT*) malloc(sizeof(STATEMENT));
            statement->string = statement_str;
            ENTITY* entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->statement = statement;
            entity->block = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
            start_i = directive_or_comment_end_index + 1; // after '\n'
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
                // head will contain only "else" or "do"
                head = GetKeyWord(&string[block_start_index]);
                block_head_end_index = block_start_index + strlen(head);
            }
            block->head = (char *) malloc(sizeof(char) * strlen(head) + 1);
            strcpy(block->head, head);
            block->head[strlen(head)] = '\0';

            // find end of block
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

            // if head == "do", add tail to block
            if (strcmp(head, "do") == 0) {
                // tail is after "do" till ';'
                int tail_start_i = SkipSpaces(string, block_inside_end_i, strlen(string));
                int tail_end_i = Find(';', string, tail_start_i) + 1;
                char* tail = Substring(string, tail_start_i, tail_end_i);
                block->tail = (char*) malloc(sizeof(char) * strlen(tail) + 1);
                strcpy(block->tail, tail);
                block->tail[strlen(tail)] = '\0';

                block_inside_end_i = tail_end_i;
            } else {
                block->tail = NULL;
            }

            char *block_str = Substring(string, block_start_index, block_inside_end_i);
            block->string = (char *) malloc(sizeof(char) * strlen(block_str) + 1);
            strcpy(block->string, block_str);
            block->string[strlen(block_str)] = '\0';

            start_i = block_inside_end_i;

            block->is_loop = IsLoop(block);

            if (next_after_head == '{') {
                --block_inside_end_i; // make index of '}', which will be cut when substring
            }
            char *block_inside_str = Substring(string, block_inside_start_i, block_inside_end_i);

            max_entities = INT_MAX;
            // if there is one line do/while, there is only one statement after "do"
            if (strcmp(head, "do") == 0 && next_after_head != '{') {
                max_entities = 1;
            }
            VectorEntity block_entities = GetEntities(block_inside_str, max_entities);
            block->children = block_entities;
            ENTITY *entity = (ENTITY*) malloc(sizeof(ENTITY));
            entity->block = block;
            entity->statement = NULL;
            PushBackVectorEntity(&vector_entity, *entity);
        }
        ++entity_number;
    }

    if (compiled) {
        FreeRegexes();
    }
    return vector_entity;
}