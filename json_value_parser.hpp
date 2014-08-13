#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <string>
#include <stdexcept>
#include <type_traits>
/* I want to include as few headers as possible, but tuples suck to set up properly */
#include <tuple>

namespace std {
    template<typename T, size_t N> struct array;
    template<typename T, typename A> class deque;
    template<typename T, typename A> class forward_list;
    template<typename T, typename A> class list;
    template<typename T, typename A> class vector;

    template<typename T1, typename T2> struct pair;

    template<typename K, typename C, typename A> class set;
    template<typename K, typename C, typename A> class multiset;
    template<typename K, typename H, typename KE, typename A> class unordered_set;

    template<typename K, typename T, typename C, typename A> class map;

    template<typename T, typename D> class unique_ptr;
    template<typename T> class shared_ptr;
    template<typename T> class weak_ptr;
    template<typename T> class auto_ptr;
}

namespace JSON {

#define JSON_ITRABLE_PARSER(STL_TYPE, ...)                                                              \
    struct JSONFnInvokerImpl<const std::STL_TYPE<__VA_ARGS__>> {                                        \
        json_finline inline static std::wstring ToJSON(const std::STL_TYPE<__VA_ARGS__>* classFrom) {   \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::ToJSON(classFrom);                       \
        }                                                                                               \
    }                                                                                                   \

#define JSON_SMRTPTR_PARSER(STL_TYPE, PTR_TYPE)                                                     \
    struct JSONFnInvokerImpl<const std::STL_TYPE<PTR_TYPE>> {                                       \
        json_finline inline static std::wstring ToJSON(const std::STL_TYPE<PTR_TYPE>* classFrom) {  \
            if(!*classFrom) {                                                                       \
                return L"null";                                                                     \
            }                                                                                       \
            return JSONFnInvoker<PTR_TYPE>::ToJSON((*classFrom).get());                             \
        }                                                                                           \
    }                                                                                               \

////////////////////////////////////////////////////////////////////////////////
// PODParser implementation
// Transforms basic data types
////
    template<typename Type>
    struct PODParser {
        json_finline inline static Type FromJSON(const std::wstring& json) {
            std::wstringstream wss;
            wss << json;

            Type value;
            wss >> value;

            if(wss.fail()) {
                throw std::invalid_argument("bad json value for key");
            }

            return value;
        }

        json_finline inline static std::wstring ToJSON(const Type* value) {
            // return std::to_wstring(value);
            //Cygwin won't let me use the above...
            std::wstringstream wss;
            wss << *value;
            return wss.str();
        }
    };

template<typename T> struct JSONFnInvoker;

////////////////////////////////////////////////////////////////////////////////
// IterableParser implementation
// Transforms an iterable type into an array of its internal values
////
    template<typename Type>
    struct IterableParser {
        json_finline inline static Type FromJSON(const std::wstring& json) {
            //TODO
            Type value;
            return value;
        }

        json_finline inline static std::wstring ToJSON(const Type* value) {
            std::wstring result(L"[");

            if(!value->empty()) {
                auto itr = std::begin(*value);
                auto endItr = std::prev(std::end(*value));
                typedef decltype(*itr) valtype;

                for(; itr != endItr; ++itr) {
                    result += JSONFnInvoker<valtype>::ToJSON(*itr);
                    result += L",";
                }
                result += JSONFnInvoker<valtype>::ToJSON(*itr);
            }

            result += L"]";
            return result;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONFnInvoker implementation
////
    template<typename T> struct JSONFnInvokerImpl;
    template<typename T, size_t N = 1> struct JSONArrayHandler;
    template<typename T, bool AM, bool IP, bool AR> struct JSONFnInvokerDecider;

    template<typename ClassOn>
    struct JSONFnInvoker {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONFnInvokerDecider<const ClassOn,
                                        std::is_arithmetic<ClassOn>::value,
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value
                                       >::ToJSON(classFrom);
        }
    };

    template<typename ClassOn>
    struct JSONFnInvoker<ClassOn&> {
        json_finline inline static std::wstring ToJSON(const ClassOn& classFrom) {
            return JSONFnInvokerDecider<const ClassOn,
                                        std::is_arithmetic<ClassOn>::value,
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value
                                       >::ToJSON(&classFrom);
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONFnInvokerImpl<const ClassOn>::ToJSON(classFrom);
        }
    };

    template<typename ClassOn, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, /* Arithmetic */ true, IsPtr, Array> {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return boost::lexical_cast<std::wstring>(*classFrom);
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<const char, /* Arithmetic */ true, IsPtr, Array> {
        json_finline inline static std::wstring ToJSON(const char* classFrom) {
            std::wstring json(L"\"");
            json += boost::lexical_cast<std::wstring>(*classFrom);
            json += L"\"";
            return json;
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<const wchar_t, /* Arithmetic */ true, IsPtr, Array> {
        json_finline inline static std::wstring ToJSON(const wchar_t* classFrom) {
            std::wstring json(L"\"");
            json += boost::lexical_cast<std::wstring>(*classFrom);
            json += L"\"";
            return json;
        }
    };

    template<typename ClassOn, bool Arithmetic, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, /* IsPtr */ true, Array> {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            if(*classFrom == nullptr) {
                return L"null";
            }

            return JSONFnInvoker<const ClassOn>::ToJSON(*classFrom);
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, /* Array */ true> {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONArrayHandler<const ClassOn>::ToJSON(classFrom);
        }
    };

    template<typename ClassOn>
    struct JSONFnInvokerImpl {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return classFrom->ToJSON();
        }
    };

    /* stl flat iterable types */
    template<typename T, std::size_t A>
    JSON_ITRABLE_PARSER(array, T, A);

    template<typename T, typename A>
    JSON_ITRABLE_PARSER(deque, T, A);

    template<typename T, typename A>
    JSON_ITRABLE_PARSER(forward_list, T, A);

    template<typename T, typename A>
    JSON_ITRABLE_PARSER(list, T, A);

    template<typename T, typename A>
    JSON_ITRABLE_PARSER(vector, T, A);

    template<typename K, typename C, typename A>
    JSON_ITRABLE_PARSER(set, K, C, A);

    template<typename K, typename C, typename A>
    JSON_ITRABLE_PARSER(multiset, K, C, A);

    template<typename K, typename H, typename KE, typename A>
    JSON_ITRABLE_PARSER(unordered_set, K, H, KE, A);

    /* stl map types */
    template<typename K, typename T, typename C, typename A>
    JSON_ITRABLE_PARSER(map, K, T, C, A);

    template<typename T1, typename T2>
    struct JSONFnInvokerImpl<const std::pair<T1, T2>> {
        json_finline inline static std::wstring ToJSON(const std::pair<T1, T2>* classFrom) {
            std::wstring json(L"[");
            json += JSONFnInvoker<T1>::ToJSON(&classFrom->first);
            json += L",";
            json += JSONFnInvoker<T2>::ToJSON(&classFrom->second);
            json += L"]";

            return json;
        }
    };

    //WE DON'T OWN THESE SMART PTRS!
    template<typename T, typename D>
    struct  JSONFnInvokerImpl<const std::unique_ptr<T, D>> {
        json_finline inline static std::wstring ToJSON(const std::unique_ptr<T, D>* classFrom) {
            if(!*classFrom) {
                return L"null";
            }
            return JSONFnInvoker<T>::ToJSON((*classFrom).get());
        }
    };

    //WE DON'T OWN THESE SMART PTRS!
    template<typename T>
    JSON_SMRTPTR_PARSER(shared_ptr, T);

    //WE DON'T OWN THESE SMART PTRS!
    template<typename T>
    JSON_SMRTPTR_PARSER(weak_ptr, T);

    //WE DON'T OWN THESE SMART PTRS!
    template<typename T>
    JSON_SMRTPTR_PARSER(auto_ptr, T);

    /* ugh */
    template<typename TupleType,
             size_t curIndex,
             bool lastValue,
             typename curType,
             typename... Types>
    struct TupleHandler {
        json_finline inline static void ToJSON(const TupleType* classFrom,
                                  std::wstring& jsonString) {
            jsonString += JSONFnInvoker<curType>::ToJSON(&std::get<curIndex>(*classFrom));
            jsonString += L",";
            TupleHandler<TupleType,
                         curIndex + 1,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, jsonString);
        }
    };

    template<typename TupleType,
             size_t curIndex,
             typename curType,
             typename... Types>
    struct TupleHandler <TupleType,
                         curIndex,
                         true,
                         curType,
                         Types...> {
        json_finline inline static void ToJSON(const TupleType* classFrom,
                                  std::wstring& jsonString) {
            jsonString += JSONFnInvoker<curType>::ToJSON(&std::get<curIndex>(*classFrom));
        }
    };

    template<typename... Types>
    struct JSONFnInvokerImpl<const std::tuple<Types...>> {
        json_finline inline static std::wstring ToJSON(const std::tuple<Types...>* classFrom) {
            std::wstring json(L"[");
            TupleHandler<std::tuple<Types...>,
                         0,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, json);
            json += L"]";
            return json;
        }
    };

/////////////////////////////////////////
// Compile-time sized array handler
    template<typename ClassOn,
             size_t rank>
    struct JSONArrayHandler {
        json_finline inline static std::wstring ToJSON(const ClassOn classFrom) {
            if(std::extent<ClassOn>::value == 0) {
                return L"[]";
            }

            typedef typename std::remove_extent<ClassOn>::type valueType;

            std::wstring json(L"[");
            json += JSONArrayHandler<valueType, std::rank<valueType>::value>::ToJSON(classFrom[0]);

            for(size_t i = 1; i < std::extent<ClassOn>::value; ++i) {
                json += L",";
                json += JSONArrayHandler<valueType, std::rank<valueType>::value>::ToJSON(classFrom[i]);
            }
            json += L"]";
            return json;
        }
    };

    /* Not an array */
    template<typename ClassOn>
    struct JSONArrayHandler<ClassOn, 0> {
        json_finline inline static std::wstring ToJSON(const ClassOn classFrom) {
            return JSONFnInvoker<ClassOn>::ToJSON(&classFrom);
        }
    };
}
#endif
