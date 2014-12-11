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
#include "json_member_mapper.hpp"

namespace JSON {
    template<typename classFor, typename underlyingType>
    json_finline static stringt MemberToJSON(const classFor& classFrom, underlyingType classFor::* member) {
        return JSONFnInvoker<decltype(classFrom.*member)>::ToJSON(classFrom.*member);
    }

    template<typename classFor, typename underlyingType>
    json_finline static stringt MemberToJSON(const classFor& classFrom, underlyingType* member) {
        return JSONFnInvoker<underlyingType>::ToJSON(member);
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    json_finline static stringt MembersToJSON(const classFor& classFrom,
                                              const K<key, keys...>& k1,
                                              const ML<type, types...>& ml) {
        stringt json(1, JSON_ST('\"'));
        json.append(key);
        json.append(JSON_ST("\":"), 2);

        json.append(MemberToJSON(classFrom, type::value));
        if(sizeof...(keys) > 0)
        {
            json.append(1, JSON_ST(','));
        }
        json.append(MembersToJSON(classFrom, KeyList<keys...>(), MemberList<types...>()));
        return json;
    }

    template<typename classFor,
             template<const char_t*...> class K,
             template<typename... M> class ML>
    json_finline static stringt MembersToJSON(const classFor& classFrom,
                                              const K<>& k1,
                                              const ML<>& ml) {
        return JSON_ST("");
    }

    template<typename classType>
    struct KeysHolder;

    template<typename classFor>
    stringt ToJSON(const classFor& classFrom) {
        stringt json(JSON_ST("{"));

        json.append(MembersToJSON(classFrom,
                                  JSON::KeysHolder<classFor>::keys,
                                  JSON::MembersHolder<classFor>::members
            )
        );

        json.append(JSON_ST("}"));
        return json;
    }

    template<typename classFor>
    classFor FromJSON(const stringt& jsonData) {
        classFor classInto;
        auto iter = jsonData.begin();
        auto end = jsonData.end();

        FromJSON<classFor>(iter, end, classInto);
        return classInto;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                                \
    namespace JSON {                                                                \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                                   \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                                \
        JSON_MAKE_MEMBER_MAP(CLASS_NAME, __VA_ARGS__)                               \
                                                                                    \
        template stringt ToJSON<CLASS_NAME>(const CLASS_NAME&);                     \
                                                                                    \
        template<>                                                                  \
        jsonIter FromJSON<CLASS_NAME>(jsonIter iter, jsonIter end,                  \
                                      CLASS_NAME& classInto) {                      \
            typedef CLASS_NAME classFor;                                            \
            stringt nextKey;                                                        \
            typedef decltype(MemberMap<classFor>::mapping.values.begin()) memberIter;\
            memberIter insertAt;                                                    \
                                                                                    \
            iter = ValidateObjectStart(iter, end);                                  \
                                                                                    \
            BOOST_PP_SEQ_FOR_EACH(                                                  \
                JSON_MAKE_FROMJSONENABLE_BODY, _,                                   \
                        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
                                )                                                   \
                                                                                    \
            return ValidateObjectEnd(iter, end);                                    \
        }                                                                           \
                                                                                    \
        template CLASS_NAME FromJSON<CLASS_NAME>(const stringt&);                   \
    }


////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_CREATE_KEYS(CLASS_NAME, ...)                               \
    template<>                                                          \
    struct KeysHolder<CLASS_NAME> {                                     \
    private:                                                            \
        BOOST_PP_SEQ_FOR_EACH(                                          \
            JSON_CREATE_KEYS_IMPL, ~,                                   \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
        )                                                               \
    public:                                                             \
        static constexpr const auto keys =                              \
            JSON::KeyList<JSON_LIST_KEYS(CLASS_NAME, __VA_ARGS__)>();   \
    };                                                                  \
                                                                        \
    BOOST_PP_SEQ_FOR_EACH(                                              \
        JSON_REFERENCE_KEY, KeysHolder<CLASS_NAME>::,                   \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                           \
    )                                                                   \
                                                                        \
    constexpr const JSON::KeyList<                                      \
        JSON_LIST_KEYS(CLASS_NAME, __VA_ARGS__)                         \
    > KeysHolder<CLASS_NAME>::keys;


#define JSON_CREATE_KEYS_IMPL(s, IGNORED, VARDATA)  \
    BOOST_PP_EXPAND(JSON_KEY_CREATOR VARDATA)


#ifndef _MSC_VER
#define JSON_KEY_CREATOR(...)                                      \
    BOOST_PP_OVERLOAD(JSON_KEY_CREATOR, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_KEY_CREATOR(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_KEY_CREATOR, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif


#define JSON_KEY_CREATOR1(VARNAME)                          \
    JSON_KEY_CREATOR2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))


#define JSON_KEY_CREATOR2(VARNAME, JSONKEY)                                 \
    static constexpr const char_t VARNAME##__JSON_KEY[] = JSON_ST(JSONKEY);

//////////////////////////////////////////////////
#define JSON_REFERENCE_KEY(s, CLASS, VARDATA)                                       \
    constexpr const char_t CLASS BOOST_PP_EXPAND(JSON_KEY_REFERENCE VARDATA)


#ifndef _MSC_VER
#define JSON_KEY_REFERENCE(...)                                      \
    BOOST_PP_OVERLOAD(JSON_KEY_REFERENCE, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_KEY_REFERENCE(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_KEY_REFERENCE, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif


#define JSON_KEY_REFERENCE2(VARNAME, JSONKEY)                   \
    JSON_KEY_REFERENCE1(VARNAME)


#define JSON_KEY_REFERENCE1(VARNAME)    \
     VARNAME##__JSON_KEY[];

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_LIST_MEMBERS(CLASS_NAME, ...)          \
    JSON_FIRST_MEMBER_POINTER(                      \
        CLASS_NAME,                                 \
        BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)      \
    )                                               \
    BOOST_PP_SEQ_FOR_EACH(                          \
        JSON_CREATE_MEMBER_POINTER, CLASS_NAME,    \
        BOOST_PP_SEQ_POP_FRONT(                     \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
        )                                           \
    )

#define JSON_CREATE_MEMBER_POINTER(s, CLASS_NAME, VARDATA)    \
    , decltype(MembersHolder<CLASS_NAME>:: JSON_MEMBER_NAME VARDATA)

//////////////////////////////////////////////
#define JSON_FIRST_MEMBER_POINTER(CLASS_NAME, VARDATA)  \
    decltype(MembersHolder<CLASS_NAME>:: JSON_MEMBER_NAME VARDATA)

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


#define JSON_MAKE_FROMJSONENABLE_BODY_IMPL2(VARNAME, IGNORED)                   \
    iter = ParseNextKey(iter, end, nextKey);                                    \
    iter = ValidateKeyValueMapping(iter, end);                                  \
                                                                                \
    iter = AdvancePastWhitespace(iter, end);                                    \
                                                                                \
    insertAt = MemberMap<classFor>::mapping.values.find(nextKey.c_str());       \
    if(insertAt == MemberMap<classFor>::mapping.values.end()) {                 \
        ThrowBadJSONError(iter, end, JSON_ST("No key in object"));              \
    }                                                                           \
                                                                                \
    iter = JSON::JSONFnInvoker<decltype(classFor::VARNAME)>::FromJSON(iter, end,\
                *reinterpret_cast<decltype(classFor::VARNAME)*>(                \
                    insertAt->second.GetPointer(classInto)                      \
                )                                                               \
            );                                                                  \
                                                                                \
    iter = AdvancePastWhitespace(iter, end);                                    \
    if(iter != end && *iter == JSON_ST(',') ) {                                 \
        ++iter;                                                                 \
    }                                                                           \
    else if (*iter != JSON_ST('}')) {                                           \
        ThrowBadJSONError(iter, end, JSON_ST("Missing key separator"));         \
    }

#endif
