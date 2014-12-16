#pragma once
#ifndef __JSON_TYPE_Tag_HPP__
#define __JSON_TYPE_Tag_HPP__

namespace JSON {
namespace detail {
    struct _const       {};
    struct _class       {};
    struct _enum        {};
    struct _arithmetic  {};
    struct _pointer     {};
    struct _array       {};

    template<bool is_const, bool is_class, bool is_enum, bool is_arithmetic, bool is_pointer,
             bool is_array>
    struct _TypeTag;

    template<bool is_class, bool is_enum, bool is_arithmetic, bool is_pointer,
             bool is_array>
    struct _TypeTag<true, is_class, is_enum, is_arithmetic, is_pointer, is_array> {
        typedef _const tag;
    };

    template<>
    struct _TypeTag<false, true, false, false, false, false> {
        typedef _class tag;
    };

    template<>
    struct _TypeTag<false, false, true, false, false, false> {
        typedef _enum tag;
    };

    template<>
    struct _TypeTag<false, false, false, true, false, false> {
        typedef _arithmetic tag;
    };

    template<>
    struct _TypeTag<false, false, false, false, true, false> {
        typedef _pointer tag;
    };

    template<>
    struct _TypeTag<false, false, false, false, false, true> {
        typedef _array tag;
    };

    template<typename ClassType>
    struct TypeTag {
        typedef typename std::remove_reference<ClassType>::type basetype;
        typedef typename _TypeTag<
            std::is_const<basetype>::value,
            std::is_class<basetype>::value,
            std::is_enum<basetype>::value,
            std::is_arithmetic<basetype>::value,
            std::is_pointer<basetype>::value,
            std::is_array<basetype>::value
        >::tag tag;
    };
} /* detail */
} /* JSON */
#endif
