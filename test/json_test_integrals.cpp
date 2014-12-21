#include "json_test_integrals.hpp"
#include "test_common.hpp"
#include "json.hpp"

JSON_ENABLE(bools, (t), (f))
JSON_ENABLE(shorts, (min_s), (max_s), (min_us), (max_us));
JSON_ENABLE(ints, (min_i), (max_i), (min_ui), (max_ui));
JSON_ENABLE(uchars, (min_uc), (max_uc), (min_uwc), (max_uwc));
JSON_ENABLE(longs, (min_l), (max_l), (min_ul), (max_ul));
JSON_ENABLE(longlongs, (min_ll), (max_ll), (min_ull), (max_ull));
JSON_ENABLE(int8_ts, (min_i8), (max_i8), (min_ui8), (max_ui8));
JSON_ENABLE(int16_ts, (min_i16), (max_i16), (min_ui16), (max_ui16));
JSON_ENABLE(int32_ts, (min_i32), (max_i32), (min_ui32), (max_ui32));
JSON_ENABLE(int64_ts, (min_i64), (max_i64), (min_ui64), (max_ui64));

void TestBools() {
    TEST_BODY(bools);
}

void TestShorts() {
    TEST_BODY(shorts);
}

void TestInts() {
    TEST_BODY(ints);
}

void TestUChars() {
    TEST_BODY(uchars);
}

void TestLongs() {
    TEST_BODY(longs);
}

void TestLongLongs() {
    TEST_BODY(longlongs);
}

void TestI8s() {
    TEST_BODY(int8_ts);
}

void TestI16s() {
    TEST_BODY(int16_ts);
}

void TestI32s() {
    TEST_BODY(int32_ts);
}

void TestI64s() {
    TEST_BODY(int64_ts);
}

void TestIntegrals() {
    TestBools();
    TestShorts();
    TestInts();
    TestUChars();
    TestLongs();
    TestLongLongs();
    TestI8s();
    TestI16s();
    TestI32s();
    TestI64s();
}
