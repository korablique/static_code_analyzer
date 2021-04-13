#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "string_functions.h"

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

void Test() {
    ReplaceTest();
    SkipSpacesTest();
    ReplaceExceptTest();
}