#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace JSON {
    template<typename memberType, memberType member>
    struct MemberInfo {
        static const stringt key;
    };

    template<typename... members>
    struct MemberList {};

    template<typename classType>
    jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end);

    template<typename classType, typename underlyingType>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end,
                                         underlyingType classType::* member) {
        return JSONFnInvoker<underlyingType>::FromJSON(iter, end, classOn.*member);
    }

    template<typename classType, typename underlyingType>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end,
                                         underlyingType* member) {
        return JSONFnInvoker<underlyingType>::FromJSON(iter, end, *member);
    }

    template<typename classType,
             typename underlyingType, underlyingType member,
             template<typename UT, UT MT> class MemberInfo>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end,
                                         MemberInfo<underlyingType, member>&& m) {
        return MemberFromJSON(classOn, iter, end, member);
    }

    template<typename classType, typename memberType>
    jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end) {
        return MemberFromJSON(classOn, iter, end, memberType());
    }

    namespace MapTypes {
        template<typename T>
        using maptype = std::unordered_map<std::reference_wrapper<const stringt>,
                                           jsonIter (*const)(T&, jsonIter, jsonIter),
                                           std::hash<stringt>,
                                           std::equal_to<stringt>>;

        template<typename T>
        using value_type = typename maptype<T>::value_type;
    }

    template<typename classFor,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(ML<member, members...>&& ml) {
        return CreateMap<classFor>(
            MemberList<members...>(),
            MapTypes::value_type<classFor>{
                std::reference_wrapper<const stringt>(member::key),
                &MemberFromJSON<classFor, member>
            }
        );
    }

    template<typename classFor,
             typename member, typename... members,
             template<typename... M> class ML,
             typename... value_types>
    json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(ML<member, members...>&& ml,
                                                                              value_types&&... pairs) {
        return CreateMap<classFor>(
            MemberList<members...>(),
            MapTypes::value_type<classFor>{
                std::reference_wrapper<const stringt>(member::key),
                &MemberFromJSON<classFor, member>
            },
            pairs...
        );
    }

#ifndef _MSC_VER
    template<typename classFor,
             template<typename... M> class ML,
             typename... value_types>
    json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(ML<>&& ml,
                                                                              value_types&&... pairs) {
#else
    template<typename classFor,
        typename... members,
        template<typename... M> class ML,
        typename... value_types>
        json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(ML<members...>&& ml,
        value_types&&... pairs) {
#endif
        return { pairs... };
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
