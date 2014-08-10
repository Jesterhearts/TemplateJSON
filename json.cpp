#include "json.hpp"

#include <vector>
#include <tuple>
#include <map>

JSON_CLASS(Test, 
public:
     /* variable type, variable name, json key */
     static const JSON_VAR(char, mychar, = 'x');
);

JSON_CLASS(Nested, 
public:
     /* variable type, variable name, json key */
     JSON_VAR(Test, mytest);
);

JSON_CLASS(HasVector, 
public:
     /* variable type, variable name, json key */
     JSON_VAR(std::vector<char>, myvec);
);

JSON_CLASS(UsesTuple, 
public:
     /* variable type, variable name, json key */
    typedef std::tuple<char, int, double, long> tupletype;
    JSON_VAR(tupletype, mytuple);
);


JSON_CLASS(HasMap, 
public:
     /* variable type, variable name, json key */
    typedef std::map<int, double> maptype;
    JSON_VAR(maptype, mymap);
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
    json = hm.ToJSON();
    std::wcout << json << std::endl;

    return 0;
}
