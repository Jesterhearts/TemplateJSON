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
    json_finline stringt MemberToJSON(const classFor& classFrom, underlyingType classFor::* member) {
        return JSONFnInvoker<decltype(classFrom.*member)>::ToJSON(classFrom.*member);
    }

    template<typename classFor, typename underlyingType>
    json_finline stringt MemberToJSON(const classFor& classFrom, underlyingType* member) {
        return JSONFnInvoker<underlyingType>::ToJSON(member);
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    json_finline stringt MembersToJSON(const classFor& classFrom,
                                       const K<key, keys...>& k1,
                                       const ML<type, types...>& ml) {
        stringt json(1, JSON_ST('\"'));
        json.append(key);
        json.append(JSON_ST("\":"), 2);

        json.append(MemberToJSON(classFrom, type::value));
        if(sizeof...(keys) > 0) {
            json.append(1, JSON_ST(','));
        }
        json.append(MembersToJSON(classFrom, K<keys...>(), ML<types...>()));
        return json;
    }

    template<typename classFor,
             template<const char_t*...> class K,
             template<typename... M> class ML>
    json_finline stringt MembersToJSON(const classFor& classFrom, K<>&& k1, ML<>&& ml) {
        return JSON_ST("");
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    json_finline jsonIter MembersFromJSON(classFor& classInto,
                                          jsonIter iter, jsonIter end,
                                          const K<key, keys...>& k1,
                                          const ML<type, types...>& ml) {
        stringt nextKey;
        iter = ParseNextKey(iter, end, nextKey);
        iter = ValidateKeyValueMapping(iter, end);

        iter = AdvancePastWhitespace(iter, end);

        auto insertAt = MemberMap<classFor>::mapping.values.find(nextKey.c_str());
        if(insertAt == MemberMap<classFor>::mapping.values.end()) {
            ThrowBadJSONError(iter, end, JSON_ST("No key in object"));
        }

        iter = insertAt->second.MemberToJSON(classInto, iter, end);

        iter = AdvancePastWhitespace(iter, end);
        if(iter != end && *iter == JSON_ST(',') ) {
            ++iter;
        }
        else if (*iter != JSON_ST('}')) {
            ThrowBadJSONError(iter, end, JSON_ST("Missing key separator"));
        }

        return MembersFromJSON(classInto, iter, end, K<keys...>(), ML<types...>());
    }

    template<typename classFor,
             template<const char_t*...> class K,
             template<typename... M> class ML>
    json_finline jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end, K<>&& k1,
                                          ML<>&& ml) {
        return iter;
    }

    template<typename classType>
    struct KeysHolder;

    template<typename classFor>
    stringt ToJSON(const classFor& classFrom) {
        stringt json(JSON_ST("{"));

        json.append(MembersToJSON(classFrom,
                                  KeysHolder<classFor>::keys,
                                  MembersHolder<classFor>::members
            )
        );

        json.append(JSON_ST("}"));
        return json;
    }

    template<typename classFor>
    jsonIter FromJSON(jsonIter iter, jsonIter end, classFor& classInto) {
        iter = ValidateObjectStart(iter, end);

        iter = MembersFromJSON<classFor>(classInto, iter, end,
                                         KeysHolder<classFor>::keys,
                                         MembersHolder<classFor>::members
               );

        return ValidateObjectEnd(iter, end);
    }

    template<typename classFor>
    classFor FromJSON(const stringt& jsonData) {
        classFor classInto;
        auto iter = jsonData.begin();
        auto end = jsonData.end();

        FromJSON(iter, end, classInto);

        return classInto;
    }

    template<typename classFor>
    struct ClassMap {
        ClassMap()
            : values(CreateMap<classFor>(KeysHolder<classFor>::keys, MembersHolder<classFor>::members))
        {}

        const MapTypes::maptype<classFor> values;
    };

    template<typename classFor>
    struct MemberMap {
        static const ClassMap<classFor> mapping;
    };

    template<typename classFor>
    const ClassMap<classFor> MemberMap<classFor>::mapping;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                \
    namespace JSON {                                                \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                   \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                \
                                                                    \
        template struct MemberMap<CLASS_NAME>;                      \
                                                                    \
        template stringt ToJSON<CLASS_NAME>(const CLASS_NAME&);     \
        template CLASS_NAME FromJSON<CLASS_NAME>(const stringt&);   \
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

#endif
