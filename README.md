T-JSON
============

This is a simple way to make a C++ class serialize to/deserialize from JSON.

---
To create a class:

- &#35;include "json.hpp"
- Inherit from JSON::JSONBase&lt;YourClass&gt;
- Use the macro JSON_ENABLE(YourClass, (myfield1 [, "key"]), ...) in the global namespace
- The list of tuples after "YourClass" in the JSON_ENABLE declaration will be the fields that are serialized and deserialized.
 - The first part of the tuple is the field name 
 - The second part is an optional key to use for the JSON. If left out, the stringified field name will be used as the key.

--------
For example (using the default key):

    class MySimpleClass : public JSON::JSONBase<MySimpleClass> {
    public:
        MySimpleClass() : m_int(10) {};
        
        int m_int;
    };
    
    JSON_ENABLE(MySimpleClass, (m_int))
    
Then to make the class into JSON:

    MySimpleClass simple;
    std::string json = simple.ToJSON();
    /* json == {"m_int":10} */
    
To create an instance of the class from JSON:
    
    MySimpleClass simple = MySimpleClass::FromJSON(json_string);
    
---
Json conversion is fully-recursive for any type that is JSON_ENABLED. This includes all stl types and any user-defined classes which are JSON-Enabled using this code.

e.g.

    class NestedContainer : public JSON::JSONBase<NestedContainer> {
    public:
        NestedContainer() : m_simpleClass(), m_int(20) {};
        MySimpleClass m_simpleClass;
        int m_int;
    };
    
    JSON_ENABLE(NestedContainer, (m_simpleClass), (m_int, "not_nested_int"))
    
Calling ToJSON would produce:

    {"m_simpleClass":{"m_int":10},"not_nested_int":20}

--- 
#### Warning:

There is no cycle detection logic. If deserializing encounters a cycle your program will most likely crash (or enter an infinite loop).

---
### Constant fields:
Specifying a const field to be included in the JSON is acceptable; However, a warning will be issued informing you that you have a const field specified to be written into when de-serializing. The warning is purely informational. No data will be written to the const field.

It is possible to disable this warning by defining JSON_NO_WARN_CONST

---
### Wide string support
By default json strings are output as std::string. This can be changed by defining JSON_USE_WIDE_STRINGS

No changes need to be made to JSON_ENABLE declarations when switching between types. Only consumers and suppliers of json strings will need to be updated to handle the change.

---
### Tested compilers:
- VS2010
- VS2012
- VS2013
- Cygwin g++ 4.8.3


