#pragma once
#ifndef __JSON_COMMON_DEFS_HPP__
#define __JSON_COMMON_DEFS_HPP__

#include "json_functions.hpp"

#ifndef _MSC_VER
#define json_no_return [[noreturn]]
#else
#define json_no_return __declspec(noreturn)
#endif

#endif
