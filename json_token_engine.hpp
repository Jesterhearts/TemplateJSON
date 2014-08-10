#pragma once
#ifndef __JSON_TOKEN_ENGINE_HPP__
#define __JSON_TOKEN_ENGINE_HPP__

/**
 * Contains compile-time programs for doing string testing and comparison
 */

namespace JSON {
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
}
#endif
