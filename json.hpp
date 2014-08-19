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
    template<typename __JSON_FRIEND_TYPE__> friend struct JSON::JSONEnabler;


////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                              \
    namespace JSON {                                                              \
        template<>                                                                \
        struct JSONEnabler<CLASS_NAME> {                                          \
            json_finline static void ToJSON(const CLASS_NAME* classFor,           \
                                                   std::wstring& jsonData) {      \
                JSON_START_TOJSONENABLE_BODY(                                     \
                    BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)                        \
                                          )                                       \
                                                                                  \
                BOOST_PP_SEQ_FOR_EACH(                                            \
                    JSON_MAKE_TOJSONENABLE_BODY, _,                               \
                        BOOST_PP_SEQ_POP_FRONT(                                   \
                            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                 \
                                               )                                  \
                                    )                                             \
            }                                                                     \
                                                                                  \
            json_finline static CLASS_NAME FromJSON(const std::wstring& jsonData) {  \
                CLASS_NAME classInto;                                             \
                                                                                  \
                auto iter = jsonData.begin();                                     \
                auto end = jsonData.end();                                        \
                                                                                  \
                FromJSON(iter, end, classInto);                                   \
                return classInto;                                                 \
            }                                                                     \
                                                                                  \
            json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,    \
                                                        CLASS_NAME& classInto) {  \
                                                                                  \
                iter = AdvancePastWhitespace(iter, end);                          \
                if(iter == end || *iter != L'{') {                                \
                    throw std::invalid_argument("No object start token");         \
                }                                                                 \
                ++iter;                                                           \
                                                                                  \
                std::wstring nextKey;                                             \
                                                                                  \
                DataMap memberMap;                                                \
                DataMap::const_iterator insertAt;                                 \
                                                                                  \
                BOOST_PP_SEQ_FOR_EACH(                                            \
                    JSON_COLLECT_FROMJSONENABLE_DATA, _,                          \
                            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                 \
                                    )                                             \
                                                                                  \
                BOOST_PP_SEQ_FOR_EACH(                                            \
                    JSON_MAKE_FROMJSONENABLE_BODY, _,                             \
                            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                 \
                                    )                                             \
                                                                                  \
                iter = AdvancePastWhitespace(iter, end);                          \
                if(iter == end || *iter != L'}') {                                \
                    throw std::invalid_argument("No object end token");           \
                }                                                                 \
                ++iter;                                                           \
                return iter;                                                      \
            }                                                                     \
        };                                                                        \
    }


////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_START_TOJSONENABLE_BODY(VARDATA)                   \
    BOOST_PP_EXPAND(JSON_START_TOJSONENABLE_BODY_IMPL VARDATA)


#ifndef _MSC_VER
#define JSON_START_TOJSONENABLE_BODY_IMPL(...)                                      \
    BOOST_PP_OVERLOAD(JSON_START_TOJSONENABLE_BODY_IMPL, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_START_TOJSONENABLE_BODY_IMPL(...)                                      \
   BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_START_TOJSONENABLE_BODY_IMPL,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif


#define JSON_START_TOJSONENABLE_BODY_IMPL1(VARNAME)       \
    JSON_START_TOJSONENABLE_BODY_IMPL2(VARNAME, BOOST_PP_WSTRINGIZE(VARNAME))


#define JSON_START_TOJSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)            \
    jsonData += L"\"" JSONKEY L"\":";                                   \
    typedef decltype(classFor->VARNAME) BOOST_PP_CAT(__type, VARNAME);  \
    jsonData += JSON::JSONFnInvoker<BOOST_PP_CAT(__type, VARNAME)>      \
                    ::ToJSON(&classFor->VARNAME);

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_MAKE_TOJSONENABLE_BODY(s, IGNORED, VARDATA)      \
    BOOST_PP_EXPAND(JSON_MAKE_TOJSONENABLE_BODY_IMPL VARDATA)


#ifndef _MSC_VER
#define JSON_MAKE_TOJSONENABLE_BODY_IMPL(...)                                      \
    BOOST_PP_OVERLOAD(JSON_MAKE_TOJSONENABLE_BODY_IMPL, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_MAKE_TOJSONENABLE_BODY_IMPL(...)                                      \
   BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_MAKE_TOJSONENABLE_BODY_IMPL,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif


#define JSON_MAKE_TOJSONENABLE_BODY_IMPL1(VARNAME)        \
    JSON_MAKE_TOJSONENABLE_BODY_IMPL2(VARNAME, VARNAME)


#define JSON_MAKE_TOJSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)             \
    jsonData += L",\"" BOOST_PP_WSTRINGIZE(JSONKEY) L"\":";             \
    typedef decltype(classFor->VARNAME) BOOST_PP_CAT(__type, VARNAME);  \
    jsonData += JSON::JSONFnInvoker<BOOST_PP_CAT(__type, VARNAME)>      \
                    ::ToJSON(&classFor->VARNAME);


////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_COLLECT_FROMJSONENABLE_DATA(s, IGNORED, VARDATA)   \
    BOOST_PP_EXPAND(JSON_COLLECT_FROMJSONENABLE_DATA_IMPL VARDATA)


#ifndef _MSC_VER
#define JSON_COLLECT_FROMJSONENABLE_DATA_IMPL(...)                                      \
    BOOST_PP_OVERLOAD(JSON_COLLECT_FROMJSONENABLE_DATA_IMPL, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_COLLECT_FROMJSONENABLE_DATA_IMPL(...)                                      \
   BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_COLLECT_FROMJSONENABLE_DATA_IMPL,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif


#define JSON_COLLECT_FROMJSONENABLE_DATA_IMPL1(VARNAME)        \
    JSON_COLLECT_FROMJSONENABLE_DATA_IMPL2(VARNAME, BOOST_PP_WSTRINGIZE(VARNAME))


#define JSON_COLLECT_FROMJSONENABLE_DATA_IMPL2(VARNAME, JSONKEY)    \
    memberMap.insert(std::make_pair(JSONKEY, (void*)&classInto.VARNAME));  \

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_MAKE_FROMJSONENABLE_BODY(s, IGNORED, VARDATA)   \
    BOOST_PP_EXPAND(JSON_MAKE_FROMJSONENABLE_BODY_IMPL VARDATA)

#ifndef _MSC_VER
#define JSON_MAKE_FROMJSONENABLE_BODY_IMPL(...)                                      \
    BOOST_PP_OVERLOAD(JSON_MAKE_FROMJSONENABLE_BODY_IMPL, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_MAKE_FROMJSONENABLE_BODY_IMPL(...)                                      \
   BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_MAKE_FROMJSONENABLE_BODY_IMPL,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif


#define JSON_MAKE_FROMJSONENABLE_BODY_IMPL1(VARNAME)        \
    JSON_MAKE_FROMJSONENABLE_BODY_IMPL2(VARNAME, BOOST_PP_WSTRINGIZE(VARNAME))


#define JSON_MAKE_FROMJSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)           \
    iter = ParseNextKey(iter, end, nextKey);                            \
    iter = AdvancePastWhitespace(iter, end);                            \
    if(iter == end || *iter != L':') {                                  \
        throw std::invalid_argument("Not a valid key map");             \
    }                                                                   \
                                                                        \
    ++iter;                                                             \
    iter = AdvancePastWhitespace(iter, end);                            \
    typedef decltype(classInto.VARNAME) BOOST_PP_CAT(__type, VARNAME);  \
                                                                        \
    insertAt = memberMap.find(nextKey);                                 \
    if(insertAt == memberMap.end()) {                                   \
        throw std::invalid_argument("No key in object");                \
    }                                                                   \
                                                                        \
    iter = JSON::JSONFnInvoker<BOOST_PP_CAT(__type, VARNAME)>           \
                      ::FromJSON(iter, end,                             \
           *static_cast<BOOST_PP_CAT(__type, VARNAME)*>(insertAt->second)); \
    iter = AdvancePastWhitespace(iter, end);                            \
    if(iter != end && *iter == L',') {                                  \
        ++iter;                                                         \
    }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_INHERITS(...)                                  \
    std::wstring ToJSON() {                                 \
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
    typedef std::unordered_map<std::wstring, void*> DataMap;

    template<typename ClassFor>
    struct JSONEnabler {
        json_finline static std::wstring ToJSON(const ClassFor*);
        json_finline static std::wstring FromJSON(const std::wstring&);
        json_finline static jsonIter FromJSON(jsonIter, jsonIter, ClassFor&);
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

        static ClassFor FromJSON(const std::wstring& jsonString) {
            return JSONEnabler<ClassFor>::FromJSON(jsonString);
        }

        static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassFor& into) {
            return JSONEnabler<ClassFor>::FromJSON(iter, end, into);
        }
    };
}
#endif
