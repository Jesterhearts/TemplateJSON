#pragma once
#ifndef __JSON_TEST_ALTERNATIVE_PARSING_HPP__
#define __JSON_TEST_ALTERNATIVE_PARSING_HPP__

struct custom_key {
    custom_key() : i(27) {}
    custom_key(int i) : i(i) {}

    int i;
};

//TODO
struct out_of_order {

};

#endif
