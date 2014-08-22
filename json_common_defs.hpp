#pragma once
#ifndef __JSON_COMMON_DEFS_HPP__
#define __JSON_COMMON_DEFS_HPP__

#include "json_functions.hpp"

#ifndef _MSC_VER
#define json_finline [[gnu::always_inline]] inline
#define json_no_return [[noreturn]]
#else
#define json_finline __forceinline
#define json_no_return __declspec (noreturn)
#endif

#ifndef JSON_NO_WARN_CONST
    #ifndef _MSC_VER
        #define json_deserialize_const_warning  \
            [[gnu::deprecated("Cannot deserialize into a const value!")]]
    #else
        #define json_deserialize_const_warning  \
            __declspec(deprecated("Cannot deserialize into a const value!"))
    #endif
#else
    #define json_deserialize_const_warning
#endif


namespace JSON {
    template<typename T> struct JSONFnInvoker;
    template<typename T> struct JSONFnInvokerImpl;
    template<typename T, size_t N = 1> struct JSONArrayHandler;
    template<typename T, bool AM, bool IP, bool AR, bool C = false> struct JSONFnInvokerDecider;
}

#endif
