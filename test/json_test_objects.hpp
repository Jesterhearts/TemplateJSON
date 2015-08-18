#pragma once
#ifndef __JSON_TEST_OBJECTS_HPP__
#define __JSON_TEST_OBJECTS_HPP__

#include "json_private_access.hpp"

struct basic {
    int i;
};

struct nested {
    nested(basic&& b) : basic1(b) {}
    nested() : basic1{10} {}

    basic basic1;
};

class private_member {
public:
    private_member(int i) : member(i) {}

    private_member() : member(7) {}

private:
    int member;

    JSON_PRIVATE_ACCESS();
};

struct const_member {
    const_member(int c) : c(c) {}

    const_member() : c(13) {}

    const int c;
};

#endif
