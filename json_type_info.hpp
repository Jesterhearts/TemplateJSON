#pragma once
#ifndef __JSON_TYPE_INFO_HPP__
#define __JSON_TYPE_INFO_HPP__

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
    struct _TypeInfo;

    template<bool is_class, bool is_enum, bool is_arithmetic, bool is_pointer,
             bool is_array>
    struct _TypeInfo<true, is_class, is_enum, is_arithmetic, is_pointer, is_array> {
        typedef _const type;
    };

    template<>
    struct _TypeInfo<false, true, false, false, false, false> {
        typedef _class type;
    };

    template<>
    struct _TypeInfo<false, false, true, false, false, false> {
        typedef _enum type;
    };

    template<>
    struct _TypeInfo<false, false, false, true, false, false> {
        typedef _arithmetic type;
    };

    template<>
    struct _TypeInfo<false, false, false, false, true, false> {
        typedef _pointer type;
    };

    template<>
    struct _TypeInfo<false, false, false, false, false, true> {
        typedef _array type;
    };

    template<typename ClassType>
    struct TypeInfo {
        typedef typename std::remove_reference<ClassType>::type basetype;
        typedef typename _TypeInfo<
            std::is_const<basetype>::value,
            std::is_class<basetype>::value,
            std::is_enum<basetype>::value,
            std::is_arithmetic<basetype>::value,
            std::is_pointer<basetype>::value,
            std::is_array<basetype>::value
        >::type type;
    };
} /* detail */
} /* JSON */
#endif
