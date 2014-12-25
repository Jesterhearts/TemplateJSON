#pragma once
#ifndef __JSON_NUMBER_PARSERS_HPP__
#define __JSON_NUMBER_PARSERS_HPP__

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <limits>

namespace tjson {
namespace detail {
    template<size_t bytes> constexpr uint8_t max_string_length();
    template<> //127, 255
    constexpr uint8_t max_string_length<1>() { return 3; };
    template<> //32767, 65535
    constexpr uint8_t max_string_length<2>() { return 5; };
    template<> //2147483647, 4294967295
    constexpr uint8_t max_string_length<4>() { return 10; };
    template<> //9223372036854775807, 18446744073709551615
    constexpr uint8_t max_string_length<8>() { return 20; };

    template<typename Type, Type base>
    json_finline void itoa(Type value, detail::Stringbuf& out) {
        static_assert(base > 1 && base <= 10, "Unsupported base");
        static_assert(std::is_integral<Type>::value, "Must be an integral type");
        const auto BufferSize = max_string_length<sizeof(Type)>() + std::is_signed<Type>::value;

        if(!value) {
            out.push_back('0');
            return;
        }

        char str[BufferSize];
        char* to = str + BufferSize - 1;

        const bool negative = std::is_signed<Type>::value && value < 0;
        const bool min_signed = std::is_signed<Type>::value && value == std::numeric_limits<Type>::min();

        if(min_signed) {
            value += 1;
        }

        if(negative) {
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
    json_finline void to_json(ClassType from, detail::Stringbuf& out) {
        out.append(boost::lexical_cast<std::string>(from));
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_integral> = true>
    json_finline void to_json(ClassType from, detail::Stringbuf& out) {
        itoa<ClassType, 10>(from, out);
    }

    template<>
    json_finline void to_json<bool, true>(bool from, detail::Stringbuf& out) {
        out.append(from ? "true" : "false", from ? 4 : 5);
    }

    template<typename Type>
    json_finline jsonIter atoi(jsonIter iter, Type& into) {
        static_assert(std::is_integral<Type>::value, "Must be an integral value");

        iter = advance_past_whitespace(iter);

        if(!std::is_signed<Type>::value && *iter == '-') {
            json_parsing_error(iter, "Not a valid integral number");
        }

        const Type sign = (std::is_signed<Type>::value && *iter == '-') ? -1 : 1;
        if(*iter == '-' || *iter == '+') {
            ++iter;
        }

        bool offByOne = false;

        for(into = 0; std::isdigit(*iter); ++iter) {
            if(into > std::numeric_limits<Type>::max() / 10) {
                json_parsing_error(iter, "Value will overflow");
            }
            into *= 10;

            Type temp = (*iter) - '0';
            if(sign != -1 && temp > std::numeric_limits<Type>::max() - into) {
                json_parsing_error(iter, "Value will overflow");
            }

            if(sign == -1 && temp > std::numeric_limits<Type>::max() - into) {
                if(temp > std::numeric_limits<Type>::max() - into + 1) {
                    json_parsing_error(iter, "Value will overflow");
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
    json_finline jsonIter from_json(jsonIter iter, ClassType& into) {
        return atoi(iter, into);
    }

    template<>
    json_finline jsonIter from_json<bool, true>(jsonIter iter, bool& into) {
        iter = advance_past_whitespace(iter);

        if(memcmp("true", iter, 4) == 0) {
            into = true;
            return iter + 4;
        }

        if(memcmp("false", iter, 5) == 0) {
            into = false;
            return iter + 5;
        }

        json_parsing_error(iter, "Could not convert to number");
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_floating_point> = true>
    json_finline jsonIter from_json(jsonIter iter, ClassType& into) {
        iter = advance_past_whitespace(iter);
        auto endOfNumber = find_end_of_number(iter);

        try {
            into = boost::lexical_cast<ClassType>(iter, std::distance(iter, endOfNumber));
        } catch(boost::bad_lexical_cast& e) {
            json_parsing_error(iter, e.what());
        }

        return endOfNumber;
    }
}
}
#endif
