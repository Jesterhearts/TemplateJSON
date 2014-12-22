#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace JSON {
    template<typename memberType, memberType member>
    struct MemberInfo {
        static const char* key;
        static const size_t len;
    };

    template<typename... members>
    struct MemberList {};

    template<typename classType, typename underlyingType>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter,
                                         underlyingType classType::* member) {
        return detail::FromJSON(iter, classOn.*member);
    }

    template<typename classType, typename underlyingType>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter,
                                         underlyingType* member) {
        return detail::FromJSON(iter, *member);
    }

    template<typename classType,
             typename underlyingType, underlyingType member,
             template<typename UT, UT MT> class MemberInfo>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter,
                                         MemberInfo<underlyingType, member>&&) {
        return MemberFromJSON(classOn, iter, member);
    }

    template<typename classType, typename memberType>
    jsonIter MemberFromJSON(classType& classOn, jsonIter iter) {
        return MemberFromJSON(classOn, iter, memberType());
    }

    template<typename classFor,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline jsonIter MemberFromJSON(classFor& on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<member, members...>&&) {
        if(keylen == member::len && std::memcmp(startOfKey, member::key, member::len) == 0) {
            return MemberFromJSON<classFor, member>(on, iter);
        }
        else {
            return MemberFromJSON(on, startOfKey, keylen, iter, ML<members...>());
        }
    }

#ifndef _MSC_VER
    template<typename classFor,
             template<typename... M> class ML,
             typename... value_types>
    json_finline jsonIter MemberFromJSON(classFor& on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<>&&) {
#else
    template<typename classFor,
        typename... members,
        template<typename... M> class ML,
        typename... value_types>
    json_finline jsonIter MemberFromJSON(classFor& on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<members...>&&) {
#endif
        ThrowBadJSONError(startOfKey, "No key in object");
    }

#define JSON_LIST_MEMBERS(CLASS_NAME, ...)          \
    JSON_MEMBER_POINTER(                            \
        CLASS_NAME,                                 \
        BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)      \
    )                                               \
    BOOST_PP_SEQ_FOR_EACH(                          \
        JSON_CREATE_MEMBER_POINTER, CLASS_NAME,     \
        BOOST_PP_SEQ_POP_FRONT(                     \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
        )                                           \
    )

#define JSON_CREATE_MEMBER_POINTER(s, CLASS_NAME, VARDATA)  \
    , JSON_MEMBER_POINTER(CLASS_NAME, VARDATA)

//////////////////////////////////////////////
#define JSON_MEMBER_POINTER(CLASS_NAME, VARDATA)                \
    MemberInfo<decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),    \
               &CLASS_NAME:: JSON_VARNAME VARDATA>

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_CREATE_MEMBERS(CLASS_NAME, ...)                        \
    template<>                                                      \
    struct MembersHolder<CLASS_NAME> {                              \
        json_finline constexpr static MemberList<                   \
            JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)              \
        > members() {                                               \
            return MemberList<                                      \
                        JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)  \
            >();                                                    \
        }                                                           \
        MembersHolder() = delete;                                   \
        ~MembersHolder() = delete;                                  \
    };

#ifndef _MSC_VER
#define JSON_VARNAME(...)                                      \
    BOOST_PP_OVERLOAD(JSON_VARNAME, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_VARNAME(...)                                                                   \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_VARNAME, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_VARNAME1(VARNAME)  \
    VARNAME

#define JSON_VARNAME2(VARNAME, IGNORED) \
    JSON_VARNAME1(VARNAME)

};

#endif
