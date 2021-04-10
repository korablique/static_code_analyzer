#include <memory.h>
#include <stdio.h>
#include "test.h"
#include "string_functions.h"

void ReplaceTest() {
    char str[] = "abc\ndef\n";
    char expected[] = "abc def ";
    Replace(str, '\n', ' ');
    if (strcmp(str, expected) != 0) {
        printf("Replace test failed\nExpected: %s\nActual: %s\n", expected, str);
    }
}

void Test() {
    ReplaceTest();
}