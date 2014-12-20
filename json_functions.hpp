#pragma once
#ifndef __JSON_FUNCTIONS_HPP__
#define __JSON_FUNCTIONS_HPP__

#include <string>

namespace JSON {

   using jsonIter = const char*;

    template<typename ClassFor>
    void ToJSON(const ClassFor& classFrom, std::string& out);

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
