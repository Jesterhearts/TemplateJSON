#pragma once
#ifndef __TEST_COMMON_HPP__
#define __TEST_COMMON_HPP__

#include "json.hpp"

#include <iostream>
#include <string>

#define TEST_BODY(classname)                    \
    std::cout << #classname ": " << std::endl;  \
    RunTest<classname>();

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

#endif
