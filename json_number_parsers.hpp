#pragma once
#ifndef __JSON_NUMBER_PARSERS_HPP__
#define __JSON_NUMBER_PARSERS_HPP__

#include <cstdlib>

namespace JSON {
namespace detail {
    template<typename Type, size_t MaxLexicalLength>
    json_finline void itoa(Type value, std::string& out) {
        const auto BufferSize = MaxLexicalLength + std::is_signed<Type>::value;
        char str[BufferSize];
        char* to = str + BufferSize - 1;
        const bool negative = value < 0;

        uint8_t count = 0;
        static_assert(MaxLexicalLength < std::numeric_limits<uint8_t>::max(),
                      "Lexical length exceeds count range");

        while(value) {
            *to = '0' + value % 10;
            value /= 10;
            --to;
            ++count;
        }

        if(std::is_signed<Type>::value && negative) {
            *to = '-';
            --to;
        }

        out.append(to + 1, count);
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_arithmetic> = true>
    json_finline void ToJSON(ClassType from, std::string& out) {
        std::string result = boost::lexical_cast<std::string>(from);
        out.append(result);
    }

    template<>
    json_finline void ToJSON<bool, true>(bool from, std::string& out) {
        out.append(from ? "true" : "false", from ? 4 : 5);
    }

    template<>
    json_finline void ToJSON<uint8_t, true>(uint8_t from, std::string& out) {
        //255
        itoa<uint8_t, 3>(from, out);
    }

    template<>
    json_finline void ToJSON<uint16_t, true>(uint16_t from, std::string& out) {
        //65535
        itoa<uint16_t, 5>(from, out);
    }

    template<>
    json_finline void ToJSON<uint32_t, true>(uint32_t from, std::string& out) {
        //4294967295
        itoa<uint32_t, 10>(from, out);
    }

    template<>
    json_finline void ToJSON<uint64_t, true>(uint64_t from, std::string& out) {
        //18446744073709551615
        itoa<uint64_t, 20>(from, out);
    }

#if UINTPTR_MAX == 0xffffffff
    template<>
    json_finline void ToJSON<unsigned long, true>(unsigned long from, std::string& out) {
        //18446744073709551615
        itoa<unsigned long, 20>(from, out);
    }
#endif

    template<>
    json_finline void ToJSON<int8_t, true>(int8_t from, std::string& out) {
        //127
        itoa<int8_t, 3>(from, out);
    }

    template<>
    json_finline void ToJSON<int16_t, true>(int16_t from, std::string& out) {
        //32767
        itoa<int16_t, 5>(from, out);
    }

    template<>
    json_finline void ToJSON<int32_t, true>(int32_t from, std::string& out) {
        //2147483647
        itoa<int32_t, 10>(from, out);
    }

    template<>
    json_finline void ToJSON<int64_t, true>(int64_t from, std::string& out) {
        //9223372036854775807
        itoa<int64_t, 19>(from, out);
    }

#if UINTPTR_MAX == 0xffffffff
    template<>
    json_finline void ToJSON<long, true>(long from, std::string& out) {
        //18446744073709551615
        itoa<long, 20>(from, out);
    }
#endif

    template<typename ClassType,
             enable_if<ClassType, std::is_arithmetic> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        iter = AdvancePastWhitespace(iter, end);
        auto endOfNumber = AdvancePastNumbers(iter, end);

        try {
            into = boost::lexical_cast<ClassType>(&*iter, std::distance(iter, endOfNumber));
        }
        catch(boost::bad_lexical_cast blc) {
            ThrowBadJSONError(iter, end, std::string("Could not convert to type ") + typeid(into).name());
        }

        return endOfNumber;
    }
}
}
#endif
