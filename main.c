#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>

// regex для for, while, main, if
// [^A-Za-z0-9_] - класс символов, кроме этих (^ - отрицание)
// ' *' - неопределённое кол-во пробелов
// '|' - или
//[^A-Za-z0-9_](for|while|if|int main) *\(

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

    const char* error = (char*) malloc(sizeof(char) * 500);  // Where to put an error message // TODO needed 2nd error[] for statements?
    int error_offset;    // Offset in pattern where error was found
    char block_pattern[] = "[^A-Za-z0-9_](for|while|if|int main) *\\(";
    char statement_pattern[] = ".+;";

    // regex compilation
    pcre *compiled_block_regex = pcre_compile(block_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE compilation failed");
    }

    pcre *compiled_statement_regex = pcre_compile(statement_pattern, 0, &error, &error_offset, NULL);
    if (compiled_block_regex == NULL) {
        printf("PCRE compilation failed");
    }

    int start_i = 0;
    char** entities = (char**) malloc(sizeof(char*) * 1000);
    for (int i = 0; i < 1000; ++i) {
        entities[i] = (char*) malloc(sizeof(char) * 1000);
    }
//    char entities[1000][1000] = {0};
    int entities_i = 0;
    while (start_i < strlen(input_string)) {
//    for (int input_str_i = 0; input_str_i < strlen(input_string); ++input_str_i) {
        // compare with regex string
        int result_size = 33; // a multiple of 3
        int block_result[result_size];
        int statement_result[result_size];
        int statement_result_code = pcre_exec(compiled_statement_regex, NULL,
                                              input_string, strlen(input_string),
                                              start_i, 0,
                                              statement_result, result_size);

        int block_result_code = pcre_exec(compiled_block_regex, NULL,
                                          input_string, strlen(input_string),
                                          start_i, 0,
                                          block_result, result_size);
        // TODO: need this?
//        if (block_result_code < 0) {
//            printf("No block match");
//        }

        // find end of if
        int block_start_index = block_result[0];
        int opening_brace_counter = 0;
        int block_end_index = -1;
        // if block found, find its end
        if (block_result > 0) {
            for (int i = block_start_index; i < file_size; ++i) {
                // find '}'
                if (input_string[i] == '{') {
                    ++opening_brace_counter;
                } else if (input_string[i] == '}') {
                    --opening_brace_counter;
                    if (opening_brace_counter == 0) {
                        block_end_index = i + 1; // чтоб он указывал на позицию ЗА последним символом блока
                        break;
                    }
                }
            }
        }

        // check if it is statement and block or just for
        // TODO что если блоков вообще нет?
        int statement_start_index = statement_result[0];
        int statement_end_index = statement_result[1];
        // skip spaces at the start of statement
        for (int i = statement_start_index; i < statement_end_index; ++i) {
            if (input_string[i] == ' ') {
                ++statement_start_index;
            } else {
                break; // ++statement_start_index, чтоб уж все убрать
            }
        }
        int statement_size = statement_end_index - statement_start_index;
        // если индекс стейтмента < индекса блока, то это стейтмент, и его можно вырезать (записать стейтмент в массив строк (всех))
        if (statement_start_index < block_start_index || block_result_code < 0) {
            for (int str_i = statement_start_index, buf_i = 0; str_i < statement_end_index && buf_i < statement_size; ++str_i, ++buf_i) {
                entities[entities_i][buf_i] = input_string[str_i];
            }
            entities[entities_i][statement_size] = '\0';
            start_i = statement_end_index;
        } else if (statement_start_index >= block_start_index || statement_result_code < 0) {
            // если индекс блока меньше, то это for - тоже записать в массив строк
            int block_size = block_end_index - block_start_index;
            for (int str_i = block_start_index, buf_i = 0; str_i < block_end_index && buf_i < block_size; ++str_i, ++buf_i) {
                entities[entities_i][buf_i] = input_string[str_i];
            }
            entities[entities_i][block_size] = '\0';
            start_i = block_end_index;
        }
        ++entities_i;
    }

    // print
    for (int entity_number = 0; entity_number < entities_i; ++entity_number) {
        printf("Entity %d:\n%s\n", entity_number + 1, entities[entity_number]);
    }

    // at the end
    pcre_free(compiled_block_regex);












//    for (int i = 0; i < strlen(input_string); ++i) {
//        if (input_string[i] == '\n') {
//            input_string[i] = ' ';
//        }
//    }

//    printf("Read:%s", string);

/*    regex_t regex;
    int regex_not_compiled;
    char msgbuf[100];

// Compile regular expression
//    regex_not_compiled = regcomp(&regex, "[^A-Za-z0-9_](for|while|if|int main) *\(", 0);
    regex_not_compiled = regcomp(&regex, "if", 0);

    if (regex_not_compiled) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }

// Execute regular expression
    int n_matches = 1;              // number of matches to get
    regmatch_t pmatch[n_matches];   // positions
    int result = regexec(&regex, input_string, n_matches, pmatch, 0);
    if (result == 0) {
        // match found
        //  pmatch[i].rm_so
        PAIR position_indices[n_matches];
        for (int i = 0; i < n_matches; ++i) {   // iteration over the matches
            if (pmatch[i].rm_so == -1) {
                break;
            }
            position_indices[i].start = pmatch[i].rm_so;
            position_indices[i].end = pmatch[i].rm_eo;
        }

        // 'if' is found
        int opening_brace_counter = 0;
        int end_if_index = -1;
        for (int i = position_indices[0].start; i < file_size; ++i) {
            // find '{'
            if (input_string[i] == '{') {
                ++opening_brace_counter;
            } else if (input_string[i] == '}') {
                --opening_brace_counter;
                if (opening_brace_counter == 0) {
                    end_if_index = i;
                }
            }
        }

        int start_if_index = position_indices[0].start;
        int if_length = end_if_index - start_if_index + 1;
        char* buffer = (char*) malloc(sizeof(char) * 1000);
        for (int i = start_if_index, buffer_i = 0; i < start_if_index + if_length, buffer_i < 1000; ++i, ++buffer_i) {
            buffer[buffer_i] = input_string[i];
        }
        printf("%s", buffer);
    } else if (result == REG_NOMATCH) {
        // no matches
        printf("no matches\n");
    } else {
        regerror(result, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        exit(1);
    }

// Free memory allocated to the pattern buffer by regcomp()
    regfree(&regex);

    */
    return 0;
}


