#pragma once
#ifndef __JSON_TEST_INTEGRALS_HPP__
#define __JSON_TEST_INTEGRALS_HPP__

#include <limits>
#include <cstdint>

void TestIntegrals();

struct bools {
    bools() : t(true), f(false) {}

    bool t;
    bool f;
};

struct shorts {
    shorts()
        :   min_s(std::numeric_limits<short>::min()),
            max_s(std::numeric_limits<short>::max()),
            min_us(std::numeric_limits<unsigned short>::min()),
            max_us(std::numeric_limits<unsigned short>::max())
    {}

    short min_s;
    short max_s;
    unsigned short min_us;
    unsigned short max_us;
};

struct ints {
    ints()
        :   min_i(std::numeric_limits<int>::min()),
            max_i(std::numeric_limits<int>::max()),
            min_ui(std::numeric_limits<unsigned int>::min()),
            max_ui(std::numeric_limits<unsigned int>::max())
    {}

    int min_i;
    int max_i;
    unsigned int min_ui;
    unsigned int max_ui;
};

struct longs {
    longs()
        :   min_l(std::numeric_limits<long>::min()),
            max_l(std::numeric_limits<long>::max()),
            min_ul(std::numeric_limits<unsigned long>::min()),
            max_ul(std::numeric_limits<unsigned long>::max())
    {}

    long min_l;
    long max_l;
    unsigned long min_ul;
    unsigned long max_ul;
};

struct longlongs {
    longlongs()
        :   min_ll(std::numeric_limits<long long>::min()),
            max_ll(std::numeric_limits<long long>::max()),
            min_ull(std::numeric_limits<unsigned long long>::min()),
            max_ull(std::numeric_limits<unsigned long long>::max())
    {}

    long long min_ll;
    long long max_ll;
    unsigned long long min_ull;
    unsigned long long max_ull;
};

struct int8_ts {
    int8_ts()
        :   min_i8(std::numeric_limits<int8_t>::min()),
            max_i8(std::numeric_limits<int8_t>::max()),
            min_ui8(std::numeric_limits<uint8_t>::min()),
            max_ui8(std::numeric_limits<uint8_t>::max())
    {}

    int8_t min_i8;
    int8_t max_i8;
    uint8_t min_ui8;
    uint8_t max_ui8;
};

struct int16_ts {
    int16_ts()
        :   min_i16(std::numeric_limits<int16_t>::min()),
            max_i16(std::numeric_limits<int16_t>::max()),
            min_ui16(std::numeric_limits<uint16_t>::min()),
            max_ui16(std::numeric_limits<uint16_t>::max())
    {}

    int16_t min_i16;
    int16_t max_i16;
    uint16_t min_ui16;
    uint16_t max_ui16;
};

struct int32_ts {
    int32_ts()
        :   min_i32(std::numeric_limits<int32_t>::min()),
            max_i32(std::numeric_limits<int32_t>::max()),
            min_ui32(std::numeric_limits<uint32_t>::min()),
            max_ui32(std::numeric_limits<uint32_t>::max())
    {}

    int32_t min_i32;
    int32_t max_i32;
    uint32_t min_ui32;
    uint32_t max_ui32;
};

struct int64_ts {
    int64_ts()
        :   min_i64(std::numeric_limits<int64_t>::min()),
            max_i64(std::numeric_limits<int64_t>::max()),
            min_ui64(std::numeric_limits<uint64_t>::min()),
            max_ui64(std::numeric_limits<uint64_t>::max())
    {}

    int64_t min_i64;
    int64_t max_i64;
    uint64_t min_ui64;
    uint64_t max_ui64;
};

#endif
