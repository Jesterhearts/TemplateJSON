T-JSON
============

This is a simple way to make a C++ class serialize to/deserialize from JSON.

---
To create a class:

- In your class header file:
 1. If you specify any constructors for your class you must also provide a construct for the class that accepts as its arguments values for all of the fields that you want to be initialized when deserializing.
 2. If you wish to specify private data members for serializing/deserializing
   - `#include "json_private_access.hpp`
   - In your class body invoke `JSON_PRIVATE_ACCESS()`
      - This can go anywhere in the declaration
      - It only declares as friends the types needed by the json serializer to handle serializing your class. It will not affect visibility modifiers.

- In your class implementation file:
 1. `#include "json.hpp"`
 2. Use the macro `JSON_ENABLE(YourClass, (myfield1 [, "key"]), ...)` in the global namespace
   - The list of tuples after "YourClass" in the JSON_ENABLE declaration will be the fields that are serialized and deserialized.
   - The first part of the tuple is the field name 
   - The second part is an optional key to use for the JSON. If left out, the stringified field name will be used as the key.

To convert a class to/from json:
- `#include "json_functions.hpp"`
- `tjson::to_json(YourClass)`
- `tjson::from_json<YourClass>(string_for_your_class)`


--------
For example (using default keys):

MySimpleClass.hpp

    /* MySimpleClass.hpp */
    class MySimpleClass {
    public:
        MySimpleClass() : m_int(10) {}
        MySimpleClass(int i) : m_int(i) {};
        
        int m_int;
    };
    
MySimpleClass.cpp

    /* MySimpleClass.cpp */
    #include "json.hpp"
    JSON_ENABLE(MySimpleClass, (m_int))
    
To convert the class to a JSON string:

    #include "json_functions.hpp"
    MySimpleClass simple;
    std::string json = tjson::to_json(simple);
    /* json == {"m_int":10} */
    
To create an instance of the class from JSON:
    
    #include "json_functions.hpp"
    MySimpleClass simple = tjson::from_json<MySimpleClass>(json);
    
---
Json conversion is fully-recursive for any type that is JSON_ENABLED. This includes all stl types and any user-defined classes which are JSON-Enabled using this code.

e.g.

    /* NestedContainer.hpp */
    class NestedContainer {
    public:
        NestedContainer() : m_simpleClass(), m_int(20) {};
        NestedContainer(MySimpleClass&& c, int i) : m_simpleClass(c), m_int(i) {};
        MySimpleClass m_simpleClass;
        int m_int;
    };
---
    /* NestedContainer.cpp */
    #include "json.hpp"
    JSON_ENABLE(NestedContainer, (m_simpleClass), (m_int, "not_nested_int"))
    
Calling to_json would produce:

    {"m_simpleClass":{"m_int":10},"not_nested_int":20}

--- 
#### Note:
There is no cycle detection logic. If deserializing encounters a cycle your program will crash.

---
### Constant fields:
Since the deserialization will internally access the constructor for your class, constant fields can be specified. They will be initialized during construction.

---
### Static fields:
Static members are currently unsupported.

---
### Arrays:
Supported:
- std::array

Unsupported:
- Constant sized arrays (e.g. const char* [20])
- Heap allocated arrays (e.g. int* = new int[20])

---
### Dependencies
This program requires the boost.preprocessor and boost.lexical_cast libraries.

---
### Tested compilers:
- Cygwin64 g++ (GCC) 4.9.2
- MinGW-W64 g++ (x86_64-posix-seh-rev1, Built by MinGW-W64 project) 4.9.2
- MSVC 2015

