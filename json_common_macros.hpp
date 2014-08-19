#pragma once
#ifndef __JSON_COMMON_MACROS_HPP__
#define __JSON_COMMON_MACROS_HPP__

#ifndef _MSC_VER
#define json_finline [[gnu::always_inline]] inline
#define json_no_return [[noreturn]]
#define json_deserialize_const_warning [[gnu::deprecated("Cannot deserialize into a const value!")]]
#else
#define json_finline __forceinline
#define json_no_return __declspec (noreturn)
#define json_deserialize_const_warning __declspec(deprecated("Cannot deserialize into a const value!"))
#endif

#endif
