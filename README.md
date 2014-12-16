T-JSON
============

This is a simple way to make a C++ class serialize to/deserialize from JSON.

---
To create a class:

- In your class .cpp file:
 1. `#include "json.hpp"`
 2. Use the macro `JSON_ENABLE(YourClass, (myfield1 [, "key"]), ...)` in the global namespace
   - The list of tuples after "YourClass" in the JSON_ENABLE declaration will be the fields that are serialized and deserialized.
   - The first part of the tuple is the field name 
   - The second part is an optional key to use for the JSON. If left out, the stringified field name will be used as the key.

To convert a class to/from json:
- `#include "json_functions.hpp"`
- Call `JSON::ToJSON<YourClass>(classInstance)`
- Or `JSON::FromJSON<YourClass>()`


--------
For example (using default keys):

MySimpleClass.hpp

    /* MySimpleClass.hpp */
    class MySimpleClass {
    public:
        MySimpleClass() : m_int(10) {};
        
        int m_int;
    };
    
MySimpleClass.cpp

    /* MySimpleClass.cpp */
    #include "json.hpp"
    JSON_ENABLE(MySimpleClass, (m_int))
    
To convert the class to a JSON string:

    #include "json_functions.hpp"
    MySimpleClass simple;
    std::string json = JSON::ToJSON<MySimpleClass>(simple);
    /* json == {"m_int":10} */
    
To create an instance of the class from JSON:
    
    #include "json_functions.hpp"
    MySimpleClass simple = JSON::FromJSON<MySimpleClass>(json);
    
---
Json conversion is fully-recursive for any type that is JSON_ENABLED. This includes all stl types and any user-defined classes which are JSON-Enabled using this code.

e.g.

    /* NestedContainer.hpp */
    class NestedContainer {
    public:
        NestedContainer() : m_simpleClass(), m_int(20) {};
        MySimpleClass m_simpleClass;
        int m_int;
    };
---
    /* NestedContainer.cpp */
    #include "json.hpp"
    JSON_ENABLE(NestedContainer, (m_simpleClass), (m_int, "not_nested_int"))
    
Calling ToJSON would produce:

    {"m_simpleClass":{"m_int":10},"not_nested_int":20}

--- 
#### Warning:
There is no cycle detection logic. If deserializing encounters a cycle your program will most likely crash.

---
### Constant fields:
Specifying a const field to be included in the JSON is acceptable; However, a warning will be issued informing you that you have a const field specified to be written into when de-serializing. The warning is purely informational. No data will be written to the const field.

It is possible to disable this warning by defining JSON_NO_WARN_CONST

---
### Dependencies
This program requires the boost.preprocessor and boost.lexical_cast libraries.

---
### Tested compilers:
- VS2013.4
- Cygwin g++ 4.8.3


