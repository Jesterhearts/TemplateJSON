#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <string>
#include <type_traits>

#include "json_parsing_helpers.hpp"
#include "json_iterable_parser.hpp"
#include "json_tuple_parser.hpp"
#include "json_array_parser.hpp"

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

#define JSON_SMRTPTR_PARSER(STL_TYPE, PTR_TYPE)                                                     \
    struct JSONFnInvokerImpl<std::STL_TYPE<PTR_TYPE>> {                                             \
        json_finline static stringt ToJSON(const std::STL_TYPE<PTR_TYPE>* classFrom) {              \
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
    const stringt nullToken(JSON_ST("null"));

////////////////////////////////////////////////////////////////////////////////
// JSONFnInvoker implementation
////
    template<typename ClassOn>
    struct JSONFnInvoker {
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
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
        json_finline static stringt ToJSON(const ClassOn& classFrom) {
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
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
            return JSONFnInvokerImpl<ClassOn>::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONFnInvokerImpl<ClassOn>::FromJSON(iter, end, into);
        }
    };

    template<typename ClassOn, bool Arithmetic, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, Array, /* Const */ true> {
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
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
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
            return boost::lexical_cast<stringt>(*classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);
            auto endOfNumber = AdvancePastNumbers(iter, end);

            stringt number(iter, endOfNumber);
            into = boost::lexical_cast<ClassOn>(number);

            return endOfNumber;
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<char, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
        json_finline static stringt ToJSON(const char* classFrom) {
            stringt json(JSON_ST("\""));

#ifdef JSON_USE_WIDE_STRINGS
            json += boost::lexical_cast<std::wstring>(*classFrom);
#else
            json += *classFrom;
#endif

            json += JSON_ST("\"");
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
#ifdef JSON_USE_WIDE_STRINGS
            into = std::string(iter, endOfString)[0];
#else
            into = *iter;
#endif
            //Advance past the end
            ++endOfString;
            return endOfString;
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<wchar_t, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
        json_finline static stringt ToJSON(const wchar_t* classFrom) {
            stringt json(JSON_ST("\""));
#ifdef JSON_USE_WIDE_STRINGS
            json += *classFrom;
#else
            std::wstring wideChar(*classFrom);
            std::string narrowChar(wideChar.begin(), wideChar.end());
            json += narrowChar;
#endif
            json += JSON_ST("\"");
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

            //get the character
#ifdef JSON_USE_WIDE_STRINGS
            into = std::wstring(iter, endOfString)[0];
#else
            into = *iter;
#endif
            //Advance past the end
            ++endOfString;
            return endOfString;
        }
    };

    template<typename ClassOn, bool Arithmetic, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, /* IsPtr */ true, Array, /* Const */ false> {
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
            if(*classFrom == nullptr) {
                return JSON_ST("null");
            }
            //classFrom is pointer to pointer
            typedef decltype(**classFrom) nextype;
            return JSONFnInvoker<nextype>::ToJSON(**classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);

            if(iter != end &&
               static_cast<size_t>(end - iter) > nullToken.length()) {
                stringt info(iter, iter + nullToken.length());
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
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
            return JSONArrayHandler<ClassOn>::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONArrayHandler<ClassOn>::FromJSON(iter, end, into);
        }
    };

    template<>
    struct JSONFnInvokerImpl<std::string> {
        json_finline static stringt ToJSON(const std::string* classFrom) {
            stringt json(JSON_ST("\""));
#ifdef JSON_USE_WIDE_STRINGS
            std::wstring wideString(classFrom->begin(), classFrom->end());
            json += wideString;
#else
            json += *classFrom;
#endif
            json += JSON_ST("\"");
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
        json_finline static stringt ToJSON(const std::wstring* classFrom) {
            stringt json(JSON_ST("\""));
#ifdef JSON_USE_WIDE_STRINGS
            json += *classFrom;
#else
            std::string narrowString(classFrom->begin(), classFrom->end());
            json += narrowString;
#endif
            json += JSON_ST("\"");
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
        json_finline static stringt ToJSON(const ClassOn* classFrom) {
            return classFrom->ToJSON();
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     ClassOn& classInto) {
            return ClassOn::FromJSON(iter, end, classInto);
        }
    };

    /* stl flat iterable types */
    template<typename T, std::size_t A>
    JSON_ITERABLE_PARSER(array, T, A);

    template<typename T, typename A>
    JSON_ITERABLE_PARSER(deque, T, A);

    template<typename T, typename A>
    JSON_ITERABLE_PARSER(forward_list, T, A);

    template<typename T, typename A>
    JSON_ITERABLE_PARSER(list, T, A);

    template<typename T, typename A>
    JSON_ITERABLE_PARSER(vector, T, A);

    template<typename K, typename C, typename A>
    JSON_ITERABLE_PARSER(set, K, C, A);

    template<typename K, typename C, typename A>
    JSON_ITERABLE_PARSER(multiset, K, C, A);

    template<typename K, typename H, typename KE, typename A>
    JSON_ITERABLE_PARSER(unordered_set, K, H, KE, A);

    /* stl map types */
    template<typename K, typename T, typename C, typename A>
    JSON_ITERABLE_PARSER(map, K, T, C, A);

    template<typename T1, typename T2>
    struct JSONFnInvokerImpl<std::pair<T1, T2>> {
        json_finline static stringt ToJSON(const std::pair<T1, T2>* classFrom) {
            stringt json(JSON_ST("["));
            json += JSONFnInvoker<T1>::ToJSON(&classFrom->first);
            json += JSON_ST(",");
            json += JSONFnInvoker<T2>::ToJSON(&classFrom->second);
            json += JSON_ST("]");

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
        json_finline static stringt ToJSON(const std::unique_ptr<T, D>* classFrom) {
            if(!*classFrom) {
                return JSON_ST("null");
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

    jsonIter ParseNextKey(jsonIter iter, jsonIter end, stringt& nextKey) {
        return JSONFnInvokerImpl<stringt>::FromJSON(iter, end, nextKey);
    }
}
#endif
