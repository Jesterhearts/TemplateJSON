#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

/* Helper macros */
#define WIDEN(STRING)   \
    L ## STRING

#define JSON_COUNTERTOSTRIMPL(VALUE)    \
    WIDEN( #VALUE )

#define JSON_COUNTERTOSTR(VALUE)    \
    JSON_COUNTERTOSTRIMPL( VALUE )

/* constructor macros */
#define JSON_CLASS(CLASS_NAME, CLASS_BODY)                                                      \
    class CLASS_NAME {                                                                          \
        /* inject the stuff that we need to function */                                         \
        static constexpr const wchar_t* __##CLASS_NAME = WIDEN(#CLASS_BODY);                    \
        static constexpr const size_t __##CLASS_NAME##_size =                                   \
                                                sizeof (WIDEN(#CLASS_BODY)) / sizeof(wchar_t);  \
                                                                                                \
        template<class ThisClass, int uniqueID>                                                 \
        static void VarToJSON(ThisClass&, JSON::VarToJSONHelper<uniqueID>);                     \
        CLASS_BODY                                                                              \
    };

#define JSON_VAR(CLASS, TYPE, VARNAME)                                              \
    static constexpr const int __##VARNAME_id = __COUNTER__;                        \
    /* This is off by 1 from the id! */                                             \
    static constexpr const wchar_t* __##VARNAME = WIDEN(#VARNAME)                   \
                                                  L"_"                              \
                                                  JSON_COUNTERTOSTR( __COUNTER__ ); \
                                                                                    \
    template<int uniqueID>                                                          \
    static void __TYPE##_VarToJSONFunctionCreator(CLASS& classInto)   {             \
        CLASS::VarToJSON<CLASS>(classInto,                                          \
                                JSON::VarToJSONHelper<__##VARNAME_id>());           \
    }                                                                               \
                                                                                    \
    template<class ThisClass>                                                       \
    static void VarToJSON(ThisClass& classInto,                                     \
                          JSON::VarToJSONHelper<__##VARNAME_id> helper) {           \
                                                                                    \
    }                                                                               \
    TYPE VARNAME;

/* holy shit so many templates */
namespace JSON {

    template<int uniqueID>
    struct VarToJSONHelper {
        const static int help = uniqueID;
    };

//the decorator
// TODO: this is no longer used, but it's useful for testing stuff right now
// until things are complete
// #define __json
    constexpr const size_t DECORATOR_LEN = sizeof(L"__json") / sizeof(wchar_t) - 1;

    template<size_t offset, wchar_t token, wchar_t... tokens>
    struct JSONTokenGetter {
        static constexpr wchar_t GetTokenAtOffset() {
            return JSONTokenGetter<offset - 1, tokens...>::GetTokenAtOffset();
        }
    };

    template<wchar_t token, wchar_t... tokens>
    struct JSONTokenGetter<0, token, tokens...> {
        static constexpr wchar_t GetTokenAtOffset() {
            return token;
        }
    };

    //This case handles successful comparisons and recurses
    template<bool test,
             bool stopTesting,
             bool testPass,
             size_t testedCount, size_t offset, size_t numCharsInTestWord,
             wchar_t... testchars>
    struct JSONTokenMatcherPart { 
        static constexpr bool MatchToken() {
            return JSONTokenMatcherPart<JSONTokenGetter<
                                                        offset + numCharsInTestWord,
                                                        testchars...
                                                       >::GetTokenAtOffset() == JSONTokenGetter<
                                                                                                offset,
                                                                                                testchars...
                                                                                               >::GetTokenAtOffset(),
                                        sizeof...(testchars) == numCharsInTestWord + offset + 1,
                                        testedCount + 1 == numCharsInTestWord,
                                        testedCount + 1, offset + 1, numCharsInTestWord,
                                        testchars...
                                       >::MatchToken();
        }
    };

    //This is the fail case, no need to continue
    template<bool stopTesting, bool testPass,
             size_t testedCount, size_t offset, size_t numCharsInTestWord,
             wchar_t... testchars>
    struct JSONTokenMatcherPart<false, stopTesting, testPass, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //The string isn't long enough
    template<bool test, bool testPass,
             size_t testedCount, size_t offset, size_t numCharsInTestWord,
             wchar_t... testchars>
    struct JSONTokenMatcherPart<test, true, testPass, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //YAY! A Match!
    template<bool test, bool stopTesting,
             size_t testedCount, size_t offset, size_t numCharsInTestWord,
             wchar_t... testchars>
    struct JSONTokenMatcherPart<test, stopTesting, true, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //Also a match
    template<bool test,
            size_t testedCount, size_t offset, size_t numCharsInTestWord,
            wchar_t... testchars>
    struct JSONTokenMatcherPart<test, true, true, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //This is just a convenience class for starting the tokenization
    template<size_t charsInTestWord, wchar_t... testChars>
    struct JSONTokenMatcherStart {
        static constexpr bool MatchToken() {
            return JSONTokenMatcherPart<true, false, false, 0, 0, charsInTestWord, testChars...>::MatchToken();
        }
    };

    //This class checks if a string at an offset matches the decorator
    template<const wchar_t *const *classInfo, size_t infoLen, unsigned int offset, bool inRange>
    struct JSONTagMatcher {
        static constexpr bool MatchJSONVarTag() {
            return JSONTokenMatcherStart<6,
                                         L'_', L'_', L'j', L's', L'o', L'n',
                                         classInfo[0][offset],
                                         classInfo[0][offset + 1],
                                         classInfo[0][offset + 2],
                                         classInfo[0][offset + 3],
                                         classInfo[0][offset + 4],
                                         classInfo[0][offset + 5]
                                    >::MatchToken();
        }
    };

    //This keeps us from going out of bounds while searching the array
    template<const wchar_t *const *classInfo, size_t infoLen, unsigned int offset>
    struct JSONTagMatcher<classInfo, infoLen, offset, false> {
        static constexpr bool MatchJSONVarTag() {
            return false;
        }
    };

    //This recursively searches the input string until it finds a matching token
    //  or fails to find one
    template<const wchar_t *const *classInfo, size_t infoLen, unsigned int offset, bool foundToken, bool recurse>
    struct JSONClassParserTokenFinder {
        static constexpr const wchar_t* FindJSONToken() {
            return JSONClassParserTokenFinder<classInfo,
                                              infoLen,
                                              offset + 1,
                                              JSONTagMatcher<classInfo,
                                                             infoLen,
                                                             offset,
                                                             (offset + 1) < infoLen
                                                            >::MatchJSONVarTag(),
                                               (offset + DECORATOR_LEN) < infoLen             
                                            >::FindJSONToken();
        }
    };

    //This is the termination case when a recursive search finds a token
    template<const wchar_t *const *classInfo, size_t infoLen, unsigned int offset, bool recurse>
    struct JSONClassParserTokenFinder<classInfo, infoLen, offset, true, recurse> {
        static constexpr const wchar_t* FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return classInfo[0] + offset - 1;
        }
    };

    //Also a termination case
    template<const wchar_t *const *classInfo, size_t infoLen, unsigned int offset>
    struct JSONClassParserTokenFinder<classInfo, infoLen, offset, true, false> {
        static constexpr const wchar_t* FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return classInfo[0] + offset - 1;
        }
    };

    //This handles the recursive search failing to locate a token 
    template<const wchar_t *const *classInfo, size_t infoLen, unsigned int offset, bool foundToken>
    struct JSONClassParserTokenFinder<classInfo, infoLen, offset, foundToken, false> {
        static constexpr const wchar_t* FindJSONToken() {
            return nullptr;
        }
    };

    template<const wchar_t *const *classInfo, size_t infoLen>
    struct JSONClassParser {
        static constexpr const wchar_t* FindNextJSONToken() {
            return JSONClassParserTokenFinder<classInfo, infoLen, 0, false, true>::FindJSONToken();
        }
    };

    template<const wchar_t *const *classInfo, size_t infoLen>
    struct JSONParser {
        static void FromJSON() {
            // bool t = JSONTagMatcher<classInfo, infoLen, 0, true>::MatchJSONVarTag();
            // std::wcout << t << std::endl;
            std::wcout << *classInfo << std::endl;
            const wchar_t* first = JSONClassParser<classInfo, infoLen>::FindNextJSONToken();
            std::wcout << first << std::endl;
        }
    };

    template<class Type>
    struct JSONValuePODParser {
        static Type FromJSON(const std::wstring& json) {
            std::wstringstream wss;
            wss << json;

            Type value;
            wss >> value;

            if(wss.fail()) {
                throw std::invalid_argument("bad json value for key"); 
            }

            return value;
        }
    };
}

JSON_CLASS(Test, 
public:
     JSON_VAR(Test, int, __json);
);

int main() {

    JSON::JSONParser<&Test::__Test, Test::__Test_size>::FromJSON();

    return 0;
}
