#pragma once
#ifndef __JSON_PRIVATE_ACCESS_HPP__
#define __JSON_PRIVATE_ACCESS_HPP__

#include "json_functions.hpp"

#define JSON_PRIVATE_ACCESS()                               \
    template<typename ClassType>                            \
    friend std::string JSON::ToJSON(const ClassType&);      \
                                                            \
    template<typename ClassType>                            \
    friend ClassType JSON::FromJSON(const std::string&);    \
                                                            \
    template<typename ClassType>                            \
    friend struct JSON::MembersHolder;                      \
                                                            \
    template<typename memberType, memberType member>        \
    friend struct JSON::MemberInfo;                         \
                                                            \
    template<typename ClassType>                            \
    friend JSON::jsonIter JSON::FromJSON(JSON::jsonIter, ClassType&);

#endif
