#include "json_test_strings.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(strings, (string), (wstring), (unicode), (wunicode));
JSON_ENABLE(escaped_strings, (quotes), (escaped), (both), (newline));
JSON_ENABLE(chars, (c), (wc));

void TestStringClass() {
    TEST_BODY(strings)
}

void TestEscaping() {
    TEST_BODY(escaped_strings);
}

void TestChars() {
    TEST_BODY(chars)
}

void TestStrings() {
    TestStringClass();
    TestEscaping();
    TestChars();
}

#ifndef JSON_TEST_ALL
int main() {
    TestStrings();
}
#endif
