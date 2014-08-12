#include "json.hpp"

#include <vector>
#include <tuple>
#include <map>
#include <memory>

JSON_CLASS(Test,
public:
     static const JSON_VAR(char, mychar, = 'x');
);

JSON_CLASS(Nested,
public:
     JSON_VAR(Test, mytest);
);

JSON_CLASS(HasVector, 
public:
     JSON_VAR(std::vector<char>, myvec);
);

JSON_CLASS(UsesTuple,
public:
    typedef std::tuple<char, int, double, long> tupletype;
    JSON_VAR(tupletype, mytuple);
);

JSON_CLASS(HasMap,
public:
    typedef std::map<int, double> maptype;
    JSON_VAR(maptype, mymap);
);

JSON_CLASS(HasPTR,
public:
    JSON_VAR(int*, myptr);
);

JSON_CLASS(HasArray,
public:
    JSON_ARRAY(int, myintarr, [3]);
    JSON_ARRAY(int, mynestedarr, [3][3]);
);

JSON_CLASS(HasSmrtPtrs,
public:
    JSON_VAR(std::unique_ptr<int>, mysmartint);
    JSON_VAR(std::shared_ptr<int>, myshrdint);
);

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
