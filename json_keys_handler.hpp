#pragma once
#ifndef __JSON_KEYS_HANDLER_HPP__
#define __JSON_KEYS_HANDLER_HPP__

namespace tjson {

#define JSON_CREATE_KEYS(CLASS_NAME, ...)               \
    BOOST_PP_SEQ_FOR_EACH(                              \
        JSON_REFERENCE_KEY, CLASS_NAME,                 \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)           \
    )

#define JSON_REFERENCE_KEY(s, CLASS_NAME, VARDATA)              \
    template<>                                                  \
    struct MemberInfo<                                          \
        JSON_MEMBER_INFO_ARGS(CLASS_NAME, VARDATA)> {           \
        static constexpr const char key[] = "\""                \
            BOOST_PP_EXPAND(JSON_KEY_NAME VARDATA) "\"";        \
    };                                                          \
                                                                \
    constexpr const char MemberInfo<                            \
        JSON_MEMBER_INFO_ARGS(CLASS_NAME, VARDATA)              \
    >::key[];

#define JSON_MEMBER_INFO_ARGS(CLASS_NAME, VARDATA) \
        decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),   \
        &CLASS_NAME:: JSON_VARNAME VARDATA

#define JSON_KEY_NAME(...)                                         \
    BOOST_PP_OVERLOAD(JSON_KEY_NAME, __VA_ARGS__)(__VA_ARGS__)

#define JSON_KEY_NAME2(VARNAME, JSONKEY)   \
    JSONKEY

#define JSON_KEY_NAME1(VARNAME)                            \
    JSON_KEY_NAME2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))

}
#endif
