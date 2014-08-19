#pragma once
#ifndef __JSON_COMMON_DEFS_HPP__
#define __JSON_COMMON_DEFS_HPP__

#include <string>
#include <unordered_map>

#include <boost/preprocessor/stringize.hpp>

#ifndef _MSC_VER
#define json_finline [[gnu::always_inline]] inline
#define json_no_return [[noreturn]]
#define json_deserialize_const_warning [[gnu::deprecated("Cannot deserialize into a const value!")]]
#else
#define json_finline __forceinline
#define json_no_return __declspec (noreturn)
#define json_deserialize_const_warning __declspec(deprecated("Cannot deserialize into a const value!"))
#endif

namespace JSON {
#ifdef JSON_USE_WIDE_STRINGS
#define JSON_ST(STRING) L ## STRING
    typedef std::wstring stringt;
#else
#define JSON_ST(STRING) STRING
    typedef std::string stringt;
#endif

    typedef std::unordered_map<stringt, void*> DataMap;
}

#endif
