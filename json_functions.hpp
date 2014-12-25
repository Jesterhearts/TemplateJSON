#pragma once
#ifndef __JSON_FUNCTIONS_HPP__
#define __JSON_FUNCTIONS_HPP__

#include <string>

namespace tjson {
    namespace detail {
        struct Stringbuf;
    }

    using jsonIter = const char*;

    template<typename ClassType>
    void to_json(const ClassType& classFrom, detail::Stringbuf& out);

    template<typename ClassType>
    std::string to_json(const ClassType& classFrom);

    template<typename ClassType>
    ClassType from_json(const std::string& jsonString);

    template<typename ClassType>
    jsonIter from_json(jsonIter iter, ClassType* into);

    template<typename ClassType>
    struct MembersHolder;

    template<typename memberType, memberType member>
    struct MemberInfo;
}

#endif
