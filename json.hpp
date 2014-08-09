#pragma once
#ifndef __JSON_HPP_
#define __JSON_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "json_value_parser.hpp"

/* Helper macros */
#define WIDEN(STRING)   \
    L ## STRING

#define EXPAND_MACRO_WIDEN_IMPL(MACRO)  \
    WIDEN(#MACRO)

#define EXPAND_MACRO_WIDEN(MACRO) \
    EXPAND_MACRO_WIDEN_IMPL(MACRO)

#define MAKE_UNIQUE_VAL_IMPL(VARNAME, NUMBER)   \
    VARNAME ##_## NUMBER

#define MAKE_UNIQUE_VAL_HELPER(VARNAME, NUMBER) \
    MAKE_UNIQUE_VAL_IMPL(VARNAME, NUMBER)

#define MAKE_UNIQUE_VAL(VARNAME)    \
    MAKE_UNIQUE_VAL_HELPER(VARNAME, __COUNTER__)

/* Don't put this anywhere in your class definition unless you like reading horrific template
   errors */
#define JSON_VAR_DECORATOR \
    static constexpr JSON::__json_var

/* constructor macros */
#define JSON_CLASS_IMPL(CLASS_NAME, ...)                                                        \
    class CLASS_NAME {                                                                          \
        typedef CLASS_NAME __THIS_JSON_CLASS__;                                                 \
        /* inject the stuff that we need to function. It shouldn't be public */                 \
        /* These need to be able to hit functions even if private */                            \
        template <class ThisClass, const wchar_t *const *classString, size_t offset>            \
        friend struct JSON::VartoJSONFnInvoker;                                                 \
                                                                                                \
        template <class ThisClass>                                                              \
        friend struct JSON::TypetoJSONFnInvoker;                                                \
                                                                                                \
        /* This gives us the string for the class that we parse at compile time */              \
        static constexpr const wchar_t* __##CLASS_NAME = WIDEN(#__VA_ARGS__);                   \
                                                                                                \
    public:                                                                                     \
        std::wstring ToJSON() const {                                                           \
            return JSON::ClassJSONFnsInvoker<CLASS_NAME, &__##CLASS_NAME>                       \
                            ::InvokeToJSONFns(*this);                                           \
        }                                                                                       \
    private:                                                                                    \
        /* Here we actually make the rest of the class for them */                              \
        __VA_ARGS__                                                                             \
    }                                                                                           \

#define JSON_CLASS_HELPER(...)  \
    JSON_CLASS_IMPL(__VA_ARGS__)

/* Do indirection so macros in the class body get invoked properly */
#define JSON_CLASS(CLASS_NAME, ...)  \
    JSON_CLASS_HELPER(CLASS_NAME, __VA_ARGS__)

#define JSON_VAR_PREAMBLE(TYPE, VARNAME, JSONKEY, KEY, ...)  \
    TYPE VARNAME __VA_ARGS__;                                                               \
    /* This is the thing our tokenizer looks for, KEY gives it the ID for the               \
       specialization.                                                                      \
     */                                                                                     \
    JSON_VAR_DECORATOR KEY = EXPAND_MACRO_WIDEN(KEY);                                       \
    /* prevent duplicate keys */                                                            \
    static constexpr void* __KEYARG_##JSONKEY = nullptr;                                    \


#define JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, KEY, ...)                                     \
    JSON_VAR_PREAMBLE(TYPE, VARNAME, JSONKEY, KEY, __VA_ARGS__)                             \
                                                                                            \
    /* This function does the actual work */                                                \
    static void VarToJSON(const __THIS_JSON_CLASS__& classFrom,                             \
                          std::wstring& jsonData,                                           \
                          const JSON::VarToJSONIdentifier<                                  \
                                    JSON::VarNameHasher<&KEY>::Hash()>&& key) {             \
        jsonData += L"\"" #JSONKEY L"\":";                                                  \
        jsonData += JSON::TypetoJSONFnInvokerHelper<TYPE>                                   \
                        ::ToJSON(static_cast<TYPE>(classFrom.VARNAME));                     \
    }                                                                                       \

/* This makes sure our JSON key (KEY_ARG) for hashing and searching are the same */
#define JSON_VAR_HELPER(TYPE, VARNAME, JSONKEY, KEY_ARG, ...)    \
    JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, KEY_ARG, __VA_ARGS__)

/* Make a variable. The varargs will become the the value the variable is initialized to, if
   specified
 */
#define JSON_VAR(TYPE, VARNAME, JSONKEY, ...)    \
    JSON_VAR_HELPER(TYPE, VARNAME, JSONKEY, MAKE_UNIQUE_VAL(VARNAME), __VA_ARGS__)

/* holy shit so many templates */
namespace JSON {
    /* Important constants used in parsing */

    //The decorator
    //  - typedef lets us use it in class defs and still find the unique string
    typedef const wchar_t* __json_var;

    typedef std::unordered_map<std::wstring, std::wstring> DataMap;
    typedef std::pair<std::wstring, std::wstring> DataType;

    //This is used to match against when searching the stringy class.
    // It's defined in terms of what we use to make the identifier, so hopefully
    //  it doesn't break everytime we change things
    constexpr const wchar_t* DECORATOR_STR = EXPAND_MACRO_WIDEN(JSON_VAR_DECORATOR);
    constexpr const size_t DECORATOR_STR_LEN = sizeof(EXPAND_MACRO_WIDEN(JSON_VAR_DECORATOR)) / sizeof(wchar_t);

    /* Helpers for the template programs */
    template<unsigned int uniqueID>
    struct VarToJSONIdentifier {
        const static unsigned int help = uniqueID;
    };

    template<wchar_t testChar,
             wchar_t candidateChar>
    struct TokenTester {
        static constexpr bool Equal() {
            return testChar == candidateChar;
        }

        static constexpr bool NotEqual() {
            return testChar != candidateChar;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONIsNullOrWhitespace implementation
////
    template<wchar_t testChar>
    struct IsNullOrWhitespace {
        static constexpr bool MatchToken() {
            /* The specializations do the check for valid chars
               so if it hits this it can't be
             */
            return false;
        }
    };

    template<>
    struct IsNullOrWhitespace<L' '> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    template<>
    struct IsNullOrWhitespace<L'\t'> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    template<>
    struct IsNullOrWhitespace<L'\0'> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

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
// JSONTokenMatcherPart implementation
////
    //This case handles successful comparisons and recurses
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset = 0,
             bool matchFailed = false,
             bool endCandidateWord = false,
             bool endTestWord = false>
    struct TokenMatcherPart { 
        static constexpr bool MatchToken() {
            return TokenMatcherPart<candidateWord,
                                    testWord,
                                    candidateOffset + 1,
                                    testOffset + 1,
                                    TokenTester<testWord[0][testOffset],
                                                candidateWord[0][candidateOffset]
                                               >::NotEqual(),
                                    TokenTester<candidateWord[0][candidateOffset + 1],
                                                L'\0'
                                               >::Equal(),
                                    TokenTester<testWord[0][testOffset + 1],
                                                L'\0'
                                               >::Equal()
                                   >::MatchToken();
        }
    };

    //This is the fail case, no need to continue
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool endCandidateWord,
             bool endTestWord>
    struct TokenMatcherPart<candidateWord,
                            testWord,
                            candidateOffset,
                            testOffset,
                            /* matchFailed */ true,
                            endCandidateWord,
                            endTestWord> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //The string isn't long enough
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool matchFailed,
             bool endTestWord>
    struct TokenMatcherPart<candidateWord,
                            testWord,
                            candidateOffset,
                            testOffset,
                            matchFailed,
                            /* endCandidateWord */ true,
                            endTestWord> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //Also not long enough, also it didn't match
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool endTestWord>
    struct TokenMatcherPart<candidateWord,
                            testWord,
                            candidateOffset,
                            testOffset,
                            /* matchFailed */ true,
                            /* endCandidateWord */ true,
                            endTestWord> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //YAY! A Match!
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool matchFailed,
             bool endCandidateWord>
    struct TokenMatcherPart<candidateWord,
                            testWord,
                            candidateOffset,
                            testOffset,
                            matchFailed,
                            endCandidateWord,
                            /* endTestWord */ true> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //Also a match, but full length
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset,
             bool matchFailed>
    struct TokenMatcherPart<candidateWord,
                            testWord,
                            candidateOffset,
                            testOffset,
                            matchFailed,
                            /* endCandidateWord */ true,
                            /* endTestWord */ true> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// JSONTagMatcher implementation
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

////////////////////////////////////////////////////////////////////////////////
// JSONVartoJSONFnInvoker implementation
////
    template<typename classOn,
             const wchar_t *const *classString,
             size_t offset>
    struct VartoJSONFnInvoker {
        static void ToJSON(const classOn& classFrom, std::wstring& jsonData) {
            constexpr unsigned int id = VarNameHasher<classString, offset>::Hash();
            classOn::VarToJSON(classFrom, jsonData, JSON::VarToJSONIdentifier<id>());
        }
    };

////////////////////////////////////////////////////////////////////////////////
// ClassJSONFnsBuilder implementation
////
    template<typename ClassFor,
             const wchar_t *const *classInfo,
             size_t offset = 0,
             bool first = false,
             bool started = false,
             bool noMoreVars = false>
    struct ClassJSONFnsBuilder {
        static void BuildFns (const ClassFor& classOn, std::wstring& jsonData) {
            constexpr size_t tokenEnd = ClassParserTokenFinder<classInfo, offset>::FindJSONToken();
            ClassJSONFnsBuilder<ClassFor,
                                classInfo,
                                tokenEnd,
                                true,
                                true,
                                tokenEnd == 0
                               >::BuildFns(classOn, jsonData);
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo,
             size_t offset,
             bool first>
    struct ClassJSONFnsBuilder<ClassFor,
                               classInfo,
                               offset,
                               first,
                               /* started */ true,
                               /* noMoreVars */ false> {
        static void BuildFns (const ClassFor& classOn, std::wstring& jsonData) {
            if(!first) {
                jsonData += L",";
            }
            /* Do the function call */
            constexpr unsigned int varKey = VarNameHasher<classInfo, offset>::Hash();
            VartoJSONFnInvoker<ClassFor, classInfo, offset>::ToJSON(classOn, jsonData);

            constexpr size_t tokenEnd = ClassParserTokenFinder<classInfo, offset>::FindJSONToken();
            /* Try to do more calls */
            ClassJSONFnsBuilder<ClassFor,
                                classInfo,
                                tokenEnd,
                                false,
                                true,
                                tokenEnd == 0
                               >::BuildFns(classOn, jsonData);
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo,
             size_t offset,
             bool first>
    struct ClassJSONFnsBuilder<ClassFor,
                               classInfo,
                               offset,
                               first,
                               /* started */ true,
                               /* noMoreVars */ true> {
        static void BuildFns (const ClassFor& classOn, std::wstring& jsonData) {
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo>
    struct ClassJSONFnsInvoker {
        static std::wstring InvokeToJSONFns(const ClassFor& classOn) {
            std::wstring jsonData(L"{");
            ClassJSONFnsBuilder<ClassFor, classInfo>::BuildFns(classOn, jsonData);
            jsonData += L"}";

            return jsonData;
        }
    };
}
#endif
