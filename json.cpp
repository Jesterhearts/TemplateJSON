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
    static void __TYPE##_VarToJSONFunctionCreator(CLASS& classFrom)   {             \
        CLASS::VarToJSON<CLASS>(classFrom,                                          \
                                JSON::VarToJSONHelper<__##VARNAME_id>());           \
    }                                                                               \
                                                                                    \
    template<class ThisClass>                                                       \
    static void VarToJSON(ThisClass& classFrom,                                     \
                          JSON::VarToJSONHelper<__##VARNAME_id> helper) {           \
      /* stringmap.append = jsonValueParserForTYPE :: parse(classFrom.VARNAME) */   \
                                                                                    \
    }                                                                               \
    TYPE VARNAME;

/* holy shit so many templates */
namespace JSON {

    template<int uniqueID>
    struct VarToJSONHelper {
        const static int help = uniqueID;
    };

    constexpr const wchar_t* DECORATOR_STR = L"JSON_VAR(";

    constexpr const wchar_t nullchar = L'\0';

    template<wchar_t testChar, wchar_t candidateChar>
    struct JSONTokenTester {
        static constexpr bool Equal() {
            return testChar == candidateChar;
        }

        static constexpr bool NotEqual() {
            return testChar != candidateChar;
        }
    };

    template<const wchar_t* const* word,
             size_t offset,
             size_t eatenCount = 0,
             bool matchSpaceFailed = false,
             bool matchTabFailed = false>
    struct JSONWhitespaceCounter {
        static constexpr size_t Count() {
            return JSONWhitespaceCounter<word,
                                         offset + 1,
                                         eatenCount + 1,
                                         JSONTokenTester<word[0][offset],
                                                         L' '
                                                        >::NotEqual(),
                                         JSONTokenTester<word[0][offset],
                                                         L'\t'
                                                        >::NotEqual()
                                        >::Count();
        }
    };

    template<const wchar_t* const* word,
             size_t offset,
             size_t eatenCount>
    struct JSONWhitespaceCounter<word,
                               offset,
                               eatenCount,
                               true,
                               true> {
        static constexpr size_t Count() {
            return eatenCount - 1;
        }
    };


    //This case handles successful comparisons and recurses
    template<const wchar_t* const* candidateWord,
             const wchar_t* const* testWord,
             size_t candidateOffset,
             size_t testOffset = 0,
             bool matchFailed = false,
             bool endCandidateWord = false,
             bool endTestWord = false>
    struct JSONTokenMatcherPart { 
        static constexpr bool MatchToken() {
            return JSONTokenMatcherPart<candidateWord,
                                        testWord,
                                        candidateOffset + 1,
                                        testOffset + 1,
                                        JSONTokenTester<testWord[0][testOffset],
                                                        candidateWord[0][candidateOffset]
                                                       >::NotEqual(),
                                        JSONTokenTester<candidateWord[0][candidateOffset + 1],
                                                        nullchar
                                                       >::Equal(),
                                        JSONTokenTester<testWord[0][testOffset + 1],
                                                        nullchar
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
    struct JSONTokenMatcherPart<candidateWord,
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
    struct JSONTokenMatcherPart<candidateWord,
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
    struct JSONTokenMatcherPart<candidateWord,
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
    struct JSONTokenMatcherPart<candidateWord,
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
    struct JSONTokenMatcherPart<candidateWord,
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

    //This class checks if a string at an offset matches the decorator
    template<const wchar_t *const *classInfo, unsigned int offset>
    struct JSONTagMatcher {
        static constexpr bool MatchJSONVarTag() {
            return JSONTokenMatcherPart<classInfo,
                                        &DECORATOR_STR,
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
