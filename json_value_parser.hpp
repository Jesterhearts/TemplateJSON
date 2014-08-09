#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <sstream>
#include <string>
#include <stdexcept>
#include <algorithm>

namespace std {
    template<class T, std::size_t A> class array;
    template<class T, class A> class deque;
    template<class T, class A> class forward_list;
    template<class T, class A> class list;
    template<class T, class A> class queue;
    template<class T, class A> class set;
    template<class T, class A> class stack;
    template<class T, class A> class vector;
}

namespace JSON {

////////////////////////////////////////////////////////////////////////////////
// PODParser implementation
// Transforms basic data types
////
    template<typename Type>
    struct PODParser {
        static Type FromJSON(const std::wstring& json) {
            std::wstringstream wss;
            wss << json;

            Type value;
            wss >> value;

            if(wss.fail()) {
                throw std::invalid_argument("bad json value for key");
            }

            return value;
        }

        static std::wstring ToJSON(const Type& value) {
            // return std::to_wstring(value);
            //Cygwin won't let me use the above...
            std::wstringstream wss;
            wss << value;
            return wss.str();
        }
    };

template<typename T> struct TypetoJSONFnInvokerHelper;

////////////////////////////////////////////////////////////////////////////////
// FlatIterableParser implementation
// Transforms stl types like vector, set, etc. that have an array-like structure and support 
// std iterators
////
    template<typename Type>
    struct FlatIterableParser {
        static Type FromJSON(const std::wstring& json) {
            //TODO
            Type value;
            return value;
        }

        static std::wstring ToJSON(const Type& value) {
            std::wstring result(L"[");

            auto itr = std::begin(value);
            auto endItr = std::prev(std::end(value));
            typedef decltype(*itr) valtype;

            for(; itr != endItr; ++itr) {
                result += TypetoJSONFnInvokerHelper<valtype>::ToJSON(*itr);
                result += L",";
            }

            if(!value.empty()) {
                result += TypetoJSONFnInvokerHelper<valtype>::ToJSON(value.back());
            }

            result += L"]";
            return result;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// TypetoJSONFnInvoker implementation
////
    template<typename classOn>
    struct TypetoJSONFnInvoker {
        static std::wstring ToJSON(const classOn& classFrom) {
            return classFrom.ToJSON();
        }
    };

    /* basic data types */
    template<>
    struct TypetoJSONFnInvoker<const char&> {
        static std::wstring ToJSON(const char& classFrom) {
            std::wstring result(L"\"");
            result += PODParser<char>::ToJSON(classFrom);
            result += L"\"";
            return result;
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const wchar_t&> {
        static std::wstring ToJSON(const wchar_t& classFrom) {
            std::wstring result(L"\"");
            result += classFrom;
            result += L"\"";
            return result;
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const bool&> {
        static std::wstring ToJSON(const bool& classFrom) {
            return classFrom ? L"true" : L"false";
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const int&> {
        static std::wstring ToJSON(const int& classFrom) {
            return PODParser<int>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const long&> {
        static std::wstring ToJSON(const long& classFrom) {
            return PODParser<long>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const long long&> {
        static std::wstring ToJSON(const long long& classFrom) {
            return PODParser<long long>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const unsigned int&> {
        static std::wstring ToJSON(const unsigned int& classFrom) {
            return PODParser<unsigned int>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const unsigned long&> {
        static std::wstring ToJSON(const unsigned long& classFrom) {
            return PODParser<unsigned long>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const unsigned long long&> {
        static std::wstring ToJSON(const unsigned long long& classFrom) {
            return PODParser<unsigned long long>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const float&> {
        static std::wstring ToJSON(const float& classFrom) {
            return PODParser<float>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const double&> {
        static std::wstring ToJSON(const double& classFrom) {
            return PODParser<double>::ToJSON(classFrom);
        }
    };

    template<>
    struct TypetoJSONFnInvoker<const long double&> {
        static std::wstring ToJSON(const long double& classFrom) {
            return PODParser<long double> ::ToJSON(classFrom);
        }
    };

    /* stl array types */
    template<typename T>
    struct TypetoJSONFnInvoker<const std::vector<T>&> {
        static std::wstring ToJSON(const std::vector<T>& classFrom) {
            return FlatIterableParser<std::vector<T>>::ToJSON(classFrom);
        }
    };

    // template<typename T>
    // struct TypetoJSONFnInvoker<const std::list<T>&> {
    //     static std::wstring ToJSON(const std::vector<T>& classFrom) {
    //         return FlatIterableParser<std::vector<T>>::ToJSON(classFrom);
    //     }
    // };

    // template<typename T>
    // struct TypetoJSONFnInvoker<const std::deque<T>&> {
    //     static std::wstring ToJSON(const std::vector<T>& classFrom) {
    //         return FlatIterableParser<std::vector<T>>::ToJSON(classFrom);
    //     }
    // };

    // template<typename T>
    // struct TypetoJSONFnInvoker<const std::forward_list<T>&> {
    //     static std::wstring ToJSON(const std::vector<T>& classFrom) {
    //         return FlatIterableParser<std::vector<T>>::ToJSON(classFrom);
    //     }
    // };

    // template<typename T>
    // struct TypetoJSONFnInvoker<const std::array<T>&> {
    //     static std::wstring ToJSON(const std::vector<T>& classFrom) {
    //         return FlatIterableParser<std::vector<T>>::ToJSON(classFrom);
    //     }
    // };


    /* */
    template<typename classOn>
    struct TypetoJSONFnInvokerHelper {
        static std::wstring ToJSON(const classOn& classFrom) {
            return TypetoJSONFnInvoker<const classOn&>::ToJSON(classFrom);
        }
    };
}
#endif
