#pragma once
#ifndef __JSON_KEYS_HANDLER_HPP__
#define __JSON_KEYS_HANDLER_HPP__

namespace JSON {

    template<const stringt&... keys>
    struct KeyList {};

    template<typename classType>
    struct KeysHolder;

#define JSON_CREATE_KEYS(CLASS_NAME, ...)               \
    template<>                                          \
    struct KeysHolder<CLASS_NAME> {                     \
    private:                                            \
        BOOST_PP_SEQ_FOR_EACH(                          \
            JSON_CREATE_KEYS_IMPL, ~,                   \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
        )                                               \
    public:                                             \
        constexpr static JSON::KeyList<                 \
            JSON_LIST_KEYS(CLASS_NAME, __VA_ARGS__)     \
        > keys() {                                      \
            return JSON::KeyList<                       \
                JSON_LIST_KEYS(CLASS_NAME, __VA_ARGS__) \
            >();                                        \
        }                                               \
    };                                                  \
                                                        \
    BOOST_PP_SEQ_FOR_EACH(                              \
        JSON_REFERENCE_KEY, KeysHolder<CLASS_NAME>::,   \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)           \
    )

#define JSON_CREATE_KEYS_IMPL(s, IGNORED, VARDATA)  \
    BOOST_PP_EXPAND(JSON_KEY_CREATOR VARDATA)

#ifndef _MSC_VER
#define JSON_KEY_CREATOR(...)                                      \
    BOOST_PP_OVERLOAD(JSON_KEY_CREATOR, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_KEY_CREATOR(...)                                                                   \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_KEY_CREATOR, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_KEY_CREATOR1(VARNAME)              \
    static const stringt VARNAME##__JSON_KEY;

#define JSON_KEY_CREATOR2(VARNAME, JSONKEY) \
    JSON_KEY_CREATOR1(VARNAME)

#define JSON_REFERENCE_KEY(s, CLASS, VARDATA)                       \
    const stringt CLASS BOOST_PP_EXPAND(JSON_KEY_REFERENCE VARDATA)

#ifndef _MSC_VER
#define JSON_KEY_REFERENCE(...)                                      \
    BOOST_PP_OVERLOAD(JSON_KEY_REFERENCE, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_KEY_REFERENCE(...)                                                                   \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_KEY_REFERENCE, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_KEY_REFERENCE2(VARNAME, JSONKEY)   \
     VARNAME##__JSON_KEY = JSON_ST(JSONKEY);

#define JSON_KEY_REFERENCE1(VARNAME)                            \
    JSON_KEY_REFERENCE2(VARNAME, BOOST_PP_STRINGIZE(VARNAME))

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_LIST_KEYS(CLASS_NAME, ...)             \
    JSON_LIST_KEY(                                  \
        CLASS_NAME,                                 \
        BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)      \
    )                                               \
    BOOST_PP_SEQ_FOR_EACH(                          \
        JSON_LIST_KEYS_IMPL, CLASS_NAME,            \
        BOOST_PP_SEQ_POP_FRONT(                     \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)   \
        )                                           \
    )

#define JSON_LIST_KEYS_IMPL(s, CLASS_NAME, VARDATA) \
    , JSON_LIST_KEY(CLASS_NAME, VARDATA)

#define JSON_LIST_KEY(CLASS_NAME, VARDATA)  \
    JSON_LIST_KEY_IMPL VARDATA

#ifndef _MSC_VER
#define JSON_LIST_KEY_IMPL(...)                                     \
    BOOST_PP_OVERLOAD(JSON_LIST_KEY_IMPL, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_LIST_KEY_IMPL(...)                                                                 \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_LIST_KEY_IMPL,__VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_LIST_KEY_IMPL2(VARNAME, JSONKEY)   \
    JSON_LIST_KEY_IMPL1(VARNAME)

#define JSON_LIST_KEY_IMPL1(VARNAME)    \
    VARNAME##__JSON_KEY

}
#endif
