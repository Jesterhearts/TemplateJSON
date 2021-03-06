#pragma once
#ifndef __INTERNAL_JSON_FUNCTIONS_HPP__
#define __INTERNAL_JSON_FUNCTIONS_HPP__

#include <string>

#ifdef _MSC_VER
static_assert(_MSC_VER >= 1900, "Unsupported compiler");
#endif

namespace tjson {
    namespace detail {
        struct Stringbuf;
        struct Tokenizer;
        
        template<typename ClassType>
        struct MembersHolder;
        
        template<typename memberType, memberType member>
        struct MemberInfo;
    }

    template<typename ClassType>
    void to_json(const ClassType& classFrom, detail::Stringbuf& out);

    template<typename ClassType>
    std::string to_json(const ClassType& classFrom);

    template<typename ClassType>
    ClassType from_json(const std::string& jsonString);
}

#endif
