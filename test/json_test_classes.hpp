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

template<typename T>
class Templated {
public:
    T val;
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
     static const char mychar2 = 'y';
};


class Nested : Test {
public:
     // JSON_INHERITS(Nested, Test);
     Simple mytest;
};


class HasVector { 
public:
     std::vector<char> myvec;
};

class UsesTuple {
public:
    std::tuple<char, int, double, long> mytuple;
};

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


class HasSmrtPtrs {
public:
    HasSmrtPtrs() {}
    HasSmrtPtrs(HasSmrtPtrs&& other)
        : mysmartint(std::move(other.mysmartint)), myshrdint(std::move(other.mysmartint)) {};

    std::unique_ptr<int> mysmartint;
    std::shared_ptr<int> myshrdint;
private:
    HasSmrtPtrs(const HasSmrtPtrs&) = delete;
};


#endif
