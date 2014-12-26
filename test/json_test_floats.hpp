#pragma once
#ifndef __JSON_TEST_FLOATS_HPP__
#define __JSON_TEST_FLOATS_HPP__

#include <limits>

void TestFloats();

struct floats {
    floats(float f1, float f2) : min_f(f1), max_f(f2) {}

    floats()
        :   min_f(std::numeric_limits<float>::min()),
            max_f(std::numeric_limits<float>::max())
    {}

    float min_f;
    float max_f;
};

struct doubles {
    doubles(double d1, double d2) : min_d(d1), max_d(d2) {}

    doubles()
        :   min_d(std::numeric_limits<double>::min()),
            max_d(std::numeric_limits<double>::max())
    {}

    double min_d;
    double max_d;
};

#endif
