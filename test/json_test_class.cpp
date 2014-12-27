#include "json_test_classes.hpp"
#include "json.hpp"

JSON_ENABLE(Simple, (s));

typedef Templated<int> TInt;
JSON_ENABLE(TInt, (val));

JSON_ENABLE(MySimpleClass, (m_int));

JSON_ENABLE(NestedContainer, (m_simpleClass), (m_int, "not_nested_int"));

JSON_ENABLE(Test, (mychar, "testchar"), (mychar2));

JSON_ENABLE(Nested, (mytest, "nested_class"));

JSON_ENABLE(HasVector, (myvec));

JSON_ENABLE(UsesTuple, (mytuple));

JSON_ENABLE(HasMap, (mymap));

JSON_ENABLE(HasPTR, (myptr));

JSON_ENABLE(HasSmrtPtrs, (mysmartint), (myshrdint));
