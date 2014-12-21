#include "json_test_integrals.hpp"
#include "json.hpp"

JSON_ENABLE(shorts, (min_s), (max_s), (min_us), (max_us));
JSON_ENABLE(ints, (min_i), (max_i), (min_ui), (max_ui));
JSON_ENABLE(longs, (min_l), (max_l), (min_ul), (max_ul));
JSON_ENABLE(longlongs, (min_ll), (max_ll), (min_ull), (max_ull));
JSON_ENABLE(int8_ts, (min_i8), (max_i8), (min_ui8), (max_ui8));
JSON_ENABLE(int16_ts, (min_i16), (max_i16), (min_ui16), (max_ui16));
JSON_ENABLE(int32_ts, (min_i32), (max_i32), (min_ui32), (max_ui32));
JSON_ENABLE(int64_ts, (min_i64), (max_i64), (min_ui64), (max_ui64));

template<typename TestType>
void RunTest() {
    std::string json;

    TestType t1;
    json = JSON::ToJSON<TestType>(t1);

    std::cout << "\tserialized: " << json << std::endl;

    try
    {
        TestType t2 = JSON::FromJSON<TestType>(json);
        json = JSON::ToJSON<TestType>(t2);
        std::cout << "\tdeserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void TestShorts() {
    std::cout << "shorts:" << std::endl;
    RunTest<shorts>();
}

void TestInts() {
    std::cout << "ints:" << std::endl;
    RunTest<ints>();
}

void TestLongs() {
    std::cout << "longs:" << std::endl;
    RunTest<longs>();
}

void TestLongLongs() {
    std::cout << "longlongs:" << std::endl;
    RunTest<longlongs>();
}

void TestI8s() {
    std::cout << "int8_ts:" << std::endl;
    RunTest<int8_ts>();
}

void TestI16s() {
    std::cout << "int16_ts:" << std::endl;
    RunTest<int16_ts>();
}

void TestI32s() {
    std::cout << "int32_ts:" << std::endl;
    RunTest<int32_ts>();
}

void TestI64s() {
    std::cout << "int64_ts:" << std::endl;
    RunTest<int64_ts>();
}

void TestIntegrals() {
    TestShorts();
    TestInts();
    TestLongs();
    TestLongLongs();
    TestI8s();
    TestI16s();
    TestI32s();
    TestI64s();
}
