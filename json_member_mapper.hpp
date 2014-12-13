#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace JSON {

    template<typename memberType, memberType member>
    struct Member {};

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
             template<typename UT, UT MT> class Member>
    json_finline jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end,
                                         Member<underlyingType, member>&& m) {
        return MemberFromJSON(classOn, iter, end, member);
    }

    template<typename classType, typename memberType>
    jsonIter MemberFromJSON(classType& classOn, jsonIter iter, jsonIter end) {
        return MemberFromJSON(classOn, iter, end, memberType());
    }

    template<typename classType, typename memberType>
    struct DeclareMemberFromJSON {
        constexpr static jsonIter (*const fn)(classType&, jsonIter, jsonIter) = &MemberFromJSON<classType, memberType>;
    };

    template<typename classType>
    struct VirtualAccessor {
        constexpr VirtualAccessor(jsonIter (*const fn)(classType&, jsonIter, jsonIter)) : MemberFromJSON(fn) {};
        jsonIter (*const MemberFromJSON)(classType&, jsonIter, jsonIter);
    };

    template<typename classType>
    struct MemberMap;

    namespace MapTypes {
        template<typename T>
        using maptype = std::unordered_map<std::reference_wrapper<const stringt>,
                                           VirtualAccessor<T>,
                                           std::hash<stringt>,
                                           std::equal_to<stringt>>;

        template<typename T>
        using value_type = typename maptype<T>::value_type;
    }

    template<typename classFor,
             const stringt& key, const stringt&... keys,
             template<const stringt&...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(K<key, keys...>&& k1,
                                                                 ML<type, types...>&& ml) {
        return CreateMap<classFor>(KeyList<keys...>(), MemberList<types...>(),
                         MapTypes::value_type<classFor>{
                            std::reference_wrapper<const stringt>(key),
                            VirtualAccessor<classFor>(DeclareMemberFromJSON<classFor, type>::fn)
                        }
                );
    }

    template<typename classFor,
             const stringt& key, const stringt&... keys,
             template<const stringt&...> class K,
             typename type, typename... types,
             template<typename... M> class ML,
             typename... value_types>
    json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(K<key, keys...>&& k2,
                                                                 ML<type, types...>&& ml,
                                                                 value_types&&... pairs) {
        return CreateMap<classFor>(KeyList<keys...>(), MemberList<types...>(),
                         MapTypes::value_type<classFor>{
                            std::reference_wrapper<const stringt>(key),
                            VirtualAccessor<classFor>(DeclareMemberFromJSON<classFor, type>::fn)
                        },
                        pairs...
                );
    }

    template<typename classFor,
             template<const stringt&... T> class K,
             template<typename... M> class ML,
             typename... value_types>
    json_finline constexpr static const MapTypes::maptype<classFor> CreateMap(K<>&& k3,
                                                                 ML<>&& ml,
                                                                 value_types&&... pairs) {
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
#define JSON_MEMBER_POINTER(CLASS_NAME, VARDATA)        \
    Member<decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),\
                    &CLASS_NAME:: JSON_VARNAME VARDATA> \

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_CREATE_MEMBERS(CLASS_NAME, ...)                            \
    template<>                                                          \
    struct MembersHolder<CLASS_NAME> {                                  \
        json_finline constexpr static MemberList<                       \
            JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)                  \
        > members() {                                                   \
            return MemberList<                                          \
                        JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)      \
            >();                                                        \
        }                                                               \
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
