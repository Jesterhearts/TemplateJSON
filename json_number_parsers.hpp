#pragma once
#ifndef __JSON_NUMBER_PARSERS_HPP__
#define __JSON_NUMBER_PARSERS_HPP__

#include <limits>
#include <cstdlib>
#include <cstring>
#include <cerrno>

namespace JSON {
namespace detail {
    template<size_t bytes> constexpr uint8_t MaxIntegerStringLength();
    template<> //127, 255
    constexpr uint8_t MaxIntegerStringLength<1>() { return 3; };
    template<> //32767, 65535
    constexpr uint8_t MaxIntegerStringLength<2>() { return 5; };
    template<> //2147483647, 4294967295
    constexpr uint8_t MaxIntegerStringLength<4>() { return 10; };
    template<> //9223372036854775807, 18446744073709551615
    constexpr uint8_t MaxIntegerStringLength<8>() { return 20; };

    template<typename Type, uint8_t base>
    json_finline void itoa(Type value, std::string& out) {
        static_assert(base > 1 && base <= 10, "Unsupported base");

        const auto BufferSize = MaxIntegerStringLength<sizeof(Type)>() + std::is_signed<Type>::value;

        char str[BufferSize];
        char* to = str + BufferSize - 1;
        const bool negative = value < 0;

        while(value) {
            *to = '0' + value % base;
            value /= base;
            --to;
        }

        if(std::is_signed<Type>::value && negative) {
            *to = '-';
            --to;
        }

        ++to;
        out.append(to, BufferSize - std::abs(to - str));
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_floating_point> = true>
    json_finline void ToJSON(ClassType from, std::string& out) {
        out << from;
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_integral> = true>
    json_finline void ToJSON(ClassType from, std::string& out) {
        itoa<ClassType, 10>(from, out);
    }

    template<>
    json_finline void ToJSON<bool, true>(bool from, std::string& out) {
        out.append(from ? "true" : "false", from ? 4 : 5);
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_integral> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        char* endOfNumber;

        if(std::is_signed<ClassType>::value) {
            int64_t result = strtoll(iter, &endOfNumber, 10);
            if(result > std::numeric_limits<ClassType>::max() || result < std::numeric_limits<ClassType>::min()) {
                ThrowBadJSONError(iter, end, "Could not convert to number");
            }
        }
        else {
            uint64_t result = strtoull(iter, &endOfNumber, 10);
            if(result > std::numeric_limits<ClassType>::max()) {
                ThrowBadJSONError(iter, end, "Could not convert to number");
            }
        }

        if(iter == endOfNumber || errno == ERANGE) {
            ThrowBadJSONError(iter, end, "Could not convert to number");
        }

        return endOfNumber;
    }

    template<>
    json_finline jsonIter FromJSON<bool, true>(jsonIter iter, jsonIter end, bool& into) {
        iter = AdvancePastWhitespace(iter, end);

        if(strcmp("true", iter) == 0) {
            into = true;
            return iter + 4;
        }

        if(strcmp("false", iter) == 0) {
            into = false;
            return iter + 5;
        }

        ThrowBadJSONError(iter, end, "Could not convert to number");
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_floating_point> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        char* endOfNumber;
        double result = strtod(iter, &endOfNumber);

        if(iter == endOfNumber || errno == ERANGE || result > std::numeric_limits<ClassType>::max()
            || result < std::numeric_limits<ClassType>::min()) {
            ThrowBadJSONError(iter, end, "Could not convert to number");
        }

        return endOfNumber;
    }
}
}
#endif
