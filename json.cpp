#include "json_functions.hpp"
#include "json_test_classes.hpp"

#include <iostream>
#include <sstream>

const char Test::mychar;

int main() {
    JSON::stringt json;

#ifdef JSON_USE_WIDE_STRINGS
#define output std::wcout
#else
#define output std::cout
#endif

    Simple simple;
    json = JSON::ToJSON<Simple>(simple);
    output << json << std::endl;

    try
    {
        Simple simple2 = JSON::FromJSON<Simple>(json);
        json = JSON::ToJSON<Simple>(simple2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }


    NestedContainer nc;
    json = JSON::ToJSON<NestedContainer>(nc);
    output << json << std::endl;

    Test a;
    json = JSON::ToJSON<Test>(a);
    output << json << std::endl;

    try
    {
        Test t2 = JSON::FromJSON<Test>(json);
        json = JSON::ToJSON<Test>(t2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Nested n;
    json = JSON::ToJSON<Nested>(n);
    output << json << std::endl;
    try
    {
        Nested n2 = JSON::FromJSON<Nested>(json);
        json = JSON::ToJSON<Nested>(n2);
        output << JSON_ST("deserialized: ") << json << std::endl;
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
    output << json << std::endl;

    try
    {
        HasVector v2 = JSON::FromJSON<HasVector>(json);
        json = JSON::ToJSON<HasVector>(v2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }


#if !defined(_MSC_VER) || _MSC_VER >= 1800
    UsesTuple ut;
    ut.mytuple = std::make_tuple('t', 10, 12.5, 100);
    json = JSON::ToJSON<UsesTuple>(ut);
    output << json << std::endl;
    try
    {
        UsesTuple ut2 = JSON::FromJSON<UsesTuple>(json);
        json = JSON::ToJSON<UsesTuple>(ut2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }
#endif

    HasMap hm;
    hm.mymap.insert(std::make_pair(1337, 3.14));
    hm.mymap.insert(std::make_pair(314159, 100.0));
    json = JSON::ToJSON<HasMap>(hm);
    output << json << std::endl;

    try
    {
        HasMap hm2 = JSON::FromJSON<HasMap>(json);
        json = JSON::ToJSON<HasMap>(hm2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasPTR p;
    p.myptr = nullptr;
    json = JSON::ToJSON<HasPTR>(p);
    output << json << std::endl;
    try
    {
        HasPTR hp2 = JSON::FromJSON<HasPTR>(json);
        json = JSON::ToJSON<HasPTR>(hp2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    p.myptr = new int(10);
    json = JSON::ToJSON<HasPTR>(p);
    output << json << std::endl;
    delete p.myptr;
    try
    {
        HasPTR hp2 = JSON::FromJSON<HasPTR>(json);
        json = JSON::ToJSON<HasPTR>(hp2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
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
    output << json << std::endl;
    try
    {
        HasArray ha2 = JSON::FromJSON<HasArray>(json);
        json = JSON::ToJSON<HasArray>(ha2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

#ifndef _MSC_VER
    HasSmrtPtrs hsp;
    json = JSON::ToJSON<HasSmrtPtrs>(hsp);
    output << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = JSON::FromJSON<HasSmrtPtrs>(json);
        json = JSON::ToJSON<HasSmrtPtrs>(hsp2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    hsp.mysmartint.reset(new int(10));
    hsp.myshrdint.reset(new int(11));
    json = JSON::ToJSON<HasSmrtPtrs>(hsp);
    output << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = JSON::FromJSON<HasSmrtPtrs>(json);
        json = JSON::ToJSON<HasSmrtPtrs>(hsp2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }
#endif

    HasStrings hs;
    hs.mystring = "string";
    hs.mywstring = L"wstring";
    json = JSON::ToJSON<HasStrings>(hs);
    output << json << std::endl;
    try
    {
        HasStrings hs2 = JSON::FromJSON<HasStrings>(json);
        json = JSON::ToJSON<HasStrings>(hs2);
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
