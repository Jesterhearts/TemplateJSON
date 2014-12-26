#pragma once
#ifndef __JSON_TEST_INTEGRALS_HPP__
#define __JSON_TEST_INTEGRALS_HPP__

#include <limits>
#include <cstdint>

void TestIntegrals();

struct bools {
    bools(bool t, bool f) : t(t), f(f) {}

    bools() : t(true), f(false) {}

    bool t;
    bool f;
};

struct shorts {
    shorts(short v1, short v2, unsigned short v3, unsigned short v4)
        :   min_s(v1), max_s(v2), min_us(v3), max_us(v4) {}

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
    ints(int v1, int v2, unsigned int v3, unsigned int v4)
        :   min_i(v1), max_i(v2), min_ui(v3), max_ui(v4) {}

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

struct uchars {
    uchars(unsigned char v1, unsigned char v2, unsigned wchar_t v3, unsigned wchar_t v4)
        :   min_uc(v1), max_uc(v2), min_uwc(v3), max_uwc(v4) {}

    uchars()
        :   min_uc(std::numeric_limits<unsigned char>::min()),
            max_uc(std::numeric_limits<unsigned char>::max()),
            min_uwc(std::numeric_limits<unsigned wchar_t>::min()),
            max_uwc(std::numeric_limits<unsigned wchar_t>::max())
    {}

    unsigned char min_uc;
    unsigned char max_uc;

    unsigned wchar_t min_uwc;
    unsigned wchar_t max_uwc;
};

struct longs {
    longs(long v1, long v2, unsigned long v3, unsigned long v4)
        :   min_l(v1), max_l(v2), min_ul(v3), max_ul(v4) {}

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
    longlongs(long long v1, long long v2, unsigned long long v3, unsigned long long v4)
        :   min_ll(v1), max_ll(v2), min_ull(v3), max_ull(v4) {}

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
    int8_ts(int8_t v1, int8_t v2, uint8_t v3, uint8_t v4)
        :   min_i8(v1), max_i8(v2), min_ui8(v3), max_ui8(v4) {}

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
    int16_ts(int16_t v1, int16_t v2, uint16_t v3, uint16_t v4)
        :   min_i16(v1), max_i16(v2), min_ui16(v3), max_ui16(v4) {}

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
    int32_ts(int32_t v1, int32_t v2, uint32_t v3, uint32_t v4)
        :   min_i32(v1), max_i32(v2), min_ui32(v3), max_ui32(v4) {}

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
    int64_ts(int64_t v1, int64_t v2, uint64_t v3, uint64_t v4)
        :   min_i64(v1), max_i64(v2), min_ui64(v3), max_ui64(v4) {}

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
