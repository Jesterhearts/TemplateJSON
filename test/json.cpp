#include "json_functions.hpp"
#include "json_test_classes.hpp"
#include "json_test_integrals.hpp"

#include <iostream>
#include <sstream>

#ifndef _MSC_VER
const char Test::mychar;
const char Test::mychar2;
#endif

int main() {
    TestIntegrals();

    std::string json;

    Simple simple;
    json = JSON::ToJSON<Simple>(simple);
    std::cout << json << std::endl;

    try
    {
        Simple simple = JSON::FromJSON<Simple>(json);
        json = JSON::ToJSON<Simple>(simple);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    NestedContainer nc;
    json = JSON::ToJSON<NestedContainer>(nc);
    std::cout << json << std::endl;

    try
    {
        NestedContainer nc2 = JSON::FromJSON<NestedContainer>(json);
        json = JSON::ToJSON<NestedContainer>(nc2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Test a;
    json = JSON::ToJSON<Test>(a);
    std::cout << json << std::endl;

    try
    {
        Test t2 = JSON::FromJSON<Test>(json);
        json = JSON::ToJSON<Test>(t2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Nested n;
    json = JSON::ToJSON<Nested>(n);
    std::cout << json << std::endl;
    try
    {
        Nested n2 = JSON::FromJSON<Nested>(json);
        json = JSON::ToJSON<Nested>(n2);
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
    json = JSON::ToJSON<HasVector>(v);
    std::cout << json << std::endl;

    try
    {
        HasVector v2 = JSON::FromJSON<HasVector>(json);
        json = JSON::ToJSON<HasVector>(v2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    UsesTuple ut;
    ut.mytuple = std::make_tuple('t', 10, 12.5, 100);
    json = JSON::ToJSON<UsesTuple>(ut);
    std::cout << json << std::endl;
    try
    {
        UsesTuple ut2 = JSON::FromJSON<UsesTuple>(json);
        json = JSON::ToJSON<UsesTuple>(ut2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasMap hm;
    hm.mymap.insert(std::make_pair(1337, 3.14));
    hm.mymap.insert(std::make_pair(314159, 100.0));
    json = JSON::ToJSON<HasMap>(hm);
    std::cout << json << std::endl;

    try
    {
        HasMap hm2 = JSON::FromJSON<HasMap>(json);
        json = JSON::ToJSON<HasMap>(hm2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasPTR p;
    p.myptr = nullptr;
    json = JSON::ToJSON<HasPTR>(p);
    std::cout << json << std::endl;
    try
    {
        HasPTR hp2 = JSON::FromJSON<HasPTR>(json);
        json = JSON::ToJSON<HasPTR>(hp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    p.myptr = new int(10);
    json = JSON::ToJSON<HasPTR>(p);
    std::cout << json << std::endl;
    delete p.myptr;
    try
    {
        HasPTR hp2 = JSON::FromJSON<HasPTR>(json);
        json = JSON::ToJSON<HasPTR>(hp2);
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
    json = JSON::ToJSON<HasArray>(ha);
    std::cout << json << std::endl;
    try
    {
        HasArray ha2 = JSON::FromJSON<HasArray>(json);
        json = JSON::ToJSON<HasArray>(ha2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasSmrtPtrs hsp;
    json = JSON::ToJSON<HasSmrtPtrs>(hsp);
    std::cout << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = JSON::FromJSON<HasSmrtPtrs>(json);
        json = JSON::ToJSON<HasSmrtPtrs>(hsp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    hsp.mysmartint.reset(new int(10));
    hsp.myshrdint.reset(new int(11));
    json = JSON::ToJSON<HasSmrtPtrs>(hsp);
    std::cout << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = JSON::FromJSON<HasSmrtPtrs>(json);
        json = JSON::ToJSON<HasSmrtPtrs>(hsp2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasStrings hs;
    hs.mystring = "string";
    hs.mywstring = L"wstring";
    json = JSON::ToJSON<HasStrings>(hs);
    std::cout << json << std::endl;
    try
    {
        HasStrings hs2 = JSON::FromJSON<HasStrings>(json);
        json = JSON::ToJSON<HasStrings>(hs2);
        std::cout << "deserialized: " << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
