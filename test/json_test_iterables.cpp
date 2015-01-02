#include "json_test_iterables.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(iterables, (vector), (map));
JSON_ENABLE(tuples, (tuple));

void TestIterables() {
    TEST_BODY(iterables);
}

void TestTuples() {
    TEST_BODY(tuples);
}

#ifndef JSON_TEST_ALL
int main() {
    TestIterables();
    TestTuples();
    return 0;
}
#endif
