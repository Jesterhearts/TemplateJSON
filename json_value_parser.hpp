#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <string>
#include <type_traits>

#include "json_type_info.hpp"

namespace JSON { namespace detail {
    template<typename ClassType, typename tag>
    json_finline std::string ToJSON(const ClassType& from, tag&& a);

    template<typename ClassType>
    json_finline std::string ToJSON(const ClassType& from) {
        return ToJSON(from, typename TypeInfo<ClassType>::type());
    }
}}

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

#include "json_parsing_helpers.hpp"
#include "json_iterable_parser.hpp"
#include "json_array_parser.hpp"

#define JSON_SMRTPTR_PARSER(STL_TYPE, PTR_TYPE)                                         \
    template<typename PTR_TYPE>                                                                \
    struct JSONFnInvokerImpl<std::STL_TYPE<PTR_TYPE>> {                                 \
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,              \
                                                     std::STL_TYPE<PTR_TYPE>& into) {   \
            PTR_TYPE* temp;                                                             \
            iter = JSONFnInvoker<PTR_TYPE*>::FromJSON(iter, end, temp);                 \
            into.reset(temp);                                                           \
            return iter;                                                                \
        }                                                                               \
    };                                                                                  \

namespace JSON {
    const std::string nullToken("null");

    namespace detail {

////////////////////////////////////////////////////////////////////////////////////////////////////
        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _const&& a) {
            return ToJSON(from, typename TypeInfo<typename std::remove_const<ClassType>::type>::type());
        }

        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _enum&& a) {
            //TODO fixme
            return boost::lexical_cast<std::string>(from);
        }

        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _arithmetic&& a) {
            return boost::lexical_cast<std::string>(from);
        }

        template<>
        json_finline std::string ToJSON(const char& from, _arithmetic&& a) {
            std::string json("\"");
            json.append(1, from);
            json += "\"";
            return json;
        }

        template<>
        json_finline std::string ToJSON(const wchar_t& from, _arithmetic&& a) {
            std::string json("\"");

            std::wstring wideChar(1, from);
            std::string narrowChar(wideChar.begin(), wideChar.end());

            json.append(narrowChar);
            json.append("\"");
            return json;
        }

        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _pointer&& a) {
            if(from == nullptr) {
                return nullToken;
            }
            return ToJSON(*from, typename TypeInfo<decltype(*from)>::type());
        }

        template<typename T, typename... D,
                 template<typename T, typename... D> class SmartPointerType>
        json_finline std::string SmartPointerToJSON(const SmartPointerType<T, D...>& from) {
            if(!from) {
                return nullToken;
            }

            return detail::ToJSON(*from.get());
        }
    }

    template<>
    json_finline std::string ToJSON(const std::string& from) {
        std::string json("\"");
        json.append(from);
        json.append("\"");
        return json;
    }

    template<>
    json_finline std::string ToJSON(const std::wstring& from) {
        std::string json("\"");
        std::string narrowString(from.begin(), from.end());

        json.append(narrowString);
        json.append("\"");
        return json;
    }

    template<typename T1, typename T2>
    json_finline std::string ToJSON(const std::pair<T1, T2>& from) {
        std::string json("[");
        json.append(detail::ToJSON(from.first));
        json.append(",");
        json.append(detail::ToJSON(from.second));
        json.append("]");

        return json;
    }

    template<typename T>
    json_finline std::string ToJSON(const std::shared_ptr<T>& from) {
        return detail::SmartPointerToJSON(from);
    }

    template<typename T>
    json_finline std::string ToJSON(const std::weak_ptr<T>& from) {
        return detail::SmartPointerToJSON(from);
    }

    template<typename T>
    json_finline std::string ToJSON(const std::auto_ptr<T>& from) {
        return detail::SmartPointerToJSON(from);
    }

    template<typename T, typename D>
    json_finline std::string ToJSON(const std::unique_ptr<T, D>& from) {
        return detail::SmartPointerToJSON(from);
    }
}

#include "json_tuple_parser.hpp"
namespace JSON {
    template<typename... Types>
    json_finline static std::string ToJSON(const std::tuple<Types...>& from) {
        std::string json("[");
        TupleHandler<std::tuple<Types...>,
                     0,
                     sizeof...(Types) == 1,
                     Types...
                    >::ToJSON(from, json);
        json.append("]");
        return json;
    }

    namespace detail {
        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _class&& a) {
            JSON::ToJSON(from);
        }
    }

////////////////////////////////////////////////////////////////////////////////
// JSONFnInvoker implementation
////
    template<typename ClassOn>
    struct JSONFnInvoker {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            typedef typename std::remove_cv<ClassOn>::type type;
            return JSONFnInvokerDecider<type,
                                        std::is_arithmetic<ClassOn>::value, 
                                        std::is_pointer<ClassOn>::value,
                                        std::is_array<ClassOn>::value,
                                        std::is_const<ClassOn>::value
                                       >::FromJSON(iter, end, into);
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONFnInvokerDecider implementation
////
    /* Any non-const object */
    template<typename ClassOn, bool Arithmetic, bool IsPtr, bool Array, bool Const>
    struct JSONFnInvokerDecider {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONFnInvokerImpl<ClassOn>::FromJSON(iter, end, into);
        }
    };

    /* Any const object.
     * Deserialize is okay.
     * Serialize generates a warning if JSON_NO_WARN_CONST is not defined
     */
    template<typename ClassOn, bool Arithmetic, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, Array, /* Const */ true> {
        json_deserialize_const_warning
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, const ClassOn& into) {
            //TODO: allow advancing without generating data
            ClassOn shadow;
            return JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, Array, false>::FromJSON(iter, end, shadow);
        }
    };

    /* Any number type, not a char or a wchar_t (since those get handled as strings) */
    template<typename ClassOn, bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<ClassOn, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);
            auto endOfNumber = AdvancePastNumbers(iter, end);

            try{
                into = boost::lexical_cast<ClassOn>(&*iter, std::distance(iter, endOfNumber));
            }
            catch(boost::bad_lexical_cast blc) {
                ThrowBadJSONError(iter, end, std::string("Could not convert to type ") + typeid(into).name());
            }

            return endOfNumber;
        }
    };

    /* char and wchar_t are handled specially, since they are "strings" in json */
    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<char, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
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
            into = *iter;

            //Advance past the end
            ++endOfString;
            return endOfString;
        }
    };

    template<bool IsPtr, bool Array>
    struct JSONFnInvokerDecider<wchar_t, /* Arithmetic */ true, IsPtr, Array, /* Const */ false> {
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
            into = std::wstring(iter, endOfString)[0];

            //Advance past the end
            ++endOfString;
            return endOfString;
        }
    };

    /* Handles all pointer types. */
    template<typename ClassOn, bool Arithmetic, bool Array>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, /* IsPtr */ true, Array, /* Const */ false> {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            iter = AdvancePastWhitespace(iter, end);

            if(iter != end &&
               static_cast<size_t>(end - iter) > nullToken.length()) {
                std::string info(iter, iter + nullToken.length());
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

    /* Handles compile-time sized arrays */
    template<typename ClassOn, bool Arithmetic, bool IsPtr>
    struct JSONFnInvokerDecider<ClassOn, Arithmetic, IsPtr, /* Array */ true, /* Const */ false> {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return JSONArrayHandler<ClassOn>::FromJSON(iter, end, into);
        }
    };

    /* This handles the special needs of the std::string and std::wstring classes */
    template<>
    struct JSONFnInvokerImpl<std::string> {
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

    /* This handles all user defined objects */
    template<typename ClassOn>
    struct JSONFnInvokerImpl {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& classInto) {
            return JSON::FromJSON<ClassOn>(iter, end, classInto);
        }
    };

    /* stl flat iterable type handlers (non k-v pair types) */
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

    /* This special handler is for std::pair */
    template<typename T1, typename T2>
    struct JSONFnInvokerImpl<std::pair<T1, T2>> {
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

    /* stl smart pointer handlers */
    template<typename T, typename D>
    struct JSONFnInvokerImpl<std::unique_ptr<T, D>> {
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::unique_ptr<T, D>& into) {
            T* temp;
            iter = JSONFnInvoker<T*>::FromJSON(iter, end, temp);
            into.reset(temp);
            return iter;
        }
    };

    JSON_SMRTPTR_PARSER(shared_ptr, T);

    JSON_SMRTPTR_PARSER(weak_ptr, T);

    JSON_SMRTPTR_PARSER(auto_ptr, T);

    /* This extracts the next key from the map when deserializing.
     *
     * g++ refused to let me use forward decls for this, which is why it's allll the way down here
     */
    json_finline jsonIter ParseNextKey(jsonIter iter, jsonIter end, std::string& nextKey) {
        return JSONFnInvokerImpl<std::string>::FromJSON(iter, end, nextKey);
    }
}
#endif
