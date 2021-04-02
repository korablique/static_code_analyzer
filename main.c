#include <stdio.h>
#include <stdbool.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>

// regex для for, while, main, if
// [^A-Za-z0-9_] - класс символов, кроме этих (^ - отрицание)
// ' *' - неопределённое кол-во пробелов
// '|' - или
//[^A-Za-z0-9_](for|while|if|int main) *\(


/*
 * найти иф с помощью регулярки, найти конец ифа
if (true) {
	while (true) {
		for () {
			int a = 0;
			if (a > b) {
				int c = 0;
			}
		}
	}
}
print(asd);

считать весь файл в одну строчку,
ищем if регуляркой,
идем по символам от начала if'а, найти первую {,
    найти соответствующую ей }
    (нужен счетчик, который увеличивается, когда скобка открылась и уменьшается, когда закрылась)
вывести этот if
(возможно, регулярка не сможет посмотреть за пределы \n => если что, заменить все \n на пробелы)
 */

int main(int argc, char **argv) {
    if (argc < 1) {
        printf("File path should be in command line\n");
        abort();
    }

    // reading file to string
    FILE* input_file = fopen(argv[1], "rb");
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char* input_string = malloc(sizeof(char) * (file_size + 1));
    fread(input_string, sizeof(char), file_size, input_file);
    fclose(input_file);

    input_string[file_size] = 0; // make null terminated C string

    const char* error = (char*) malloc(sizeof(char) * 500);  // Where to put an error message
    int error_offset;    // Offset in pattern where error was found
    char pattern[] = "[^A-Za-z0-9_](for|while|if|int main) *\\(";

    // regex compilation
    pcre* compiled_regex = pcre_compile(pattern, 0, &error, &error_offset, NULL);
    if (compiled_regex == NULL) {
        printf("PCRE compilation failed");
    }

    // compare with regex string
    int result_size = 33; // a multiple of 3
    int result[result_size];
    int start_i = 0;
    int result_code = pcre_exec(compiled_regex, NULL,
                                input_string, strlen(input_string),
                                start_i, 0,
                                result, result_size);
    if (result_code < 0) {
        printf("No match");
    }

    // find end of if
    int start_if_index = result[0];
    int opening_brace_counter = 0;
    int end_if_index = -1;
    for (int i = start_if_index; i < file_size; ++i) {
        // find '}'
        if (input_string[i] == '{') {
            ++opening_brace_counter;
        } else if (input_string[i] == '}') {
            --opening_brace_counter;
            if (opening_brace_counter == 0) {
                end_if_index = i;
                break;
            }
        }
    }

    int if_size = end_if_index - start_if_index + 1;
    char buffer[if_size + 1];
    buffer[if_size] = '\0';
    for (int string_i = start_if_index, buf_i = 0; string_i <= end_if_index, buf_i < if_size; ++string_i, ++buf_i) {
        buffer[buf_i] = input_string[string_i];
    }
    printf("%s", buffer);
    // at the end
    pcre_free(compiled_regex);












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


