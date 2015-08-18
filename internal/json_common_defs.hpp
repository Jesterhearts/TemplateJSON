#pragma once
#ifndef __JSON_COMMON_DEFS_HPP__
#define __JSON_COMMON_DEFS_HPP__

#include "json_functions.hpp"

#ifndef _MSC_VER
#define json_no_return              [[noreturn]]
#define json_force_inline           [[gnu::always_inline]] inline
#define json_never_inline           [[gnu::noinline]]
#define json_cold_function          __attribute__((cold))
#define json_hot_function           __attribute__((hot))
#define json_nonull_args            __attribute__((nonnull))
#define json_return_nonull          __attribute__((returns_nonnull))
#define json_expect_true(value)     __builtin_expect(value, 1)
#define json_expect_false(value)    __builtin_expect(value, 0)
#define json_pragma(p)              _Pragma(BOOST_PP_STRINGIZE(p))

#else
#define json_no_return              __declspec(noreturn)
#define json_force_inline           __forceinline
#define json_never_inline           __declspec(noinline)
#define json_cold_function
#define json_hot_function
#define json_nonull_args
#define json_return_nonull
#define json_expect_true(value)     value
#define json_expect_false(value)    value
#define json_pragma(p)              __pragma(p)

#endif

#endif
