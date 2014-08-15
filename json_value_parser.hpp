#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <string>
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

#define JSON_ITRABLE_PARSER(STL_TYPE, ...)                                                              \
    struct JSONFnInvokerImpl<const std::STL_TYPE<__VA_ARGS__>> {                                        \
        json_finline inline static std::wstring ToJSON(const std::STL_TYPE<__VA_ARGS__>* classFrom) {   \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::ToJSON(classFrom);                       \
        }                                                                                               \
                                                                                                        \
        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,                       \
                                                     std::STL_TYPE<__VA_ARGS__>& into) {                \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::FromJSON(iter, end, into);               \
        }                                                                                               \
    }                                                                                                   \

#define JSON_SMRTPTR_PARSER(STL_TYPE, PTR_TYPE)                                                     \
    struct JSONFnInvokerImpl<const std::STL_TYPE<PTR_TYPE>> {                                       \
        json_finline inline static std::wstring ToJSON(const std::STL_TYPE<PTR_TYPE>* classFrom) {  \
            if(!*classFrom) {                                                                       \
                return nullToken;                                                                   \
            }                                                                                       \
            return JSONFnInvoker<PTR_TYPE>::ToJSON((*classFrom).get());                             \
        }                                                                                           \
                                                                                                    \
        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,                   \
                                                     std::STL_TYPE<PTR_TYPE>& into) {               \
            iter = AdvancePastWhitespace(iter, end);                                                \
            auto endOfNulltoken = iter;                                                             \
            std::advance(endOfNulltoken, nullToken.length() + 1);                                   \
                                                                                                    \
            std::wstring token(iter, endOfNulltoken);                                               \
            if(token == nullToken) {                                                                \
                /* iterator has to be AFTER the end of the token */                                 \
                iter = endOfNulltoken;                                                              \
                into.reset(nullptr);                                                                \
            }                                                                                       \
                                                                                                    \
            into.reset(new PTR_TYPE);                                                               \
            return JSONFnInvoker<PTR_TYPE>::FromJSON(iter, end, *into);                             \
        }                                                                                           \
    }                                                                                               \

namespace JSON {
    namespace {
        const std::wstring nullToken(L"null");
        typedef std::wstring::const_iterator jsonIter;
    }

template<typename T> struct JSONFnInvoker;
template<typename T> struct JSONFnInvokerImpl;

    jsonIter AdvancePastWhitespace(jsonIter iter, jsonIter end) {
        while(iter != end) {
            switch(*iter) {
            case L' ':
            case L'\t':
            case L'\n':
            case L'\r':
                break;

            default:
                return iter;
            }

            ++iter;
        }

        return iter;
    }

    jsonIter AdvancePastNumbers(jsonIter iter, jsonIter end) {
        if(iter != end &&
           *iter == L'-') {
            ++iter;
        }

        while(iter != end) {
            switch(*iter) {
            case L'0':
            case L'1':
            case L'2':
            case L'3':
            case L'4':
            case L'5':
            case L'6':
            case L'7':
            case L'8':
            case L'9':
                break;

            default:
                return iter;
            }

            ++iter;
        }

        return iter;
    }

    jsonIter AdvanceToEndOfString(jsonIter iter, jsonIter end) {
        bool escaping = true;

        while(iter != end) {
            if(*iter == L'\\' || escaping) {
                escaping = !escaping;
            }
            else if(!escaping && *iter == L'\"') {
                return iter;
            }

            ++iter;
        }

        return iter;
    }

////////////////////////////////////////////////////////////////////////////////
// IterableParser implementation
// Transforms an iterable type into an array of its internal values
////
    template<typename Type>
    struct IterableParser {
        json_finline inline static std::wstring ToJSON(const Type* value) {
            std::wstring result(L"[");

            if(!value->empty()) {
                auto iter = std::begin(*value);
                auto endItr = std::prev(std::end(*value));
                typedef decltype(*iter) valtype;

                for(; iter != endItr; ++iter) {
                    result += JSONFnInvoker<valtype>::ToJSON(*iter);
                    result += L",";
                }
                result += JSONFnInvoker<valtype>::ToJSON(*iter);
            }

            result += L"]";
            return result;
        }

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end, Type& into) {
            if(end - iter < 2) {
                throw std::invalid_argument("No array tokens");
            }

            iter = AdvancePastWhitespace(iter, end);
            if(*iter != L'[') {
                throw std::invalid_argument("No array start token");
            }
            ++iter;

            while(iter != end && *iter != L']') {
                if(*iter == L',') {
                    ++iter;
                }

                typename Type::value_type input;
                //Each call advances iter past the end of the token read by the call
                iter = JSONFnInvoker<typename Type::value_type>(iter, end, input);
                into.push_back(input);
                iter = AdvancePastWhitespace(iter, end);
            }

            if(iter == end) {
                throw std::invalid_argument("No end to JSON array");
            }

            ++iter;
            return iter;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONFnInvoker implementation
////
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

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONFnInvokerDecider<ClassOn,
                                        std::is_arithmetic<ClassOn>::value,
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value
                                       >::FromJSON(iter, end, into);
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

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONFnInvokerImpl<ClassOn>::FromJSON(iter, end, into);
        }
    };

    template<typename ClassOn, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, /* Arithmetic */ true, IsPtr, Array> {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return boost::lexical_cast<std::wstring>(*classFrom);
        }

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);
            auto endOfNumber = AdvancePastNumbers(iter, end);

            std::wstring number(iter, endOfNumber);
            into = boost::lexical_cast<ClassOn>(number);

            return endOfNumber;
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
            //classFrom is pointer to pointer
            typedef decltype(**classFrom) nextype;
            return JSONFnInvoker<const nextype>::ToJSON(**classFrom);
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, /* Array */ true> {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONArrayHandler<const ClassOn>::ToJSON(classFrom);
        }
    };

    template<>
    struct JSONFnInvokerImpl<const std::string> {
        json_finline inline static std::wstring ToJSON(const std::string* classFrom) {
            std::wstring json(L"\"");
            std::wstring wideString(classFrom->begin(), classFrom->end());
            json += wideString;
            json += L"\"";
            return json;
        }

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::string& classInto) {
        }
    };

    template<>
    struct JSONFnInvokerImpl<const std::wstring> {
        json_finline inline static std::wstring ToJSON(const std::wstring* classFrom) {
            std::wstring json(L"\"");
            json += *classFrom;
            json += L"\"";
            return json;
        }

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::wstring& classInto) {
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L'\"') {
                throw std::invalid_argument("Not a valid string begin token");
            }

            ++iter;
            auto endOfString = AdvanceToEndOfString(iter, end);
            if(endOfString == end) {
                throw std::invalid_argument("Not a valid string end token");
            }

            classInto = std::wstring(iter, endOfString);
            ++endOfString;
            return endOfString;
        }
    };

    template<typename ClassOn>
    struct JSONFnInvokerImpl {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return classFrom->ToJSON();
        }

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     ClassOn& classInto) {
            return ClassOn::FromJSON(iter, end, classInto);
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

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::pair<T1, T2>& classInto) {
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

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::unique_ptr<T, D>& classInto) {
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

        json_finline inline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::tuple<Types...>& classInto) {
        }
    };

/////////////////////////////////////////
// Compile-time sized array handler
    template<typename ClassOn,
             size_t rank>
    struct JSONArrayHandler {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            if(std::extent<ClassOn>::value == 0) {
                return L"[]";
            }

            typedef typename std::remove_extent<ClassOn>::type valueType;

            std::wstring json(L"[");
            json += JSONArrayHandler<valueType, std::rank<valueType>::value>::ToJSON(&(*classFrom)[0]);

            for(size_t i = 1; i < std::extent<ClassOn>::value; ++i) {
                json += L",";
                json += JSONArrayHandler<valueType, std::rank<valueType>::value>::ToJSON(&(*classFrom)[i]);
            }
            json += L"]";
            return json;
        }
    };

    /* Not an array */
    template<typename ClassOn>
    struct JSONArrayHandler<ClassOn, 0> {
        json_finline inline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONFnInvoker<ClassOn>::ToJSON(classFrom);
        }
    };

    jsonIter ParseNextKey(jsonIter iter, jsonIter end, std::wstring& nextKey) {
        return JSONFnInvokerImpl<const std::wstring>::FromJSON(iter, end, nextKey);
    }
}
#endif
