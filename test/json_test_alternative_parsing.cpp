#include "json_test_alternative_parsing.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(custom_key, (i, "custom_key"));

void TestCustomKey() {
    TEST_BODY(custom_key);
}

#ifndef JSON_TEST_ALL
int main() {
    TestCustomKey();
}
#endif
