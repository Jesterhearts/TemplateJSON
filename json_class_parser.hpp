#pragma once
#ifndef __JSON_CLASS_PARSER_HPP__
#define __JSON_CLASS_PARSER_HPP__

/* Don't put this anywhere in your class definition unless you like reading horrific template
   errors */
#define JSON_VAR_DECORATOR \
    static constexpr JSON::__json_var

#include "json_common_macros.hpp"
#include "json_token_engine.hpp"

/**
 * Contains the compile time program for class processing
 */
namespace JSON {
    /* Important constants used in parsing */

    //The decorator
    //  - typedef lets us use it in class defs and still find the unique string
    typedef const wchar_t* __json_var;
    //This is used to match against when searching the stringy class.
    // It's defined in terms of what we use to make the identifier, so hopefully
    //  it doesn't break everytime we change things
    constexpr const wchar_t* DECORATOR_STR = EXPAND_MACRO_WIDEN(JSON_VAR_DECORATOR);
    constexpr const size_t DECORATOR_STR_LEN = sizeof(EXPAND_MACRO_WIDEN(JSON_VAR_DECORATOR)) / sizeof(wchar_t);

////////////////////////////////////////////////////////////////////////////////
// JSONVarNameHasher implementation
/////
    template<const wchar_t *const *string,
             size_t offset = 0,
             unsigned int hashValue = 0,
             bool endOfString = false>
    struct VarNameHasher {
        static constexpr unsigned int Hash() {
            return VarNameHasher<string,
                                 offset + 1,
                                 hashValue * 131 + string[0][offset],
                                 IsNullOrWhitespace<string[0][offset + 1]>::MatchToken()
                                >::Hash();
        }
    };

    template<const wchar_t *const *string,
             size_t offset,
             unsigned int hashValue>
    struct VarNameHasher<string,
                        offset,
                        hashValue,
                        /* endOfString */ true> {
        static constexpr unsigned int Hash() {
            return hashValue;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// TagMatcher implementation
////
    //This class checks if a string at an offset matches the decorator
    template<const wchar_t *const *classInfo,
             unsigned int offset>
    struct TagMatcher {
        static constexpr bool MatchJSONVarTag() {
            return TokenMatcherPart<classInfo,
                                    &DECORATOR_STR,
                                    offset
                                   >::MatchToken();
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONClassParserTokenFinder implementation
////
    //This recursively searches the input string until it finds a matching token
    //  or fails to find one
    //The return value is advanced past the match
    //0 means failure
    template<const wchar_t *const *classInfo,
             size_t offset = 0,
             bool foundToken = false,
             bool endOfInput = false>
    struct ClassParserTokenFinder {
        static constexpr const size_t FindJSONToken() {
            return ClassParserTokenFinder<classInfo,
                                          offset + 1,
                                          TagMatcher<classInfo,
                                                     offset
                                                    >::MatchJSONVarTag(),
                                          classInfo[0][offset + 1] == L'\0'
                                         >::FindJSONToken();
        }
    };

    //This is the termination case when a recursive search finds a token
    template<const wchar_t *const *classInfo,
             size_t offset,
             bool endOfInput>
    struct ClassParserTokenFinder<classInfo,
                                  offset,
                                  /* foundToken*/ true,
                                  endOfInput> {
        static constexpr size_t FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return offset - 1 + DECORATOR_STR_LEN;
        }
    };

    //Also a termination case
    template<const wchar_t *const *classInfo,
             size_t offset>
    struct ClassParserTokenFinder<classInfo,
                                  offset,
                                  /* foundToken*/ true,
                                  /* endOfInput */ false> {
        static constexpr size_t FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return offset - 1 + DECORATOR_STR_LEN;
        }
    };

    //This handles the recursive search failing to locate a token 
    template<const wchar_t *const *classInfo,
             size_t offset,
             bool foundToken>
    struct ClassParserTokenFinder<classInfo,
                                  offset,
                                  foundToken,
                                  /* endOfInput */ true> {
        static constexpr const size_t FindJSONToken() {
            return 0;
        }
    };
}

#endif
