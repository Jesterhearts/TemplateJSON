#pragma once
#ifndef __JSON_FUNCTIONS_HPP__
#define __JSON_FUNCTIONS_HPP__

#include <string>

namespace JSON {
    namespace detail {
        struct stringbuf;
    }

   using jsonIter = const char*;

    template<typename ClassFor>
    void ToJSON(const ClassFor& classFrom, detail::stringbuf& out);

    template<typename ClassFor>
    std::string ToJSON(const ClassFor& classFrom);

    template<typename ClassFor>
    ClassFor FromJSON(const std::string& jsonString);

    template<typename ClassFor>
    jsonIter FromJSON(jsonIter iter, ClassFor& into);

    template<typename classType>
    struct MembersHolder;

    template<typename memberType, memberType member>
    struct MemberInfo;
}

#endif
