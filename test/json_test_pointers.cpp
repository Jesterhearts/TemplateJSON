#include "json_test_pointers.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(bare_pointers, (pointer), (null));

JSON_ENABLE(smart_pointers, (unique), (shared));

void TestBarePointers() {
    TEST_BODY(bare_pointers);
}

void TestSmartPointers() {
    TEST_BODY(smart_pointers);
}

#ifndef JSON_TEST_ALL
int main() {
    TestBarePointers();
    TestSmartPointers();
}
#endif
