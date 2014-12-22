#define JSON_TEST_ALL
#include "json_test_class.cpp"
#include "json_test_floats.cpp"
#include "json_test_integrals.cpp"
#include "json_test_strings.cpp"

const char Test::mychar;
const char Test::mychar2;

int main() {
    TestIntegrals();
    TestFloats();
    TestStrings();

    return 0;
}
