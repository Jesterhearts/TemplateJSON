#pragma once
#ifndef __JSON_PRIVATE_ACCESS_HPP__
#define __JSON_PRIVATE_ACCESS_HPP__

#include "json_functions.hpp"

#define JSON_PRIVATE_ACCESS()                               \
    template<typename ClassType>                            \
    friend struct tjson::detail::MembersHolder;             \
                                                            \
    template<typename memberType, memberType member>        \
    friend struct tjson::detail::MemberInfo;                        \

#endif
