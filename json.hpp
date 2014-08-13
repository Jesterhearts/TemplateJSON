#pragma once
#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/expand.hpp>

#include <boost/preprocessor/punctuation/comma.hpp>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>

#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/wstringize.hpp>

#include "json_common_macros.hpp"
#include "json_value_parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#define JSON_PRIVATE_ACCESS()   \
    template<typename __JSON_FRIEND_TYPE__> friend JSONEnabler;

#define JSON_MAKE_JSONENABLE_BODY(s, IGNORED, VARNAME)                      \
        jsonData += L"\"" BOOST_PP_WSTRINGIZE(VARNAME) L"\":";              \
        typedef decltype(classFor->VARNAME) BOOST_PP_CAT(__type, VARNAME);  \
        jsonData += JSON::JSONFnInvoker<BOOST_PP_CAT(__type, VARNAME)>      \
                        ::ToJSON(&classFor->VARNAME);                       \

#define JSON_ENABLE(CLASS_NAME, ...)                                      \
namespace JSON {                                                          \
    template<>                                                            \
    struct JSONEnabler<CLASS_NAME> {                                      \
        json_finline static void ToJSON(const CLASS_NAME* classFor,       \
                                        std::wstring& jsonData) {         \
            BOOST_PP_SEQ_FOR_EACH(JSON_MAKE_JSONENABLE_BODY,              \
                                  _,                                      \
                                  BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
                                 )                                        \
        }                                                                 \
    };                                                                    \
}                                                                         \

////////////////////////////////////////////////////////////////////////////////////////////////////
//  These functions and definitions generate the to/from JSON functions for the class at compile
//  time
////
namespace JSON {
    typedef std::unordered_map<std::wstring, std::wstring> DataMap;
    typedef std::pair<std::wstring, std::wstring> DataType;

    template<typename ClassFor>
    struct JSONEnabler {
        static std::wstring ToJSON(const ClassFor*);
    };

    template<typename ClassFor>
    class JSONBase {
    public:
        std::wstring ToJSON() const {
            std::wstring json(L"{");
            JSONEnabler<ClassFor>::ToJSON(static_cast<const ClassFor*>(this), json);
            json += L"}";
            return json;
        }
    };
}
#endif
