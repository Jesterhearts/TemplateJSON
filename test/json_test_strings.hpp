#pragma once
#ifndef __JSON_TEST_STRINGS_HPP__
#define __JSON_TEST_STRINGS_HPP__

#include <string>

void TestStrings();

struct strings {
    strings()
        :   string("A basic string"),
            wstring(L"A basic wstring")
    {}

    std::string string;
    std::wstring wstring;

    //TODO test escaping
};

struct chars {
    chars(): c('c'), wc(L'w') {}

    char c;
    wchar_t wc;

    //TODO
    // char16_t c16;
    // char32_t c32;
};

#endif
