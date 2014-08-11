#include "json.hpp"

#include <vector>
#include <tuple>
#include <map>

JSON_CLASS(Test,
public:
     /* variable type, variable name */
     static const JSON_VAR(char, mychar, = 'x');
);

JSON_CLASS(Nested,
public:
     /* variable type, variable name */
     JSON_VAR(Test, mytest);
);

JSON_CLASS(HasVector, 
public:
     /* variable type, variable name */
     JSON_VAR(std::vector<char>, myvec);
);

JSON_CLASS(UsesTuple,
public:
     /* variable type, variable name */
    typedef std::tuple<char, int, double, long> tupletype;
    JSON_VAR(tupletype, mytuple);
);

JSON_CLASS(HasMap,
public:
     /* variable type, variable name */
    typedef std::map<int, double> maptype;
    JSON_VAR(maptype, mymap);
);

JSON_CLASS(HasPTR,
public:
     /* variable type, variable name */
    JSON_VAR(int*, myptr);
);

JSON_CLASS(HasArray,
public:
     /* variable type, variable name */
    JSON_ARRAY(int, myintarr, [3]);
    JSON_ARRAY(int, mynestedarr, [3][3]);
);

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

    return 0;
}
