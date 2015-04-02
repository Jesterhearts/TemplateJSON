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

    struct data_emplace_store_tag;
    struct data_internal_store_tag;
    struct Tokenizer;

    template<typename StoredType>
    struct DataMemberBase;

    template<typename ClassType>
    using non_const = typename std::remove_const<ClassType>::type;

    template<typename ClassType>
    using basic_type = non_const<typename std::remove_reference<ClassType>::type>;

    template<typename ClassType, template<typename C> class decider>
    using enable_if = typename std::enable_if<decider<basic_type<ClassType>>::value, bool>::type;

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
    template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Number parsers
    template<typename ClassType, enable_if<ClassType, is_numeric> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, is_numeric> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Boolean parsers
    template<typename ClassType, enable_if<ClassType, is_bool> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, is_bool> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Character parsers
    template<typename ClassType, enable_if<ClassType, is_char> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, is_char> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Wide Character parsers
    template<typename ClassType, enable_if<ClassType, is_wchar> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, is_wchar> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Floating point number parsers
    template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Pointer parsers
    template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
    inline void to_json(ClassType from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

    //Class parsers
    template<typename ClassType, enable_if<ClassType, std::is_class> = true>
    inline void to_json(const ClassType& from, detail::Stringbuf& out);

    template<typename ClassType, enable_if<ClassType, std::is_class> = true>
    inline void from_json(Tokenizer& iter, DataMemberBase<ClassType>& into);

} /* detail */
} /* tjson */
#endif
