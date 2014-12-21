#pragma once
#ifndef __JSON_TEST_FLOATS_HPP__
#define __JSON_TEST_FLOATS_HPP__

#include <limits>

void TestFloats();

struct floats {
    floats()
        :   min_f(std::numeric_limits<float>::min()),
            max_f(std::numeric_limits<float>::max())
    {}

    float min_f;
    float max_f;
};

struct doubles {
    doubles()
        :   min_d(std::numeric_limits<double>::min()),
            max_d(std::numeric_limits<double>::max())
    {}

    double min_d;
    double max_d;
};

#endif
