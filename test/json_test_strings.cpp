#include "json_test_strings.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(strings, (string), (wstring));
JSON_ENABLE(chars, (c), (wc));

void TestStringClass() {
    TEST_BODY(strings)
}

void TestChars() {
    TEST_BODY(chars)
}

void TestStrings() {
    TestStringClass() ;
    TestChars();
}

#ifndef JSON_TEST_ALL
int main() {
    TestStrings();
}
#endif
