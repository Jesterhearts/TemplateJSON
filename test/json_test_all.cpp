#ifndef JSON_TEST_ALL
#define JSON_TEST_ALL
#endif

#include "json_test_floats.cpp"
#include "json_test_integrals.cpp"
#include "json_test_strings.cpp"
#include "json_test_iterables.cpp"
#include "json_test_pointers.cpp"
#include "json_test_objects.cpp"
#include "json_test_alternative_parsing.cpp"

int main() {
    TestIntegrals();
    TestFloats();
    TestStrings();
    TestIterables();
    TestTuples();
    TestBarePointers();
    TestSmartPointers();
    TestNested();
    TestPrivateMember();
    TestConstMember();
    TestCustomKey();

    char c;
    std::cin >> c;

    return 0;
}
