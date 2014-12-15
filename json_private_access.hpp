#pragma once
#ifndef __JSON_PRIVATE_ACCESS_HPP__
#define __JSON_PRIVATE_ACCESS_HPP__

#include "json_functions.hpp"

#define JSON_PRIVATE_ACCESS()                               \
    template<typename ClassFor>                             \
    friend std::string JSON::ToJSON(const ClassFor&);       \
                                                            \
    template<typename ClassFor>                             \
    friend ClassFor JSON::FromJSON(const std::string&);     \
                                                            \
    template<typename ClassFor>                             \
    friend struct JSON::MembersHolder;                      \
                                                            \
    template<typename ClassFor>                             \
    friend JSON::jsonIter JSON::FromJSON(JSON::jsonIter, JSON::jsonIter, ClassFor&);

#endif
