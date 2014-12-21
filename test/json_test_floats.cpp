#include "json_test_floats.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(floats, (min_f), (max_f));
JSON_ENABLE(doubles, (min_d), (max_d));

void TestSinglePrecision() {
    TEST_BODY(floats)
}

void TestDoublePrecision() {
    TEST_BODY(doubles)
}

void TestFloats() {
    TestSinglePrecision();
    TestDoublePrecision();
}
