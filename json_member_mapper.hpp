#pragma once
#ifndef __JSON_MEMBER_MAPPER_HPP__
#define __JSON_MEMBER_MAPPER_HPP__

namespace JSON {

    template<const char_t*... keys>
    struct KeyList {
        constexpr KeyList() {};
    };

    template<typename memberType, memberType member>
    struct Member {
        constexpr Member() {};

        typedef memberType type;
        constexpr static memberType value = member;
    };

    template<typename... members>
    struct MemberList {
        constexpr MemberList() {};
    };

    template<typename classType>
    jsonIter MemberToJSON(classType& classOn, jsonIter iter, jsonIter end);

    template<typename classType, typename underlyingType>
    jsonIter ResolveMemberToJSONImpl(classType& classOn, jsonIter iter, jsonIter end,
                                     underlyingType classType::* member) {
        return JSONFnInvoker<underlyingType>::FromJSON(iter, end, classOn.*member);
    }

    template<typename classType, typename underlyingType>
    jsonIter ResolveMemberToJSONImpl(classType& classOn, jsonIter iter, jsonIter end,
                                     underlyingType* member) {
        return JSONFnInvoker<underlyingType>::FromJSON(iter, end, *member);
    }

    template<typename classType, typename memberType>
    jsonIter MemberToJSON(classType& classOn, jsonIter iter, jsonIter end) {
        return ResolveMemberToJSONImpl(classOn, iter, end, memberType::value);
    }

    template<typename classType, typename memberType>
    struct DeclareMemberToJSON {
        constexpr static jsonIter (*const fn)(classType&, jsonIter, jsonIter) = &MemberToJSON<classType, memberType>;
    };

    template<typename classType>
    struct VirtualAccessor {
        constexpr VirtualAccessor(jsonIter (*const fn)(classType&, jsonIter, jsonIter)) : MemberToJSON(fn) {};
        jsonIter (*const MemberToJSON)(classType&, jsonIter, jsonIter);
    };

    template<typename classType>
    struct MemberMap;

    namespace MapTypes {
        template<typename T>
        using maptype = std::unordered_map<stringt, VirtualAccessor<T>>;

        template<typename T>
        using value_type = typename maptype<T>::value_type;
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    constexpr static const MapTypes::maptype<classFor> CreateMap(const K<key, keys...>& k1,
                                                                 const ML<type, types...>& ml) {
        return CreateMap<classFor>(KeyList<keys...>(), MemberList<types...>(),
                         MapTypes::value_type<classFor>{
                            stringt(key),
                            VirtualAccessor<classFor>(DeclareMemberToJSON<classFor, type>::fn)
                        }
                );
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML,
             typename... value_types>
    constexpr static const MapTypes::maptype<classFor> CreateMap(K<key, keys...>&& k2,
                                                                 ML<type, types...>&& ml,
                                                                 value_types&&... pairs) {
        return CreateMap<classFor>(KeyList<keys...>(), MemberList<types...>(),
                         MapTypes::value_type<classFor>{
                            stringt(key),
                            VirtualAccessor<classFor>(DeclareMemberToJSON<classFor, type>::fn)
                        },
                        pairs...
                );
    }

    template<typename classFor,
             template<const char_t*... T> class K,
             template<typename... M> class ML,
             typename... value_types>
    constexpr static const MapTypes::maptype<classFor> CreateMap(K<>&& k3,
                                                                 ML<>&& ml,
                                                                 value_types&&... pairs) {
        return { pairs... };
    }

#define JSON_LIST_MEMBERS(CLASS_NAME, ...)          \
    JSON_FIRST_MEMBER_POINTER(                      \
        CLASS_NAME,                                 \
        BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)      \
    )                                               \
    BOOST_PP_SEQ_FOR_EACH(                          \
        JSON_CREATE_MEMBER_POINTER, CLASS_NAME,     \
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
#define JSON_CREATE_MEMBERS(CLASS_NAME, ...)                            \
    template<>                                                          \
    struct MembersHolder<CLASS_NAME> {                                  \
    private:                                                            \
        BOOST_PP_SEQ_FOR_EACH(                                          \
            JSON_CREATE_MEMBERS_IMPL, CLASS_NAME,                       \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
        )                                                               \
    public:                                                             \
        static constexpr const auto members =                           \
            MemberList<JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)>();   \
    };                                                                  \
                                                                        \
    BOOST_PP_SEQ_FOR_EACH(                                              \
        JSON_REFERENCE_MEMBER, CLASS_NAME,                              \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                           \
    )                                                                   \
                                                                        \
    constexpr const MemberList<                                         \
        JSON_LIST_MEMBERS(CLASS_NAME, __VA_ARGS__)                      \
    > MembersHolder<CLASS_NAME>::members;

#define JSON_CREATE_MEMBERS_IMPL(s, CLASS_NAME, VARDATA)        \
    static constexpr const auto JSON_MEMBER_NAME VARDATA =      \
        Member<decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),    \
                     &CLASS_NAME:: JSON_VARNAME VARDATA>();

#ifndef _MSC_VER
#define JSON_MEMBER_NAME(...)                                      \
    BOOST_PP_OVERLOAD(JSON_MEMBER_NAME, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_MEMBER_NAME(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_MEMBER_NAME, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_MEMBER_NAME1(VARNAME)  \
    VARNAME ## __MEMBER

#define JSON_MEMBER_NAME2(VARNAME, IGNORED)  \
    JSON_MEMBER_NAME1(VARNAME)

#ifndef _MSC_VER
#define JSON_VARNAME(...)                                      \
    BOOST_PP_OVERLOAD(JSON_VARNAME, __VA_ARGS__)(__VA_ARGS__)
#else
#define JSON_VARNAME(...)                                      \
    BOOST_PP_CAT(BOOST_PP_OVERLOAD(JSON_VARNAME, __VA_ARGS__)(__VA_ARGS__),BOOST_PP_EMPTY())
#endif

#define JSON_VARNAME1(VARNAME)  \
    VARNAME

#define JSON_VARNAME2(VARNAME, IGNORED)  \
    JSON_VARNAME1(VARNAME)

//////////////////////////////////////////////////
#define JSON_REFERENCE_MEMBER(s, CLASS_NAME, VARDATA)       \
    constexpr const Member<                                 \
        decltype(&CLASS_NAME:: JSON_VARNAME VARDATA),       \
        &CLASS_NAME:: JSON_VARNAME VARDATA                  \
    > MembersHolder<CLASS_NAME>:: JSON_MEMBER_NAME VARDATA; \

};

#endif
