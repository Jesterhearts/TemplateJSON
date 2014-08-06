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
    /* The space helps stop us from matching substrings */                          \
    /* The _ before the ID lets people end their vars with numbers */               \
    static constexpr const wchar_t* __##VARNAME = L" " WIDEN(#VARNAME)              \
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

// TODO: this is no longer used, but it's useful for testing stuff right now
// until things are complete
    constexpr const wchar_t* DECORATOR_STR = L" __json";

    constexpr const wchar_t nullchar = L'\0';

    template<wchar_t testChar, wchar_t candidateChar>
    struct JSONTokenTester {
      static constexpr bool Equal() {
          return testChar == candidateChar;
      }
    };

    //This case handles successful comparisons and recurses
    template<bool testResult,
             bool stopTesting,
             bool testComplete,
             const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t testOffset,
             size_t candidateOffset>
    struct JSONTokenMatcherPart { 
        static constexpr bool MatchToken() {
            return JSONTokenMatcherPart<JSONTokenTester<testWord[0][testOffset],
                                                        candidateWord[0][candidateOffset]
                                                       >::Equal(),
                                        JSONTokenTester<candidateWord[0][candidateOffset + 1],
                                                        nullchar
                                                       >::Equal(),
                                        JSONTokenTester<testWord[0][testOffset + 1],
                                                        nullchar
                                                       >::Equal(),
                                        testWord,
                                        candidateWord,
                                        testOffset + 1,
                                        candidateOffset + 1
                                       >::MatchToken();
        }
    };

    //This is the fail case, no need to continue
    template<bool stopTesting,
             bool testComplete,
             const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t testOffset,
             size_t candidateOffset>
    struct JSONTokenMatcherPart</* testResult */ false,
                                stopTesting,
                                testComplete, 
                                testWord,
                                candidateWord,
                                testOffset,
                                candidateOffset
                               > {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //The string isn't long enough
    template<bool testResult,
             bool testComplete,
             const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t testOffset,
             size_t candidateOffset>
    struct JSONTokenMatcherPart<testResult,
                                /* stopTesting */ true,
                                testComplete,
                                testWord,
                                candidateWord,
                                testOffset,
                                candidateOffset
                               > {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //Also not long enough, also it didn't match
    template<bool testComplete,
             const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t testOffset,
             size_t candidateOffset>
    struct JSONTokenMatcherPart</* testResult */ false,
                                /* stopTesting */ true,
                                testComplete,
                                testWord,
                                candidateWord,
                                testOffset,
                                candidateOffset
                               > {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //YAY! A Match!
    template<bool testResult,
             bool stopTesting,
             const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t testOffset,
             size_t candidateOffset>
    struct JSONTokenMatcherPart<testResult,
                                stopTesting,
                                /* testComplete */ true,
                                testWord,
                                candidateWord,
                                testOffset,
                                candidateOffset
                               > {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //Also a match, but full length
    template<bool testResult,
             const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t testOffset,
             size_t candidateOffset>
    struct JSONTokenMatcherPart<testResult,
                                /* stopTesting */ true,
                                /* testComplete */ true,
                                testWord,
                                candidateWord,
                                testOffset,
                                candidateOffset
                               > {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //This is just a convenience class for starting the tokenization
    template<const wchar_t* const* testWord,
             const wchar_t* const* candidateWord,
             size_t candidateOffset>
    struct JSONTokenMatcherStart {
        static constexpr bool MatchToken() {
            return JSONTokenMatcherPart</* testResult - false would kill it early */ true,
                                        /* stopTesting */ false,
                                        /* testComplete */ false,
                                        testWord,
                                        candidateWord,
                                        /* testOffset */ 0,
                                        candidateOffset
                                       >::MatchToken();
        }
    };

    //This class checks if a string at an offset matches the decorator
    template<const wchar_t *const *classInfo, unsigned int offset>
    struct JSONTagMatcher {
        static constexpr bool MatchJSONVarTag() {
            return JSONTokenMatcherStart<&DECORATOR_STR,
                                         classInfo,
                                         offset
                                        >::MatchToken();
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
                                                             offset
                                                            >::MatchJSONVarTag(),
                                              classInfo[0][offset + 1] != L'\0'
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
