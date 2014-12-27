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
    Simple(int s) : s(s) {};

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
    MySimpleClass(int i) : m_int(i) {};

    MySimpleClass() : m_int(10) {};

    int m_int;
};


class NestedContainer {
public:
    NestedContainer(MySimpleClass&& m, int i) : m_simpleClass(m), m_int(i) {}

    NestedContainer() : m_simpleClass(), m_int(20) {};
    MySimpleClass m_simpleClass;
    int m_int;
};

class Test {
public:
    Test(char c1, char c2) : mychar(c1), mychar2(c2) {}

    Test() : mychar('x'), mychar2('y') {}
    const char mychar;
    const char mychar2;
};


class Nested : Test {
public:
    Nested() {}

    Nested(Simple&& s) : mytest(s) {}
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

    HasSmrtPtrs(std::unique_ptr<int>&& msi, std::shared_ptr<int>&& shrdi)
        :   mysmartint(std::move(msi)), myshrdint(std::move(shrdi)) {}

    HasSmrtPtrs(HasSmrtPtrs&& other)
        : mysmartint(std::move(other.mysmartint)), myshrdint(std::move(other.mysmartint)) {};

    std::unique_ptr<int> mysmartint;
    std::shared_ptr<int> myshrdint;
private:
    HasSmrtPtrs(const HasSmrtPtrs&) = delete;
};


#endif
