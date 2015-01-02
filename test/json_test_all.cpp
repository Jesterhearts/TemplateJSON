#define JSON_TEST_ALL
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

    return 0;
}
