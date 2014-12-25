#pragma once
#ifndef __JSON_PRIVATE_ACCESS_HPP__
#define __JSON_PRIVATE_ACCESS_HPP__

#include "json_functions.hpp"

#define JSON_PRIVATE_ACCESS()                               \
    template<typename ClassType>                            \
    friend std::string tjson::to_json(const ClassType&);      \
                                                            \
    template<typename ClassType>                            \
    friend ClassType tjson::from_json(const std::string&);    \
                                                            \
    template<typename ClassType>                            \
    friend struct tjson::MembersHolder;                      \
                                                            \
    template<typename memberType, memberType member>        \
    friend struct tjson::MemberInfo;                         \
                                                            \
    template<typename ClassType>                            \
    friend tjson::jsonIter tjson::from_json(tjson::jsonIter, ClassType&);

#endif
