#pragma once
#ifndef __JSON_TEST_CLASSES_HPP__
#define __JSON_TEST_CLASSES_HPP__

#include "json_private_access.hpp"

#include <vector>
#include <tuple>
#include <map>
#include <memory>

class Simple {
public:
    Simple() : s(10) {};
private:
    int s;
    JSON_PRIVATE_ACCESS()
};


class MySimpleClass {
public:
    MySimpleClass() : m_int(10) {};

    int m_int;
};


class NestedContainer {
public:
    NestedContainer() : m_simpleClass(), m_int(20) {};
    MySimpleClass m_simpleClass;
    int m_int;
};


class Test {
public:
     static const char mychar = 'x';
};


class Nested {
public:
     // JSON_INHERITS(Nested, Test);
     Simple mytest;
};


class HasVector { 
public:
     std::vector<char> myvec;
};


#if !defined(_MSC_VER) || _MSC_VER >= 1800
class UsesTuple {
public:
    std::tuple<char, int, double, long> mytuple;
};

#endif

class HasMap {
public:
    std::map<int, double> mymap;
};


class HasPTR {
public:
    int* myptr;
};


class HasArray {
public:
    int myintarr[3];
    int mynestedarr[3][3];
};


#ifndef _MSC_VER
/* Visual studio 2013.2 can't move-construct unique_ptrs */
class HasSmrtPtrs {
public:
    std::unique_ptr<int> mysmartint;
    std::shared_ptr<int> myshrdint;
};

#endif

class HasStrings {
public:
    std::string mystring;
    std::wstring mywstring;
};

#endif
