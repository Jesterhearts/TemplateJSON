#include "json.hpp"

#include <vector>
#include <tuple>
#include <map>
#include <memory>

class Test : public JSON::JSONBase<Test> {
public:
     static const char mychar = 'x';
};

JSON_ENABLE(Test, (mychar, L"testchar"));

class Nested : public JSON::JSONBase<Nested>, Test {
public:
     JSON_INHERITS(Nested, Test);
     Test mytest;
};

JSON_ENABLE(Nested, (mytest));

class HasVector : public  JSON::JSONBase<HasVector> { 
public:
     std::vector<char> myvec;
};

JSON_ENABLE(HasVector, (myvec));

class UsesTuple : public JSON::JSONBase<UsesTuple> {
public:
    std::tuple<char, int, double, long> mytuple;
};

JSON_ENABLE(UsesTuple, (mytuple));

class HasMap : public JSON::JSONBase<HasMap> {
public:
    std::map<int, double> mymap;
};

JSON_ENABLE(HasMap, (mymap));

class HasPTR : public JSON::JSONBase<HasPTR> {
public:
    int* myptr;
};

JSON_ENABLE(HasPTR, (myptr));

class HasArray : public JSON::JSONBase<HasArray> {
public:
    int myintarr[3];
    int mynestedarr [3][3];
};

JSON_ENABLE(HasArray, (myintarr), (mynestedarr));

class HasSmrtPtrs : public JSON::JSONBase<HasSmrtPtrs> {
public:
    std::unique_ptr<int> mysmartint;
    std::shared_ptr<int> myshrdint;
};

JSON_ENABLE(HasSmrtPtrs, (mysmartint), (myshrdint));

const char Test::mychar;

int main() {
    std::wstring json;
    Test a;
    json = a.ToJSON();
    std::wcout << json << std::endl;


    Nested n;
    json = n.ToJSON();
    std::wcout << json << std::endl;

    HasVector v;
    v.myvec.push_back('a');
    v.myvec.push_back('b');
    v.myvec.push_back('c');
    json = v.ToJSON();
    std::wcout << json << std::endl;

    UsesTuple ut;
    ut.mytuple = std::make_tuple('t', 10, 12.5, 100);
    json = ut.ToJSON();
    std::wcout << json << std::endl;

    HasMap hm;
    hm.mymap.insert(std::make_pair(1337, 3.14));
    hm.mymap.insert(std::make_pair(3.14159, 100.0));
    json = hm.ToJSON();
    std::wcout << json << std::endl;

    HasPTR p;
    p.myptr = nullptr;
    json = p.ToJSON();
    std::wcout << json << std::endl;

    p.myptr = new int(10);
    json = p.ToJSON();
    std::wcout << json << std::endl;
    delete p.myptr;

    HasArray ha;
    ha.myintarr[0] = 1;
    ha.myintarr[1] = 1;
    ha.myintarr[2] = 2;
    json = ha.ToJSON();
    std::wcout << json << std::endl;

    HasSmrtPtrs hsp;
    json = hsp.ToJSON();
    std::wcout << json << std::endl;

    hsp.mysmartint.reset(new int(10));
    hsp.myshrdint.reset(new int(11));
    json = hsp.ToJSON();
    std::wcout << json << std::endl;

    return 0;
}
