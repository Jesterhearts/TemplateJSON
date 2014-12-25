#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace tjson {
    template<typename memberType, memberType member>
    struct MemberInfo {};

    template<typename... members>
    struct MemberList {};

    template<typename ClassType, typename UnderlyingType>
    json_finline jsonIter member_from_json(ClassType& classOn, jsonIter iter,
                                         UnderlyingType ClassType::* member) {
        return detail::from_json(iter, classOn.*member);
    }

    template<typename ClassType, typename UnderlyingType>
    json_finline jsonIter member_from_json(ClassType& classOn, jsonIter iter,
                                         UnderlyingType* member) {
        return detail::from_json(iter, *member);
    }

    template<typename ClassType,
             typename UnderlyingType, UnderlyingType member,
             template<typename UT, UT MT> class MemberInfo>
    json_finline jsonIter member_from_json(ClassType& classOn, jsonIter iter,
                                         MemberInfo<UnderlyingType, member>&&) {
        return member_from_json(classOn, iter, member);
    }

    template<typename ClassType, typename memberType>
    json_finline jsonIter member_from_json(ClassType& classOn, jsonIter iter) {
        return member_from_json(classOn, iter, memberType());
    }

    template<typename ClassType,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline jsonIter member_from_json(ClassType& on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<member, members...>&&) {
        constexpr const size_t len = sizeof(member::key) - 1;
        if(keylen == len && std::memcmp(startOfKey, member::key, len) == 0) {
            return member_from_json<ClassType, member>(on, iter);
        }
        else {
            return member_from_json(on, startOfKey, keylen, iter, ML<members...>());
        }
    }

#ifndef _MSC_VER
    template<typename ClassType,
             template<typename... M> class ML,
             typename... value_types>
    json_finline jsonIter member_from_json(ClassType& on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<>&&) {
#else
    template<typename ClassType,
        typename... members,
        template<typename... M> class ML,
        typename... value_types>
    json_finline jsonIter member_from_json(ClassType& on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<members...>&&) {
#endif
        json_parsing_error(startOfKey, "No key in object");
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
