T-JSON
============

This is for making c++ class able to go to and from JSON.

To create a class:

Inherit from JSON::JSONBase<>
e.g. MyClass : public JSON::JSONBase<MyClass> {...};

Once a class has been created, you need to enable JSON for it with
JSON_ENABLE(MyClass, .... list of class variables to enable JSON for);

Extracting the JSON is done with

MyClass.ToJson();

Restrictions:
There is no cycle detection. Use JSON_VAR(myobject, myval, mykey) at your risk.
This has only been tested on g++ version 4.8.3 (it should work on any compiler that supports constexpr and the __COUNTER__ macro but I make no guarantees).

Creating a class from JSON is not supported yet.
