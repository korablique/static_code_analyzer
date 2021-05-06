#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "test.h"
#include "string_functions.h"
#include "block.h"
#include "print.h"
#include "parser.h"
#include "code_analyzer.h"

void RunStaticAnalyser(char *input, char* file_path) {
    VectorEntity entities = GetEntities(input, INT_MAX);
    BLOCK root = {"", "", NULL, entities};
    ENTITY root_entity = {NULL, &root};
    FILE* file = fopen(file_path, "w");
    PrintImpl(&root_entity, -1, file, false, true);
    fclose(file);
}

char* ReadFile(char* file_path) {
    FILE* file = fopen(file_path, "rb");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* result = malloc(sizeof(char) * (file_size + 1));
    fread(result, sizeof(char), file_size, file);
    fclose(file);
    return result;
}

void ReplaceTest() {
    char str[] = "abc\ndef\n";
    char expected[] = "abc def ";
    Replace(str, '\n', ' ');
    if (strcmp(str, expected) != 0) {
        printf("ReplaceTest failed\nExpected: %s\nActual: %s\n", expected, str);
        abort();
    }
}

void SkipSpacesTest() {
    char str[] = "\t if (a    == b) { \n";
    int first_not_space = SkipSpaces(str, 0, strlen(str));
    int expected_first_not_space = 2;
    if (first_not_space != expected_first_not_space) {
        printf("SkipSpacesTest failed (1)\nExpected: %d\nActual: %d\n",
               expected_first_not_space, first_not_space);
        abort();
    }

    int last_not_space = SkipSpacesR(str, 0, strlen(str));
    int expected_last_not_space = 17;
    if (last_not_space != expected_last_not_space) {
        printf("SkipSpacesTest failed (2)\nExpected: %d\nActual: %d\n",
               expected_last_not_space, last_not_space);
        abort();
    }

    int not_space_in_the_middle = SkipSpaces(str, 7, 17);
    int expected_in_the_middle = 11;
    if (not_space_in_the_middle != expected_in_the_middle) {
        printf("SkipSpacesTest failed (3)\nExpected: %d\nActual: %d\n",
               expected_in_the_middle, not_space_in_the_middle);
        abort();
    }

    int not_space_in_the_middle2 = SkipSpacesR(str, 0, 10);
    int expected_in_the_middle2 = 6;
    if (not_space_in_the_middle2 != expected_in_the_middle2) {
        printf("SkipSpacesTest failed (4)\nExpected: %d\nActual: %d\n",
               expected_in_the_middle2, not_space_in_the_middle2);
        abort();
    }
}

void ReplaceExceptTest() {
    char string[] = "#include <string.h>\nint a = 1;\n#define A 5\nint y = a;\n";
    char expected[] = "#include <string.h>\nint a = 1; #define A 5\nint y = a; ";
    ReplaceExcept(string, '\n', ' ');
    if (strcmp(string, expected) != 0) {
        printf("ReplaceExceptTest failed (4)\nExpected: %s\nActual: %s\n",
               expected, string);
        abort();
    }
}

void GetBoundsTest() {
    char string[] = "if (new_check) {\n\tprint(a);\n} else {\n\tprint(c);\n}";
//    int start_index1 = -1;
//    int end_index1 = -1;
//    int expected_start_index1 = Find('(', string, 0);
//    int expected_end_index1 = Find(')', string, 0);
//    GetBounds(string, 0, '(', ')', &start_index1, &end_index1);
//    if (start_index1 != expected_start_index1 || end_index1 != expected_end_index1) {
//        printf("GetBoundsTest failed (1)\nExpected: start_index = %d, end_index = %d\nActual: start_index = %d, end_index = %d\n",
//               expected_start_index1, expected_end_index1, start_index1, end_index1);
//        abort();
//    }

    int start_index2;
    int end_index2;
    int start_finding = Find('}', string, 0) + 1;
    int expected_start_index2 = Find('{', string, start_finding);
    int expected_end_index2 = Find('}', string, start_finding);
    GetBounds(string, start_finding, '{', '}', &start_index2, &end_index2);
    if (start_index2 != expected_start_index2 || end_index2 != expected_end_index2) {
        printf("GetBoundsTest failed (2)\nExpected: start_index = %d, end_index = %d\nActual: start_index = %d, end_index = %d\n",
               expected_start_index2, expected_end_index2, start_index2, end_index2);
        abort();
    }
}

void ElseTest() {
    char input[] = "if(a == b) {\n\ta = b;\n} else {\n\tj = asd;\n\twhile(a) {\n\t\tc = b;\n\t}\n}";
    RunStaticAnalyser(input, "/tmp/static_code_analyser_test1.c");
    char* result = ReadFile("/tmp/static_code_analyser_test1.c");

    char expected_result[] = "if(a == b)\n{\n\ta = b;\n}\nelse\n{\n\tj = asd;\n\twhile(a)\n\t{\n\t\tc = b;\n\t}\n}\n";
    if (strcmp(expected_result, result) != 0) {
        printf("ElseTest failed:\n%s\n\n----\n\n%s", input, result);
        abort();
    }
}

void OneLineBlocksTest() {
    char input[] = "int main() {\n\tint a = 5;\n\twhile (a > 0) \n\t\t--a;\n\tif (is_true) \n\t\tprint(a);\n\telse if (is_false) print(b);\n\telse f();\n}";
    RunStaticAnalyser(input, "/tmp/static_code_analyser_test2.c");
    char* result = ReadFile("/tmp/static_code_analyser_test2.c");

    char expected_result[] = "int main()\n{\n\tint a = 5;\n\twhile (a > 0)\n\t{\n\t\t--a;\n\t}\n\tif (is_true)\n\t{\n\t\tprint(a);\n\t}\n\telse if (is_false)\n\t{\n\t\tprint(b);\n\t}\n\telse\n\t{\n\t\tf();\n\t}\n}\n";
    if (strcmp(expected_result, result) != 0) {
        printf("OneLineBlocksTest failed:\n%s\n\n----\n\n%s", input, result);
        abort();
    }
}

void DoWhileOneLineTest() {
    char input[] = "int main() {\n\tdo a = b;\n\twhile (a < b);\n}\n";
    RunStaticAnalyser(input, "/tmp/static_code_analyser_test3.c");
    char* result = ReadFile("/tmp/static_code_analyser_test3.c");

    char expected_result[] = "int main()\n{\n\tdo\n\t{\n\t\ta = b;\n\t}\n\twhile (a < b);\n}\n";
    if (strcmp(expected_result, result) != 0) {
        printf("DoWhileOneLineTest failed:\n%s\n\n----\n\n%s", input, result);
        abort();
    }
}

void HasEndlessLoopsTest() {
    char input[] = "int main() {\n\twhile(10) {\n\t\tif (condition) {\n\t\t\tbreak;\n\t\t}\n\t}\n\n\twhile(0) {\n\t\twhile(1) {\n\t\t\tprintf(\"1\");\n\t\t}\n\t}\n\treturn 0;\n}";
    VectorEntity entities = GetEntities(input, INT_MAX);
    BLOCK root = {"", "", NULL, entities};
    ENTITY root_entity = {NULL, &root};
    VectorEntity endless_blocks = HasEndlessLoops(&root_entity);
    int expected = 1;
    if (endless_blocks.size != 1) {
        printf("HasEndlessLoopsTest failed: expected %d endless loops, found %d\n", expected, endless_blocks.size);
        abort();
    }
}

void Test() {
    printf("Run tests\n");
    ReplaceTest();
    SkipSpacesTest();
    ReplaceExceptTest();
//    GetBoundsTest();
    ElseTest();
    OneLineBlocksTest();
    DoWhileOneLineTest();
    HasEndlessLoopsTest();
    printf("Tests passed\n");
}