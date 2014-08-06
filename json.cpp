#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#define WIDEN(STRING)   \
    L ## STRING


#define JSON_CLASS(FQN, CLASS_DEF)                                                              \
    namespace JSON {                                                                            \
        constexpr const wchar_t* __##FQN = WIDEN(#CLASS_DEF);                                   \
        constexpr const size_t __##FQN##_size(sizeof (WIDEN(#CLASS_DEF)) / sizeof(wchar_t));    \
    }                                                                                           \
    class FQN CLASS_DEF

namespace JSON {

//the decorator
#define __json
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
            return JSONTokenMatcherPart<JSONTokenGetter<offset + numCharsInTestWord, testchars...>::GetTokenAtOffset() == JSONTokenGetter<offset, testchars...>::GetTokenAtOffset(),
                                        sizeof...(testchars) == numCharsInTestWord + offset + 1,
                                        testedCount + 1 == numCharsInTestWord,
                                        testedCount + 1, offset + 1, numCharsInTestWord,
                                        testchars...
                                       >::MatchToken();
        }
    };

    //This is the fail case, no need to continue
    template<bool stopTesting, bool testPass, size_t testedCount, size_t offset, size_t numCharsInTestWord, wchar_t... testchars>
    struct JSONTokenMatcherPart<false, stopTesting, testPass, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //The string isn't long enough
    template<bool test, bool testPass, size_t testedCount, size_t offset, size_t numCharsInTestWord, wchar_t... testchars>
    struct JSONTokenMatcherPart<test, true, testPass, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return false;
        }
    };

    //YAY! A Match!
    template<bool test, bool stopTesting, size_t testedCount, size_t offset, size_t numCharsInTestWord, wchar_t... testchars>
    struct JSONTokenMatcherPart<test, stopTesting, true, testedCount, offset, numCharsInTestWord, testchars...> {
        static constexpr bool MatchToken() {
            return true;
        }
    };

    //Also a match
    template<bool test, size_t testedCount, size_t offset, size_t numCharsInTestWord, wchar_t... testchars>
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
    template<const wchar_t *const *class_info, size_t info_len, unsigned int offset, bool inRange>
    struct JSONTagMatcher {
        static constexpr bool MatchJSONVarTag() {
            return JSONTokenMatcherStart<6,
                                         L'_', L'_', L'j', L's', L'o', L'n',
                                         class_info[0][offset],
                                         class_info[0][offset + 1],
                                         class_info[0][offset + 2],
                                         class_info[0][offset + 3],
                                         class_info[0][offset + 4],
                                         class_info[0][offset + 5]
                                    >::MatchToken();
        }
    };

    //This keeps us from going out of bounds while searching the array
    template<const wchar_t *const *class_info, size_t info_len, unsigned int offset>
    struct JSONTagMatcher<class_info, info_len, offset, false> {
        static constexpr bool MatchJSONVarTag() {
            return false;
        }
    };

    //This recursively searches the input string until it finds a matching token
    //  or fails to find one
    template<const wchar_t *const *class_info, size_t info_len, unsigned int offset, bool foundToken, bool recurse>
    struct JSONClassParserTokenFinder {
        static constexpr const wchar_t* FindJSONToken() {
            return JSONClassParserTokenFinder<class_info,
                                              info_len,
                                              offset + 1,
                                              JSONTagMatcher<class_info,
                                                             info_len,
                                                             offset,
                                                             (offset + 1) < info_len
                                                            >::MatchJSONVarTag(),
                                               (offset + DECORATOR_LEN) < info_len             
                                            >::FindJSONToken();
        }
    };

    //This is the termination case when a recursive search finds a token
    template<const wchar_t *const *class_info, size_t info_len, unsigned int offset, bool recurse>
    struct JSONClassParserTokenFinder<class_info, info_len, offset, true, recurse> {
        static constexpr const wchar_t* FindJSONToken() {
            //The offset is increased before we know if we found the end, so we
            //  need to decrease it here to get the real offset
            return class_info[0] + offset - 1;
        }
    };

    //This handles the recursive search failing to locate a token 
    template<const wchar_t *const *class_info, size_t info_len, unsigned int offset, bool foundToken>
    struct JSONClassParserTokenFinder<class_info, info_len, offset, foundToken, false> {
        static constexpr const wchar_t* FindJSONToken() {
            return nullptr;
        }
    };

    template<const wchar_t *const *class_info, size_t info_len>
    struct JSONClassParser {
        static constexpr const wchar_t* FindNextJSONToken() {
            return JSONClassParserTokenFinder<class_info, info_len, 0, false, true>::FindJSONToken();
        }
    };

    template<const wchar_t *const *class_info, size_t info_len>
    struct JSONParser {
        static void FromJSON() {
            // bool t = JSONTagMatcher<class_info, info_len, 0, true>::MatchJSONVarTag();
            // std::wcout << t << std::endl;
            std::wcout << *class_info << std::endl;
            const wchar_t* first = JSONClassParser<class_info, info_len>::FindNextJSONToken();
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

JSON_CLASS(Test, {
public:
     __json int abc;

     __json int xyz;

};);

int main() {

    JSON::JSONParser<&JSON::__Test, JSON::__Test_size>::FromJSON();

    return 0;
}
