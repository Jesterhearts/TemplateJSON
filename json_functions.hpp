#pragma once
#ifndef __JSON_FUNCTIONS_HPP__
#define __JSON_FUNCTIONS_HPP__

#include <string>

namespace JSON {
    namespace detail {
        struct stringbuf;
    }

   using jsonIter = const char*;

    template<typename ClassType>
    void ToJSON(const ClassType& classFrom, detail::stringbuf& out);

    template<typename ClassType>
    std::string ToJSON(const ClassType& classFrom);

    template<typename ClassType>
    ClassType FromJSON(const std::string& jsonString);

    template<typename ClassType>
    jsonIter FromJSON(jsonIter iter, ClassType& into);

    template<typename ClassType>
    struct MembersHolder;

    template<typename memberType, memberType member>
    struct MemberInfo;
}

#endif
