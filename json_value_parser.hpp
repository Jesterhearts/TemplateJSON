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
#ifdef _MSC_VER
    template<typename T, size_t N> class array;
#else
    template<typename T, size_t N> struct array;
#endif
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
    struct JSONFnInvokerImpl<std::STL_TYPE<__VA_ARGS__>> {                                              \
        json_finline static std::wstring ToJSON(const std::STL_TYPE<__VA_ARGS__>* classFrom) {          \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::ToJSON(classFrom);                       \
        }                                                                                               \
                                                                                                        \
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,                              \
                                                     std::STL_TYPE<__VA_ARGS__>& into) {                \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::FromJSON(iter, end, into);               \
        }                                                                                               \
    }                                                                                                   \

#define JSON_SMRTPTR_PARSER(STL_TYPE, PTR_TYPE)                                                     \
    struct JSONFnInvokerImpl<std::STL_TYPE<PTR_TYPE>> {                                             \
        json_finline static std::wstring ToJSON(const std::STL_TYPE<PTR_TYPE>* classFrom) {         \
            if(!*classFrom) {                                                                       \
                return nullToken;                                                                   \
            }                                                                                       \
            return JSONFnInvoker<PTR_TYPE>::ToJSON((*classFrom).get());                             \
        }                                                                                           \
                                                                                                    \
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,                          \
                                                     std::STL_TYPE<PTR_TYPE>& into) {               \
            PTR_TYPE* temp;                                                                         \
            iter = JSONFnInvoker<PTR_TYPE*>::FromJSON(iter, end, temp);                             \
            into.reset(temp);                                                                       \
            return iter;                                                                            \
        }                                                                                           \
    }                                                                                               \

namespace JSON {
    namespace {
        const std::wstring nullToken(L"null");
        typedef std::wstring::const_iterator jsonIter;

        json_no_return void ThrowBadJSONError(jsonIter iter, jsonIter end,
                                              const std::string&& errmsg) {
            std::string badJson(iter, end);
            throw std::invalid_argument(errmsg + ": " + badJson);
        }
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
            case L'.':
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
    template<typename Type, typename VType>
    struct IterableInserter {
        json_finline static void Insert(Type& type, VType& input) {
            type.insert(input);
        }
    };

    template<typename VType, typename A>
    struct IterableInserter<std::vector<VType, A>, VType> {
        json_finline static void Insert(std::vector<VType, A>& type, VType& input) {
            type.push_back(input);
        }
    };

    template<typename Type>
    struct IterableParser {
        json_finline static std::wstring ToJSON(const Type* value) {
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

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, Type& into) {
            if(end - iter < 2) {
                ThrowBadJSONError(iter, end, "No array tokens");
            }

            iter = AdvancePastWhitespace(iter, end);
            if(*iter != L'[') {
                ThrowBadJSONError(iter, end, "No array start token");
            }
            ++iter;

            while(iter != end && *iter != L']') {
                typename Type::value_type input;
                //Each call advances iter past the end of the token read by the call
                iter = JSONFnInvoker<typename Type::value_type>::FromJSON(iter, end, input);
                IterableInserter<Type, typename Type::value_type>::Insert(into, input);
                iter = AdvancePastWhitespace(iter, end);

                if(iter != end && *iter == L',') {
                    ++iter;
                }
            }

            if(iter == end) {
                ThrowBadJSONError(iter, end, "No end to JSON array");
            }

            ++iter;
            return iter;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONFnInvoker implementation
////
    template<typename T, size_t N = 1> struct JSONArrayHandler;
    template<typename T, bool AM, bool IP, bool AR, bool C = false> struct JSONFnInvokerDecider;

    template<typename ClassOn>
    struct JSONFnInvoker {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            typedef typename std::remove_cv<ClassOn>::type type;
            return JSONFnInvokerDecider<type,
                                        std::is_arithmetic<ClassOn>::value,
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value
                                       >::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into)
        try {
            typedef typename std::remove_cv<ClassOn>::type type;
            return JSONFnInvokerDecider<type,
                                        std::is_arithmetic<ClassOn>::value,
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value,
                                        std::is_const<ClassOn>::value
                                       >::FromJSON(iter, end, into);
        }
        catch(boost::bad_lexical_cast blc) {
            std::string eMsg("Could not convert to type ");
            ThrowBadJSONError(iter, end, std::move(eMsg) + static_cast<const char*>(typeid(into).name()));
        }
    };

    template<typename ClassOn>
    struct JSONFnInvoker<ClassOn&> {
        json_finline static std::wstring ToJSON(const ClassOn& classFrom) {
            typedef typename std::remove_cv<ClassOn>::type type;
            return JSONFnInvokerDecider<type,
                                        std::is_arithmetic<ClassOn>::value,
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value
                                       >::ToJSON(&classFrom);
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr, bool Array, bool Const>
    struct JSONFnInvokerDecider {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONFnInvokerImpl<ClassOn>::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONFnInvokerImpl<ClassOn>::FromJSON(iter, end, into);
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, Array, /* Const */ true> {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, Array, false>::ToJSON(classFrom);
        }

        json_deserialize_const_warning
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, const ClassOn& into) {
            //TODO: allow advancing without generating data
            ClassOn shadow;
            return JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, Array, false>::FromJSON(iter, end, shadow);
        }
    };

    template<typename ClassOn, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            return boost::lexical_cast<std::wstring>(*classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);
            auto endOfNumber = AdvancePastNumbers(iter, end);

            std::wstring number(iter, endOfNumber);
            into = boost::lexical_cast<ClassOn>(number);

            return endOfNumber;
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<char, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
        json_finline static std::wstring ToJSON(const char* classFrom) {
            std::wstring json(L"\"");
            json += boost::lexical_cast<std::wstring>(*classFrom);
            json += L"\"";
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, char& into) {
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L'\"') {
                ThrowBadJSONError(iter, end, "Not a valid string begin token");
            }

            ++iter;
            if(iter == end) {
                ThrowBadJSONError(iter, end, "No data for string");
            }

            if(*iter == L'\\') {
                ++iter;
            }

            //Todo, does this handle unicode escape sequences?
            auto endOfString = iter;
            ++endOfString;
            if(endOfString == end || *endOfString != L'\"') {
                ThrowBadJSONError(iter, end,"No string end token");
            }

            //get the character
            into = std::string(iter, endOfString)[0];

            //Advance past the end
            ++endOfString;
            return endOfString;
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<wchar_t, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
        json_finline static std::wstring ToJSON(const wchar_t* classFrom) {
            std::wstring json(L"\"");
            json += boost::lexical_cast<std::wstring>(*classFrom);
            json += L"\"";
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, wchar_t& into) {
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L'\"') {
                ThrowBadJSONError(iter, end, "Not a valid string begin token");
            }

            ++iter;
            if(iter == end) {
                ThrowBadJSONError(iter, end, "No data for string");
            }

            if(*iter == L'\\') {
                ++iter;
            }

            //Todo, does this handle unicode escape sequences?
            auto endOfString = iter;
            ++endOfString;
            if(endOfString == end || *endOfString != L'\"') {
                ThrowBadJSONError(iter, end,"No string end token");
            }

            //Advance past the end
            ++endOfString;
            //get the character
            into = *iter;
            return endOfString;
        }
    };

    template<typename ClassOn, bool Arithmetic, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, /* IsPtr */ true, Array, /* Const */ false> {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            if(*classFrom == nullptr) {
                return L"null";
            }
            //classFrom is pointer to pointer
            typedef decltype(**classFrom) nextype;
            return JSONFnInvoker<nextype>::ToJSON(**classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);

            if(iter != end &&
               static_cast<size_t>(end - iter) > nullToken.length()) {
                std::wstring info(iter, iter + nullToken.length());
                if(nullToken == info) {
                    into = nullptr;
                    return iter + nullToken.length();
                }
            }

            typedef typename std::remove_pointer<ClassOn>::type InternalClass;
            InternalClass temp;
            iter = JSONFnInvoker<InternalClass>::FromJSON(iter, end, temp);
            into = new InternalClass(temp);
            return iter;
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, /* Array */ true, /* Const */ false> {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONArrayHandler<ClassOn>::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONArrayHandler<ClassOn>::FromJSON(iter, end, into);
        }
    };

    template<>
    struct JSONFnInvokerImpl<std::string> {
        json_finline static std::wstring ToJSON(const std::string* classFrom) {
            std::wstring json(L"\"");
            std::wstring wideString(classFrom->begin(), classFrom->end());
            json += wideString;
            json += L"\"";
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::string& classInto) {
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L'\"') {
                ThrowBadJSONError(iter, end, "Not a valid string begin token");
            }

            ++iter;
            auto endOfString = AdvanceToEndOfString(iter, end);
            if(endOfString == end) {
                ThrowBadJSONError(iter, end, "Not a valid string end token");
            }

            classInto = std::string(iter, endOfString);
            ++endOfString;
            return endOfString;
        }
    };

    template<>
    struct JSONFnInvokerImpl<std::wstring> {
        json_finline static std::wstring ToJSON(const std::wstring* classFrom) {
            std::wstring json(L"\"");
            json += *classFrom;
            json += L"\"";
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::wstring& classInto) {
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L'\"') {
                ThrowBadJSONError(iter, end, "Not a valid string begin token");
            }

            ++iter;
            auto endOfString = AdvanceToEndOfString(iter, end);
            if(endOfString == end) {
                ThrowBadJSONError(iter, end, "Not a valid string end token");
            }

            classInto = std::wstring(iter, endOfString);
            ++endOfString;
            return endOfString;
        }
    };

    template<typename ClassOn>
    struct JSONFnInvokerImpl {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            return classFrom->ToJSON();
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
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
    struct JSONFnInvokerImpl<std::pair<T1, T2>> {
        json_finline static std::wstring ToJSON(const std::pair<T1, T2>* classFrom) {
            std::wstring json(L"[");
            json += JSONFnInvoker<T1>::ToJSON(&classFrom->first);
            json += L",";
            json += JSONFnInvoker<T2>::ToJSON(&classFrom->second);
            json += L"]";

            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::pair<T1, T2>& into) {
            if(end - iter < 2) {
                ThrowBadJSONError(iter, end, "No array tokens");
            }

            iter = AdvancePastWhitespace(iter, end);
            if(*iter != L'[') {
                ThrowBadJSONError(iter, end, "No array start token");
            }
            ++iter;

            typedef typename std::remove_cv<T1>::type FirstType;
            typedef typename std::remove_cv<T2>::type SecondType;

            FirstType& first = const_cast<FirstType&>(into.first);
            SecondType& second = const_cast<SecondType&>(into.second);

            //Each call advances iter past the end of the token read by the call
            iter = JSONFnInvoker<FirstType>::FromJSON(iter, end, first);

            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L',') {
                ThrowBadJSONError(iter, end, "Pair does not have two values");
            }
            ++iter;

            iter = JSONFnInvoker<SecondType>::FromJSON(iter, end, second);

            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L']') {
                ThrowBadJSONError(iter, end, "No end to JSON pair");
            }

            ++iter;
            return iter;
        }
    };

    //WE DON'T OWN THESE SMART PTRS!
    template<typename T, typename D>
    struct  JSONFnInvokerImpl<std::unique_ptr<T, D>> {
        json_finline static std::wstring ToJSON(const std::unique_ptr<T, D>* classFrom) {
            if(!*classFrom) {
                return L"null";
            }
            return JSONFnInvoker<T>::ToJSON((*classFrom).get());
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::unique_ptr<T, D>& into) {
            T* temp;
            iter = JSONFnInvoker<T*>::FromJSON(iter, end, temp);
            into.reset(temp);
            return iter;
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
        json_finline static void ToJSON(const TupleType* classFrom,
                                               std::wstring& jsonString) {
            jsonString += JSONFnInvoker<curType>::ToJSON(&std::get<curIndex>(*classFrom));
            jsonString += L",";
            TupleHandler<TupleType,
                         curIndex + 1,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, jsonString);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     TupleType& into) {
            curType& value = std::get<curIndex>(into);

            iter = JSONFnInvoker<curType>::FromJSON(iter, end, value);
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L',') {
                ThrowBadJSONError(iter, end, "Not a valid tuple value");
            }
            ++iter;
            return TupleHandler<TupleType,
                                curIndex + 1,
                                sizeof...(Types) == 1,
                                Types...
                               >::FromJSON(iter, end, into);
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
        json_finline static void ToJSON(const TupleType* classFrom,
                                               std::wstring& jsonString) {
            jsonString += JSONFnInvoker<curType>::ToJSON(&std::get<curIndex>(*classFrom));
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     TupleType& into) {
            curType& value = std::get<curIndex>(into);
            iter = JSONFnInvoker<curType>::FromJSON(iter, end, value);
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L']') {
                ThrowBadJSONError(iter, end, "No tuple end token");
            }
            ++iter;
            return iter;
        }
    };

    template<typename... Types>
    struct JSONFnInvokerImpl<std::tuple<Types...>> {
        json_finline static std::wstring ToJSON(const std::tuple<Types...>* classFrom) {
            std::wstring json(L"[");
            TupleHandler<std::tuple<Types...>,
                         0,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, json);
            json += L"]";
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::tuple<Types...>& into) {
            if(iter == end || *iter != L'[') {
                ThrowBadJSONError(iter, end, "No tuple start token");
            }
            ++iter;

            return TupleHandler<std::tuple<Types...>,
                                0,
                                sizeof...(Types) == 1,
                                Types...
                               >::FromJSON(iter, end, into);
        }
    };

/////////////////////////////////////////
// Compile-time sized array handler
    template<typename ClassOn,
             size_t rank>
    struct JSONArrayHandler {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
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

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            if(end - iter < 2) {
                ThrowBadJSONError(iter, end, "No array tokens");
            }

            iter = AdvancePastWhitespace(iter, end);
            if(*iter != L'[') {
                ThrowBadJSONError(iter, end, "No array start token");
            }
            ++iter;

            typedef typename std::remove_extent<ClassOn>::type valueType;
            for(size_t i = 0; i < std::extent<ClassOn>::value; ++i) {
                iter = JSONArrayHandler<valueType, std::rank<valueType>::value>::FromJSON(iter, end, into[i]);
                iter = AdvancePastWhitespace(iter, end);

                if(iter == end) {
                    ThrowBadJSONError(iter, end, "Not enough items in JSON array");
                }

                if(*iter != L',' && i < std::extent<ClassOn>::value - 1) {
                    ThrowBadJSONError(iter, end, "Missing comma in JSON array");
                }
                else if(*iter == L',') {
                    ++iter;
                }
            }

            if(iter == end || *iter != L']') {
                ThrowBadJSONError(iter, end, "No end or too many items in JSON array");
            }

            ++iter;
            return iter;
        }
    };

    /* Not an array */
    template<typename ClassOn>
    struct JSONArrayHandler<ClassOn, 0> {
        json_finline static std::wstring ToJSON(const ClassOn* classFrom) {
            return JSONFnInvoker<ClassOn>::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONFnInvoker<ClassOn>::FromJSON(iter, end, into);
        }
    };

    jsonIter ParseNextKey(jsonIter iter, jsonIter end, std::wstring& nextKey) {
        return JSONFnInvokerImpl<std::wstring>::FromJSON(iter, end, nextKey);
    }
}
#endif
