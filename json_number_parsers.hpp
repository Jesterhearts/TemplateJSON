#pragma once
#ifndef __JSON_NUMBER_PARSERS_HPP__
#define __JSON_NUMBER_PARSERS_HPP__

#include <cstdlib>

namespace JSON {
namespace detail {
    template<typename Type, size_t MaxLexicalLength>
    json_finline std::string itoa(Type value) {
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

        return std::string(to + 1, count);
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_arithmetic> = true>
    json_finline std::string ToJSON(const ClassType& from) {
        return boost::lexical_cast<std::string>(from);
    }

    template<>
    json_finline std::string ToJSON<bool, true>(const bool& from) {
        return from ? "true" : "false";
    }

    template<>
    json_finline std::string ToJSON<uint8_t, true>(const uint8_t& from) {
        //255
        return itoa<uint8_t, 3>(from);
    }

    template<>
    json_finline std::string ToJSON<uint16_t, true>(const uint16_t& from) {
        //65535
        return itoa<uint16_t, 5>(from);
    }

    template<>
    json_finline std::string ToJSON<uint32_t, true>(const uint32_t& from) {
        //4294967295
        return itoa<uint32_t, 10>(from);
    }

    template<>
    json_finline std::string ToJSON<uint64_t, true>(const uint64_t& from) {
        //18446744073709551615
        return itoa<uint64_t, 20>(from);
    }

    template<>
    json_finline std::string ToJSON<unsigned long, true>(const unsigned long& from) {
        //18446744073709551615
        return itoa<unsigned long, 20>(from);
    }

    template<>
    json_finline std::string ToJSON<int8_t, true>(const int8_t& from) {
        //127
        return itoa<int8_t, 3>(from);
    }

    template<>
    json_finline std::string ToJSON<int16_t, true>(const int16_t& from) {
        //32767
        return itoa<int16_t, 5>(from);
    }

    template<>
    json_finline std::string ToJSON<int32_t, true>(const int32_t& from) {
        //2147483647
        return itoa<int32_t, 10>(from);
    }

    template<>
    json_finline std::string ToJSON<int64_t, true>(const int64_t& from) {
        //9223372036854775807
        return itoa<int64_t, 19>(from);
    }

    template<>
    json_finline std::string ToJSON<long, true>(const long& from) {
        //18446744073709551615
        return itoa<long, 20>(from);
    }

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
