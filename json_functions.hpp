#pragma once
#ifndef __JSON_FUNCTIONS_HPP__
#define __JSON_FUNCTIONS_HPP__

#include <string>
#include <unordered_map>

namespace JSON {

#ifdef JSON_USE_WIDE_STRINGS
#define JSON_ST(STRING) L ## STRING
    typedef std::wstring stringt;
    typedef wchar_t char_t;
#else
#define JSON_ST(STRING) STRING
    typedef std::string stringt;
    typedef char char_t;
#endif

    typedef stringt::const_iterator jsonIter;

    template<typename ClassFor>
    stringt ToJSON(const ClassFor& classFrom);

    template<typename ClassFor>
    ClassFor FromJSON(const stringt& jsonString);

    template<typename ClassFor>
    jsonIter FromJSON(jsonIter iter, jsonIter end, ClassFor& into);

    template<typename classType>
    struct ClassMap;

    template<typename classType>
    struct MembersHolder;
}

#endif
