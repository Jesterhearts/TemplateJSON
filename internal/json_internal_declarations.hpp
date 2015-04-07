#pragma once
#ifndef __JSON_INTERNAL_DECLARATIONS__
#define __JSON_INTERNAL_DECLARATIONS__

#include <type_traits>

namespace tjson {
namespace detail {
    struct reference_only {
        reference_only() = delete;
        ~reference_only() = delete;
    };

    enum struct placement_new {
        invoke
    };

    template<typename ClassType>
    using raw_data = typename
        std::aligned_storage<sizeof(ClassType), std::alignment_of<ClassType>::value>::type;

    struct data_emplace_store_tag;
    struct data_internal_store_tag;
    struct Tokenizer;

    template<typename StoredType>
    struct DataMember;

    template<typename ClassType>
    using non_const = typename std::remove_const<ClassType>::type;

    template<typename ClassType>
    using basic_type = non_const<typename std::remove_reference<ClassType>::type>;

    template<typename ClassType, template<typename C> class decider>
    using enable_if = typename std::enable_if<decider<basic_type<ClassType>>::value, bool>::type;

#ifndef _MSC_VER
    #define JSON_ENABLE_IF(TEMPLATE_ARG, DECIDER) enable_if<TEMPLATE_ARG, DECIDER>
#else
    #define JSON_ENABLE_IF(TEMPLATE_ARG, DECIDER)   \
        typename std::enable_if<DECIDER<basic_type<TEMPLATE_ARG>>::value, bool>::type
#endif

    template<typename ClassType>
    using is_bool = std::is_same<ClassType, bool>;

    template<typename ClassType>
    using is_char = std::is_same<ClassType, char>;

    template<typename ClassType>
    using is_wchar = std::is_same<ClassType, wchar_t>;

    template<typename ClassType>
    struct is_numeric {
        constexpr static const bool value = !is_bool<ClassType>::value
                                            && !is_char<ClassType>::value
                                            && !is_wchar<ClassType>::value
                                            && std::is_integral<ClassType>::value;
    };

    //Enum parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_enum) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_enum) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Number parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_numeric) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_numeric) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Boolean parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_bool) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_bool) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Character parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_char) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_char) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Wide Character parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_wchar) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, is_wchar) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Floating point number parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_floating_point) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_floating_point) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Pointer parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_pointer) = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_pointer) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

    //Class parsers
    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_class) = true>
    inline void to_json(const ClassType& from, detail::Stringbuf& out);

    template<typename ClassType, JSON_ENABLE_IF(ClassType, std::is_class) = true>
    inline void from_json(Tokenizer& iter, DataMember<ClassType>& into);

} /* detail */
} /* tjson */
#endif
