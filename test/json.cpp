#include "json_functions.hpp"
#include "json_test_classes.hpp"
#include "json_test_integrals.hpp"
#include "json_test_floats.hpp"
#include "json_test_strings.hpp"

#include <iostream>
#include <sstream>

#ifndef _MSC_VER
const char Test::mychar;
const char Test::mychar2;
#endif

int main() {
    TestIntegrals();
    TestFloats();
    TestStrings();

    std::string json;

    Simple simple;
    json = tjson::to_json<Simple>(simple);
    std::cout << json << std::endl;

    try
    {
        Simple simple = tjson::from_json<Simple>(json);
        json = tjson::to_json<Simple>(simple);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    NestedContainer nc;
    json = tjson::to_json<NestedContainer>(nc);
    std::cout << json << std::endl;

    try
    {
        NestedContainer nc2 = tjson::from_json<NestedContainer>(json);
        json = tjson::to_json<NestedContainer>(nc2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Test a;
    json = tjson::to_json<Test>(a);
    std::cout << json << std::endl;

    try
    {
        Test t2 = tjson::from_json<Test>(json);
        json = tjson::to_json<Test>(t2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Nested n;
    json = tjson::to_json<Nested>(n);
    std::cout << json << std::endl;
    try
    {
        Nested n2 = tjson::from_json<Nested>(json);
        json = tjson::to_json<Nested>(n2);
        std::cout << "deserialized: " << json << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasVector v;
    v.myvec.push_back('a');
    v.myvec.push_back('b');
    v.myvec.push_back('c');
    json = tjson::to_json<HasVector>(v);
    std::cout << json << std::endl;

    try
    {
        HasVector v2 = tjson::from_json<HasVector>(json);
        json = tjson::to_json<HasVector>(v2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    UsesTuple ut;
    ut.mytuple = std::make_tuple('t', 10, 12.5, 100);
    json = tjson::to_json<UsesTuple>(ut);
    std::cout << json << std::endl;
    try
    {
        UsesTuple ut2 = tjson::from_json<UsesTuple>(json);
        json = tjson::to_json<UsesTuple>(ut2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasMap hm;
    hm.mymap.insert(std::make_pair(1337, 3.14));
    hm.mymap.insert(std::make_pair(314159, 100.0));
    json = tjson::to_json<HasMap>(hm);
    std::cout << json << std::endl;

    try
    {
        HasMap hm2 = tjson::from_json<HasMap>(json);
        json = tjson::to_json<HasMap>(hm2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasPTR p;
    p.myptr = nullptr;
    json = tjson::to_json<HasPTR>(p);
    std::cout << json << std::endl;
    try
    {
        HasPTR hp2 = tjson::from_json<HasPTR>(json);
        json = tjson::to_json<HasPTR>(hp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    p.myptr = new int(10);
    json = tjson::to_json<HasPTR>(p);
    std::cout << json << std::endl;
    delete p.myptr;
    try
    {
        HasPTR hp2 = tjson::from_json<HasPTR>(json);
        json = tjson::to_json<HasPTR>(hp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasArray ha;
    ha.myintarr[0] = 1;
    ha.myintarr[1] = 1;
    ha.myintarr[2] = 2;
    json = tjson::to_json<HasArray>(ha);
    std::cout << json << std::endl;
    try
    {
        HasArray ha2 = tjson::from_json<HasArray>(json);
        json = tjson::to_json<HasArray>(ha2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasSmrtPtrs hsp;
    json = tjson::to_json<HasSmrtPtrs>(hsp);
    std::cout << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = tjson::from_json<HasSmrtPtrs>(json);
        json = tjson::to_json<HasSmrtPtrs>(hsp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    hsp.mysmartint.reset(new int(10));
    hsp.myshrdint.reset(new int(11));
    json = tjson::to_json<HasSmrtPtrs>(hsp);
    std::cout << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = tjson::from_json<HasSmrtPtrs>(json);
        json = tjson::to_json<HasSmrtPtrs>(hsp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
