#include "json.hpp"

#include <vector>
#include <tuple>
#include <map>
#include <memory>

class Simple : public JSON::JSONBase<Simple> {
public:
    Simple() : s(10) {};
private:
    int s;
    JSON_PRIVATE_ACCESS()
};

JSON_ENABLE(Simple, (s))

class Test : public JSON::JSONBase<Test> {
public:
     static const char mychar = 'x';
};

JSON_ENABLE(Test, (mychar, "testchar"))

class Nested : public JSON::JSONBase<Nested> /*, Test */ {
public:
     // JSON_INHERITS(Nested, Test);
     Simple mytest;
};

JSON_ENABLE(Nested, (mytest, "nested_class"))

class HasVector : public  JSON::JSONBase<HasVector> { 
public:
     std::vector<char> myvec;
};

JSON_ENABLE(HasVector, (myvec))

#if !defined(_MSC_VER) || _MSC_VER >= 1800
class UsesTuple : public JSON::JSONBase<UsesTuple> {
public:
    std::tuple<char, int, double, long> mytuple;
};

JSON_ENABLE(UsesTuple, (mytuple))
#endif

class HasMap : public JSON::JSONBase<HasMap> {
public:
    std::map<int, double> mymap;
};

JSON_ENABLE(HasMap, (mymap))

class HasPTR : public JSON::JSONBase<HasPTR> {
public:
    int* myptr;
};

JSON_ENABLE(HasPTR, (myptr))

class HasArray : public JSON::JSONBase<HasArray> {
public:
    int myintarr[3];
    int mynestedarr[3][3];
};

JSON_ENABLE(HasArray, (myintarr), (mynestedarr))

#ifndef _MSC_VER
/* Visual studio 2013.2 can't move-construct unique_ptrs */
class HasSmrtPtrs : public JSON::JSONBase<HasSmrtPtrs> {
public:
    std::unique_ptr<int> mysmartint;
    std::shared_ptr<int> myshrdint;
};

JSON_ENABLE(HasSmrtPtrs, (mysmartint), (myshrdint))
#endif

class HasStrings : public JSON::JSONBase<HasStrings> {
public:
    std::string mystring;
    std::wstring mywstring;
};

JSON_ENABLE(HasStrings, (mystring), (mywstring))

const char Test::mychar;

int main() {
    JSON::stringt json;

#ifdef JSON_USE_WIDE_STRINGS
#define output std::wcout
#else
#define output std::cout
#endif

    Simple simple;
    json = simple.ToJSON();
    output << json << std::endl;

    try
    {
        Simple simple2 = Simple::FromJSON(json);
        json = simple2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Test a;
    json = a.ToJSON();
    output << json << std::endl;

    try
    {
        Test t2 = Test::FromJSON(json);
        json = t2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    Nested n;
    json = n.ToJSON();
    output << json << std::endl;
    try
    {
        Nested n2 = Nested::FromJSON(json);
        json = n2.ToJSON();
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
    json = v.ToJSON();
    output << json << std::endl;

    try
    {
        HasVector v2 = HasVector::FromJSON(json);
        json = v2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }


#if !defined(_MSC_VER) || _MSC_VER >= 1800
    UsesTuple ut;
    ut.mytuple = std::make_tuple('t', 10, 12.5, 100);
    json = ut.ToJSON();
    output << json << std::endl;
    try
    {
        UsesTuple ut2 = UsesTuple::FromJSON(json);
        json = ut2.ToJSON();
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
    json = hm.ToJSON();
    output << json << std::endl;

    try
    {
        HasMap hm2 = HasMap::FromJSON(json);
        json = hm2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    HasPTR p;
    p.myptr = nullptr;
    json = p.ToJSON();
    output << json << std::endl;
    try
    {
        HasPTR hp2 = HasPTR::FromJSON(json);
        json = hp2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    p.myptr = new int(10);
    json = p.ToJSON();
    output << json << std::endl;
    delete p.myptr;
    try
    {
        HasPTR hp2 = HasPTR::FromJSON(json);
        json = hp2.ToJSON();
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
    json = ha.ToJSON();
    output << json << std::endl;
    try
    {
        HasArray ha2 = HasArray::FromJSON(json);
        json = ha2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

#ifndef _MSC_VER
    HasSmrtPtrs hsp;
    json = hsp.ToJSON();
    output << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = HasSmrtPtrs::FromJSON(json);
        json = hsp2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    hsp.mysmartint.reset(new int(10));
    hsp.myshrdint.reset(new int(11));
    json = hsp.ToJSON();
    output << json << std::endl;
    try
    {
        HasSmrtPtrs hsp2 = HasSmrtPtrs::FromJSON(json);
        json = hsp2.ToJSON();
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
    json = hs.ToJSON();
    output << json << std::endl;
    try
    {
        HasStrings hs2 = HasStrings::FromJSON(json);
        json = hs2.ToJSON();
        output << JSON_ST("deserialized: ") << json << std::endl << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
