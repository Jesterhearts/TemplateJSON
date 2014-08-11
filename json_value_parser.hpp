#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

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
}

namespace JSON {

#define JSON_NUMTYPE_PARSER(PODTYPE)                                                \
    template<>                                                                      \
    struct TypeJSONFnInvokerImpl<const PODTYPE&> {                                  \
        inline static std::wstring ToJSON(const PODTYPE& classFrom) json_finline {  \
            return PODParser<PODTYPE>::ToJSON(classFrom);                           \
        }                                                                           \
    }

#define JSON_ITRABLE_PARSER(STL_TYPE, ...)                                                              \
    struct TypeJSONFnInvokerImpl<const std::STL_TYPE<__VA_ARGS__>&> {                                   \
        inline static std::wstring ToJSON(const std::STL_TYPE<__VA_ARGS__>& classFrom) json_finline {   \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::ToJSON(classFrom);                       \
        }                                                                                               \
    }

////////////////////////////////////////////////////////////////////////////////
// PODParser implementation
// Transforms basic data types
////
    template<typename Type>
    struct PODParser {
        inline static Type FromJSON(const std::wstring& json) json_finline {
            std::wstringstream wss;
            wss << json;

            Type value;
            wss >> value;

            if(wss.fail()) {
                throw std::invalid_argument("bad json value for key");
            }

            return value;
        }

        inline static std::wstring ToJSON(const Type& value) json_finline {
            // return std::to_wstring(value);
            //Cygwin won't let me use the above...
            std::wstringstream wss;
            wss << value;
            return wss.str();
        }
    };

template<typename T> struct TypeJSONFnInvoker;

////////////////////////////////////////////////////////////////////////////////
// IterableParser implementation
// Transforms an iterable type into an array of its internal values
////
    template<typename Type>
    struct IterableParser {
        inline static Type FromJSON(const std::wstring& json) json_finline {
            //TODO
            Type value;
            return value;
        }

        inline static std::wstring ToJSON(const Type& value) json_finline {
            std::wstring result(L"[");

            if(!value.empty()) {
                auto itr = std::begin(value);
                auto endItr = std::prev(std::end(value));
                typedef decltype(*itr) valtype;

                for(; itr != endItr; ++itr) {
                    result += TypeJSONFnInvoker<valtype>::ToJSON(*itr);
                    result += L",";
                }
                result += TypeJSONFnInvoker<valtype>::ToJSON(*itr);
            }

            result += L"]";
            return result;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// TypeJSONFnInvoker implementation
////
    template<typename T> struct TypeJSONFnInvokerImpl;
    template<typename T, size_t N> struct JSONArrayHandler;

    template<typename classOn>
    struct TypeJSONFnInvoker {
        inline static std::wstring ToJSON(const classOn& classFrom) json_finline {
            return TypeJSONFnInvokerImpl<const classOn&>::ToJSON(classFrom);
        }
    };

    template<typename classOn>
    struct TypeJSONFnInvoker<classOn*> {
        inline static std::wstring ToJSON(const classOn* classFrom) json_finline {
            if(classFrom == nullptr) {
                return L"null";
            }
            return TypeJSONFnInvoker<const classOn&>::ToJSON(*classFrom);
        }
    };

    template<typename classOn>
    struct TypeJSONFnInvokerImpl {
        inline static std::wstring ToJSON(const classOn& classFrom) json_finline {
            return classFrom.ToJSON();
        }
    };

    /* pretty easy */
    template<>
    struct TypeJSONFnInvokerImpl<std::wstring> {
        inline static std::wstring ToJSON(const std::wstring& classFrom) json_finline {
            return L"\"" + classFrom + L"\"";
        }
    };

    /* basic data types with special handling */
    template<>
    struct TypeJSONFnInvokerImpl<const char&> {
        inline static std::wstring ToJSON(const char& classFrom) json_finline {
            std::wstring result(L"\"");
            result += PODParser<char>::ToJSON(classFrom);
            result += L"\"";
            return result;
        }
    };

    template<>
    struct TypeJSONFnInvokerImpl<const wchar_t&> {
        inline static std::wstring ToJSON(const wchar_t& classFrom) json_finline {
            std::wstring result(L"\"");
            result += classFrom;
            result += L"\"";
            return result;
        }
    };

    template<>
    struct TypeJSONFnInvokerImpl<const bool&> {
        inline static std::wstring ToJSON(const bool& classFrom) json_finline {
            return classFrom ? L"true" : L"false";
        }
    };

    /* basic number types */
    //TODO can this be cleaner with std::is_fundamental?
    JSON_NUMTYPE_PARSER(int);
    JSON_NUMTYPE_PARSER(long);
    JSON_NUMTYPE_PARSER(long long);
    JSON_NUMTYPE_PARSER(unsigned int);
    JSON_NUMTYPE_PARSER(unsigned long);
    JSON_NUMTYPE_PARSER(unsigned long long);
    JSON_NUMTYPE_PARSER(float);
    JSON_NUMTYPE_PARSER(double);
    JSON_NUMTYPE_PARSER(long double);

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
    struct TypeJSONFnInvokerImpl<const std::pair<T1, T2>&> {
        inline static std::wstring ToJSON(const std::pair<T1, T2>& classFrom) json_finline {
            std::wstring json(L"[");
            json += TypeJSONFnInvoker<T1>::ToJSON(classFrom.first);
            json += L",";
            json += TypeJSONFnInvoker<T1>::ToJSON(classFrom.second);
            json += L"]";

            return json;
        }
    };

    /* ugh */
    template<typename TupleType,
             size_t curIndex,
             bool lastValue,
             typename curType,
             typename... Types>
    struct TupleHandler {
        inline static void ToJSON(const TupleType& classFrom,
                                  std::wstring& jsonString) json_finline {
            jsonString += TypeJSONFnInvoker<curType>::ToJSON(std::get<curIndex>(classFrom));
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
        inline static void ToJSON(const TupleType& classFrom,
                                  std::wstring& jsonString) json_finline {
            jsonString += TypeJSONFnInvoker<curType>::ToJSON(std::get<curIndex>(classFrom));
        }
    };

    template<typename... Types>
    struct TypeJSONFnInvokerImpl<const std::tuple<Types...>&> {
        inline static std::wstring ToJSON(const std::tuple<Types...>& classFrom) json_finline {
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
    template<typename classOn,
             size_t rank = 1>
    struct JSONArrayHandler {
        inline static std::wstring ToJSON(const classOn classFrom) json_finline {
            if(std::extent<classOn>::value == 0) {
                return L"[]";
            }

            typedef typename std::remove_extent<classOn>::type valueType;

            std::wstring json(L"[");
            json += JSONArrayHandler<valueType, std::rank<valueType>::value>::ToJSON(classFrom[0]);

            for(size_t i = 1; i < std::extent<classOn>::value; ++i) {
                json += L",";
                json += JSONArrayHandler<valueType, std::rank<valueType>::value>::ToJSON(classFrom[i]);
            }
            json += L"]";
            return json;
        }
    };

    /* Not an array */
    template<typename classOn>
    struct JSONArrayHandler<classOn, 0> {
        inline static std::wstring ToJSON(const classOn classFrom) json_finline {
            return TypeJSONFnInvoker<classOn>::ToJSON(classFrom);
        }
    };
}
#endif
