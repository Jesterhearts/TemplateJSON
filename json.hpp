#pragma once
#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/expand.hpp>

#include <boost/preprocessor/stringize.hpp>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/variadic/elem.hpp>

#include "json_common_defs.hpp"
#include "json_value_parser.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                                \
    namespace JSON {                                                                \
        template<>                                                                  \
        stringt ToJSON<CLASS_NAME>(const CLASS_NAME& classFrom) {                   \
                                                                                    \
            const CLASS_NAME* classFor = static_cast<const CLASS_NAME*>(&classFrom);\
            stringt jsonData(JSON_ST("{"));                                         \
                                                                                    \
            JSON_START_TOJSONENABLE_BODY(                                           \
                BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)                              \
                                      )                                             \
                                                                                    \
            BOOST_PP_SEQ_FOR_EACH(                                                  \
                JSON_MAKE_TOJSONENABLE_BODY, _,                                     \
                    BOOST_PP_SEQ_POP_FRONT(                                         \
                        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
                                           )                                        \
                                )                                                   \
                                                                                    \
            jsonData += JSON_ST("}");                                               \
            return jsonData;                                                        \
        }                                                                           \
                                                                                    \
        template<>                                                                  \
        jsonIter FromJSON<CLASS_NAME>(jsonIter iter, jsonIter end,                  \
                                      CLASS_NAME& classInto) {                      \
                                                                                    \
            iter = AdvancePastWhitespace(iter, end);                                \
            if(iter == end || *iter != JSON_ST('{')) {                              \
                throw std::invalid_argument("No object start token");               \
            }                                                                       \
            ++iter;                                                                 \
                                                                                    \
            stringt nextKey;                                                        \
                                                                                    \
            DataMap memberMap;                                                      \
            DataMap::const_iterator insertAt;                                       \
                                                                                    \
            BOOST_PP_SEQ_FOR_EACH(                                                  \
                JSON_COLLECT_FROMJSONENABLE_DATA, _,                                \
                        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
                                )                                                   \
                                                                                    \
            BOOST_PP_SEQ_FOR_EACH(                                                  \
                JSON_MAKE_FROMJSONENABLE_BODY, _,                                   \
                        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
                                )                                                   \
                                                                                    \
            iter = AdvancePastWhitespace(iter, end);                                \
            if(iter == end || *iter != JSON_ST('}')) {                              \
                throw std::invalid_argument("No object end token");                 \
            }                                                                       \
            ++iter;                                                                 \
            return iter;                                                            \
        }                                                                           \
                                                                                    \
        template<>                                                                  \
        CLASS_NAME FromJSON<CLASS_NAME>(const stringt& jsonData) {                  \
            CLASS_NAME classInto;                                                   \
                                                                                    \
            auto iter = jsonData.begin();                                           \
            auto end = jsonData.end();                                              \
                                                                                    \
            FromJSON<CLASS_NAME>(iter, end, classInto);                             \
            return classInto;                                                       \
        }                                                                           \
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
    JSON_START_TOJSONENABLE_BODY_IMPL2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))


#define JSON_START_TOJSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)            \
    jsonData += JSON_ST("\"") JSON_ST(JSONKEY) JSON_ST("\":");          \
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
    JSON_MAKE_TOJSONENABLE_BODY_IMPL2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))


#define JSON_MAKE_TOJSONENABLE_BODY_IMPL2(VARNAME, JSONKEY)             \
    jsonData += JSON_ST(",\"") JSON_ST(JSONKEY) JSON_ST("\":");         \
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
    JSON_COLLECT_FROMJSONENABLE_DATA_IMPL2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))


#define JSON_COLLECT_FROMJSONENABLE_DATA_IMPL2(VARNAME, JSONKEY)    \
    memberMap.insert(std::make_pair(JSON_ST(JSONKEY), (void*)&classInto.VARNAME));  \

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
    JSON_MAKE_FROMJSONENABLE_BODY_IMPL2(VARNAME, _)


#define JSON_MAKE_FROMJSONENABLE_BODY_IMPL2(VARNAME, IGNORED)           \
    iter = ParseNextKey(iter, end, nextKey);                            \
    iter = AdvancePastWhitespace(iter, end);                            \
    if(iter == end || *iter != JSON_ST(':')) {                          \
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
    if(iter != end && *iter == JSON_ST(',')) {                          \
        ++iter;                                                         \
    }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_INHERITS(...)                                  \
    stringt ToJSON() {                                      \
        stringt jsonData(JSON_ST("["));                     \
        JSON_START_JSONINHERITS_BODY(                       \
            BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)          \
                                  )                         \
        BOOST_PP_SEQ_FOR_EACH(                              \
            JSON_MAKE_JSONINHERITS_BODY, _,                 \
                BOOST_PP_SEQ_POP_FRONT(                     \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
                                       )                    \
                            )                               \
        jsonData += JSON_ST("]");                           \
        return jsonData;                                    \
     }

////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_START_JSONINHERITS_BODY(CLASSNAME)                 \
    jsonData += JSON_ST("{\"") JSON_ST(BOOST_PP_STRINGIZE(CLASSNAME)) JSON_ST("\":");   \
    jsonData += JSON::JSONBase<CLASSNAME>::ToJSON();            \
    jsonData += JSON_ST("}");


#define JSON_MAKE_JSONINHERITS_BODY(s, IGNORED, CLASSNAME)      \
    jsonData += JSON_ST(",{\"") JSON_ST(BOOST_PP_STRINGIZE(CLASSNAME)) JSON_ST("\":");  \
    jsonData += CLASSNAME::ToJSON();                            \
    jsonData += JSON_ST("}");

#endif
