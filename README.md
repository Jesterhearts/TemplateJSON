T-JSON
============

This is for making c++ class able to go to and from JSON.

To create a class:

Instead of class MyClass {...}; you instead do JSON_CLASS(MyClass, ...);

Every variable that you want to make into JSON just has to be declared with
Note that jsonkey should NOT be a string, it should just be plain text. It will be turned into a string for you.
type_qualifiers JSON_VAR(type, varname, jsonkey, [= some_value])

E.g.
const JSON_VAR(int, myfoo, foo, = 0);

Or
JSON_VAR(int, myfoo, foo)

Once a class has been created this way, extracting the JSON is done with

MyClass.ToJson();

Restrictions:
You cannot have duplicate json keys.
There is no cycle detection. Use JSON_VAR(myobject, myval, mykey) at your risk.
This has only been tested on g++ version 4.8.3 (it should work on any compiler that supports constexpr and the __COUNTER__ macro but I make no guarantees).

Creating a class from JSON is not supported yet.
Only POD types and user-defined JSON classes are supported for JSONifying currently.
