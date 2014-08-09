#include "json.hpp"

#include <vector>

JSON_CLASS(Test, 
public:
     /* variable type, variable name, json key */
     static const JSON_VAR(char, mychar, mychar, = 'x');
);

JSON_CLASS(Nested, 
public:
     /* variable type, variable name, json key */
     JSON_VAR(Test, mytest, mytest);
);

JSON_CLASS(HasVector, 
public:
     /* variable type, variable name, json key */
     JSON_VAR(std::vector<char>, myvec, myvec);
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

    return 0;
}
