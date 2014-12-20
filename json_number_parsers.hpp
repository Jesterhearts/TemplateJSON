#pragma once
#ifndef __JSON_NUMBER_PARSERS_HPP__
#define __JSON_NUMBER_PARSERS_HPP__

#include <boost/lexical_cast.hpp>

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

    template<typename Type, uint8_t base>
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
        if(negative) {
            value *= -1;
        }

        while(value) {
            *to = '0' + value % base;
            value /= base;
            --to;
        }

        if(negative) {
            *to = '-';
            --to;
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

    template<typename ClassType,
             enable_if<ClassType, std::is_integral> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        iter = AdvancePastWhitespace(iter, end);
        auto endOfNumber = AdvancePastNumbers(iter, end);
        std::istringstream is(iter);

        if(!(is >> into))
        {
            ThrowBadJSONError(iter, end, "Could not convert string to value ");
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
        iter = AdvancePastWhitespace(iter, end);
        auto endOfNumber = AdvancePastNumbers(iter, end);

        try {
            into = boost::lexical_cast<ClassType>(&*iter, std::distance(iter, endOfNumber));
        }
        catch(const boost::bad_lexical_cast& blc) {
            ThrowBadJSONError(iter, end, blc.what());
        }

        return endOfNumber;
    }
}
}
#endif
