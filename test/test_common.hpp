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
    json = tjson::to_json<TestType>(t1);

    std::cout << "\tserialized: " << json << std::endl;

    try
    {
        TestType t2 = tjson::from_json<TestType>(json);
        std::string json2 = tjson::to_json<TestType>(t2);
        if(json == json2) {
            std::cout << "\tSuccess!" << std::endl;
        } else {
            std::cout << "\tFailure" << std::endl;
        }

        std::cout << "\tdeserialized: " << json2 << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }
}

#endif
