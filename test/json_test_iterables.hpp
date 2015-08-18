#pragma once
#ifndef __JSON_TEST_ITERABLES_HPP__
#define __JSON_TEST_ITERABLES_HPP__

#include <map>
#include <tuple>
#include <vector>

struct iterables {
    iterables(std::vector<int>&& v, std::map<int, int>&& m) : vector(v), map(m) {}
    iterables() : vector{1, 2, 3, 4, 5}, map{{0, 1}, {2, 3}, {4, 5}} {}

    std::vector<int> vector;
    std::map<int, int> map;
};

struct tuples {
    tuples(std::tuple<char, int, double, long>&& t) : tuple(t) {};

    tuples() : tuple{'a', 1, 3.14, 42l} {}

    std::tuple<char, int, double, long> tuple;
};

#endif
