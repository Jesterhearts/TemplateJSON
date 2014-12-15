#include "json_test_classes.hpp"
#include "json.hpp"

JSON_ENABLE(Simple, (s))

JSON_ENABLE(Simple2, (s), (s2))

JSON_ENABLE(MySimpleClass, (m_int))

JSON_ENABLE(NestedContainer, (m_simpleClass), (m_int, "not_nested_int"))

JSON_ENABLE(Test, (mychar, "testchar"), (mychar2))

JSON_ENABLE(Nested, (mytest, "nested_class"))

JSON_ENABLE(HasVector, (myvec))

#if !defined(_MSC_VER) || _MSC_VER >= 1800
JSON_ENABLE(UsesTuple, (mytuple))
#endif

JSON_ENABLE(HasMap, (mymap))

JSON_ENABLE(HasPTR, (myptr))

JSON_ENABLE(HasArray, (myintarr), (mynestedarr))

JSON_ENABLE(HasSmrtPtrs, (mysmartint), (myshrdint))

JSON_ENABLE(HasStrings, (mystring), (mywstring))
