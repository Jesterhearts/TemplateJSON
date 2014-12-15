#pragma once
#ifndef __JSON_FUNCTIONS_HPP__
#define __JSON_FUNCTIONS_HPP__

#include <string>
#include <unordered_map>

namespace JSON {

    using jsonIter = std::string::const_iterator;

    template<typename ClassFor>
    std::string ToJSON(const ClassFor& classFrom);

    template<typename ClassFor>
    ClassFor FromJSON(const std::string& jsonString);

    template<typename ClassFor>
    jsonIter FromJSON(jsonIter iter, jsonIter end, ClassFor& into);

    template<typename classType>
    struct MembersHolder;
}

#endif
