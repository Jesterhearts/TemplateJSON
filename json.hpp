#pragma once
#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/expand.hpp>

#include <boost/preprocessor/punctuation/comma.hpp>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/wstringize.hpp>

#include "json_common_macros.hpp"
#include "json_value_parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#define JSON_PRIVATE_ACCESS()   \
    template<typename __JSON_FRIEND_TYPE__> friend JSONEnabler;

////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_ENABLE(CLASS_NAME, ...)                                              \
    namespace JSON {                                                              \
        template<>                                                                \
        struct JSONEnabler<CLASS_NAME> {                                          \
            json_finline inline static void ToJSON(const CLASS_NAME* classFor,    \
                                                   std::wstring& jsonData) {      \
                JSON_START_JSONENABLE_BODY(                                       \
                    BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)                        \
                                          )                                       \
                                                                                  \
                BOOST_PP_SEQ_FOR_EACH(                                            \
                    JSON_MAKE_JSONENABLE_BODY, _,                                 \
                        BOOST_PP_SEQ_POP_FRONT(                                   \
                            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                 \
                                               )                                  \
                                    )                                             \
            }                                                                     \
        };                                                                        \
    }

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_START_JSONENABLE_BODY(VARDATA)                   \
    BOOST_PP_EXPAND(JSON_START_JSONENABLE_BODY_IMPL VARDATA)


#define JSON_START_JSONENABLE_BODY_IMPL(...)                                      \
    BOOST_PP_OVERLOAD(JSON_START_JSONENABLE_BODY_IMPL, __VA_ARGS__)(__VA_ARGS__)


#define JSON_START_JSONENABLE_BODY_IMPL1(VARNAME)       \
    JSON_START_JSONENABLE_BODY_IMPL2(VARNAME, BOOST_PP_WSTRINGIZE(VARNAME))


#define JSON_START_JSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)              \
    jsonData += L"\"" JSONKEY L"\":";              \
    typedef decltype(classFor->VARNAME) BOOST_PP_CAT(__type, VARNAME);  \
    jsonData += JSON::JSONFnInvoker<BOOST_PP_CAT(__type, VARNAME)>      \
    ::ToJSON(&classFor->VARNAME);

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_MAKE_JSONENABLE_BODY(s, IGNORED, VARDATA)      \
    BOOST_PP_EXPAND(JSON_MAKE_JSONENABLE_BODY_IMPL VARDATA)


#define JSON_MAKE_JSONENABLE_BODY_IMPL(...)                                      \
    BOOST_PP_OVERLOAD(JSON_MAKE_JSONENABLE_BODY_IMPL, __VA_ARGS__)(__VA_ARGS__)


#define JSON_MAKE_JSONENABLE_BODY_IMPL1(VARNAME)        \
    JSON_MAKE_JSONENABLE_BODY_IMPL2(VARNAME, VARNAME)


#define JSON_MAKE_JSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)               \
    jsonData += L",\"" BOOST_PP_WSTRINGIZE(JSONKEY) L"\":";             \
    typedef decltype(classFor->VARNAME) BOOST_PP_CAT(__type, VARNAME);  \
    jsonData += JSON::JSONFnInvoker<BOOST_PP_CAT(__type, VARNAME)>      \
                    ::ToJSON(&classFor->VARNAME);

////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_INHERITS(...)                                  \
    const std::wstring ToJSON() {                           \
        std::wstring jsonData(L"[");                        \
        JSON_START_JSONINHERITS_BODY(                       \
            BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)          \
                                  )                         \
        BOOST_PP_SEQ_FOR_EACH(                              \
            JSON_MAKE_JSONINHERITS_BODY, _,                 \
                BOOST_PP_SEQ_POP_FRONT(                     \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
                                       )                    \
                            )                               \
        jsonData += L"]";                                   \
        return jsonData;                                    \
     }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_START_JSONINHERITS_BODY(CLASSNAME)                 \
    jsonData += L"{\"" BOOST_PP_WSTRINGIZE(CLASSNAME) L"\":";   \
    jsonData += JSON::JSONBase<CLASSNAME>::ToJSON();            \
    jsonData += L"}";


#define JSON_MAKE_JSONINHERITS_BODY(s, IGNORED, CLASSNAME)      \
    jsonData += L",{\"" BOOST_PP_WSTRINGIZE(CLASSNAME) L"\":";  \
    jsonData += CLASSNAME::ToJSON();                            \
    jsonData += L"}";



namespace JSON {
    typedef std::unordered_map<std::wstring, std::wstring> DataMap;
    typedef std::pair<std::wstring, std::wstring> DataType;

    template<typename ClassFor>
    struct JSONEnabler {
        json_finline inline static std::wstring ToJSON(const ClassFor*);
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
