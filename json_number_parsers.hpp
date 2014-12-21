#pragma once
#ifndef __JSON_NUMBER_PARSERS_HPP__
#define __JSON_NUMBER_PARSERS_HPP__

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

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

    template<typename Type, Type base>
    json_finline void itoa(Type value, std::string& out) {
        static_assert(base > 1 && base <= 10, "Unsupported base");
        static_assert(std::is_integral<Type>::value, "Must be an integral type");
        const auto BufferSize = MaxIntegerStringLength<sizeof(Type)>() + std::is_signed<Type>::value;

        if(!value) {
            out.append(1, '0');
            return;
        }

        char str[BufferSize];
        char* to = str + BufferSize - 1;

        const bool negative = std::is_signed<Type>::value && value < 0;
        const bool min_signed = std::is_signed<Type>::value && value == std::numeric_limits<Type>::min();

        if(negative && min_signed) {
            value += 1;
            value *= -1;
        }

        while(value) {
            *to = '0' + (value % base);
            value /= base;
            --to;
        }

        if(negative) {
            *to = '-';
            --to;

            if(min_signed) {
                str[BufferSize - 1] += 1;
            }
        }

        ++to;
        out.append(to, BufferSize - std::abs(to - str));
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_floating_point> = true>
    json_finline void ToJSON(ClassType from, std::string& out) {
        out.append(boost::lexical_cast<std::string>(from));
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

    template<typename Type>
    json_finline jsonIter atoi(jsonIter iter, jsonIter end, Type& into) {
        static_assert(std::is_integral<Type>::value, "Must be an integral value");

        iter = AdvancePastWhitespace(iter, end);
        if(iter == end) {
            return iter;
        }

        const size_t remain = std::distance(iter, end);
        const uint8_t maxLen = MaxIntegerStringLength<sizeof(Type)>() + std::is_signed<Type>::value;
        const jsonIter lastPossible = iter + std::min(remain, static_cast<size_t>(maxLen));

        if(!std::is_signed<Type>::value && *iter == '-') {
            ThrowBadJSONError(iter, end, "Not a valid integral number");
        }

        const Type sign = (std::is_signed<Type>::value && *iter == '-') ? -1 : 1;
        if(*iter == '-' || *iter == '+') {
            ++iter;
        }

        bool offByOne = false;

        for(into = 0; iter != lastPossible && std::isdigit(*iter); ++iter) {
            if(into > std::numeric_limits<Type>::max() / 10) {
                ThrowBadJSONError(iter, end, "Value will overflow");
            }
            into *= 10;

            Type temp = (*iter) - '0';
            if(sign != -1 && temp > std::numeric_limits<Type>::max() - into) {
                ThrowBadJSONError(iter, end, "Value will overflow");
            }

            if(sign == -1 && temp > std::numeric_limits<Type>::max() - into) {
                if(temp > std::numeric_limits<Type>::max() - into + 1) {
                    ThrowBadJSONError(iter, end, "Value will overflow");
                }
                offByOne = true;
                temp -= 1;
            }

            into += temp;
        }

        into *= sign;
        if(offByOne) {
            into -= 1;
        }

        return iter;
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_integral> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        return atoi(iter, end, into);
    }

    template<>
    json_finline jsonIter FromJSON<bool, true>(jsonIter iter, jsonIter end, bool& into) {
        iter = AdvancePastWhitespace(iter, end);

        if(memcmp("true", iter, 4) == 0) {
            into = true;
            return iter + 4;
        }

        if(memcmp("false", iter, 5) == 0) {
            into = false;
            return iter + 5;
        }

        ThrowBadJSONError(iter, end, "Could not convert to number");
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_floating_point> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        iter = AdvancePastWhitespace(iter, end);
        auto endOfNumber = AdvancePastNumbers(iter, end);

        try {
            into = boost::lexical_cast<ClassType>(iter, std::distance(iter, endOfNumber));
        } catch(boost::bad_lexical_cast& e) {
            ThrowBadJSONError(iter, endOfNumber, e.what());
        }

        return endOfNumber;
    }
}
}
#endif
