#pragma once
#ifndef __JSON_COMMON_DEFS_HPP__
#define __JSON_COMMON_DEFS_HPP__

#include "json_functions.hpp"

#ifndef _MSC_VER
#define json_no_return      [[noreturn]]
#define json_force_inline   [[gnu::always_inline]] inline
#define json_never_inline   [[gnu::noinline]]

#else
#define json_no_return      __declspec(noreturn)
#define json_force_inline   __forceinline

    #if _MSC_VER < 1900
    #define constexpr
    #endif
#endif

#endif
