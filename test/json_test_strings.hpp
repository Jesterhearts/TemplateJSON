#pragma once
#ifndef __JSON_TEST_STRINGS_HPP__
#define __JSON_TEST_STRINGS_HPP__

#include <string>

void TestStrings();

struct strings {
    strings(std::string s, std::wstring ws, std::string us, std::wstring uws)
        :   string(s), wstring(ws), unicode(us), wunicode(uws)
    {}

    strings()
        :   string("A basic string"),
            wstring(L"A basic wstring"),
            unicode("嗢"),
            wunicode(L"嗢")
    {}

    std::string string;
    std::wstring wstring;
    std::string unicode;
    std::wstring wunicode;
};

struct escaped_strings {
    escaped_strings(std::string quotes, std::string escaped, std::string both, std::string newline)
        :   quotes(quotes),
            escaped(escaped),
            both(both),
            newline(newline)
    {}

    escaped_strings()
        :   quotes("abc\""),
            escaped("\\\\"),
            both("\"\\\\\""),
            newline("\n")
    {}

    std::string quotes;
    std::string escaped;
    std::string both;
    std::string newline;
};

struct chars {
    chars(char c, wchar_t wc) : c(c), wc(wc) {}

    chars() : c('c'), wc(L'w') {}

    char c;
    wchar_t wc;

    //TODO
    // char16_t c16;
    // char32_t c32;
};

#endif
