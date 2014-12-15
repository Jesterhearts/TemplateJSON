#pragma once
#ifndef __JSON_KEYS_HANDLER_HPP__
#define __JSON_KEYS_HANDLER_HPP__

namespace JSON {

#define JSON_CREATE_KEYS(CLASS_NAME, ...)               \
    BOOST_PP_SEQ_FOR_EACH(                              \
        JSON_REFERENCE_KEY, CLASS_NAME,                 \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)           \
    )

#define JSON_REFERENCE_KEY(s, CLASS_NAME, VARDATA)      \
    template<> const std::string MemberInfo<            \
        decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),   \
        &CLASS_NAME:: JSON_VARNAME VARDATA>::key =      \
    BOOST_PP_EXPAND(JSON_KEY_REFERENCE VARDATA);

#ifndef _MSC_VER
#define JSON_KEY_REFERENCE(...)                                         \
    BOOST_PP_OVERLOAD(JSON_KEY_REFERENCE, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_KEY_REFERENCE(...)                                                                   \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_KEY_REFERENCE, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_KEY_REFERENCE2(VARNAME, JSONKEY)   \
    JSONKEY

#define JSON_KEY_REFERENCE1(VARNAME)                            \
    JSON_KEY_REFERENCE2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))

}
#endif
