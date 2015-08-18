#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace tjson {
namespace detail {
    template<typename memberType, memberType member>
    struct MemberInfo {};

    template<typename... members>
    struct MemberList {};

    template<typename ClassFor>
    using MembersFor = typename MembersHolder<ClassFor>::members;

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
#define JSON_CREATE_MEMBERS(CLASS_NAME, ...)                                    \
    template<>                                                                  \
    struct MembersHolder<CLASS_NAME> {                                          \
        using members = MemberList<JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)>; \
        MembersHolder() = delete;                                               \
        ~MembersHolder() = delete;                                              \
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

}
};

#endif
