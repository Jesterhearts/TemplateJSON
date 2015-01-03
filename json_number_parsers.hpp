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
    inline void itoa(Type value, detail::Stringbuf& out) {
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
             enable_if<ClassType, std::is_floating_point>>
    inline void to_json(ClassType from, detail::Stringbuf& out) {
        out.append(boost::lexical_cast<std::string>(from));
    }

    template<typename ClassType,
             enable_if<ClassType, is_numeric>>
    inline void to_json(ClassType from, detail::Stringbuf& out) {
        itoa<ClassType, 10>(from, out);
    }

    template<typename ClassType,
             enable_if<ClassType, is_bool>>
    inline void to_json(ClassType from, detail::Stringbuf& out) {
        static_assert(std::is_same<ClassType, bool>::value, "error in template declaration.");

        out.append(from ? "true" : "false", from ? 4 : 5);
    }

    template<typename Type>
    inline void atoi(Tokenizer& tokenizer, DataMember<Type>& into) {
        static_assert(std::is_integral<Type>::value, "Must be an integral value");

        char sign_c = tokenizer.seek();

        if(!std::is_signed<Type>::value && sign_c == '-') {
            tokenizer.parsing_error("Not a valid integral number");
        }

        const Type sign = (std::is_signed<Type>::value && sign_c == '-') ? -1 : 1;
        tokenizer.advance_if_either<'-', '+'>();

        bool offByOne = false;
        Type value;

        const char* position = tokenizer.position();

        for(value = 0; isdigit(*position); ++position) {
            if(value > std::numeric_limits<Type>::max() / 10) {
                tokenizer.parsing_error("Value will overflow");
            }
            value *= 10;

            Type temp = (*position) - '0';
            if(sign != -1 && temp > std::numeric_limits<Type>::max() - value) {
                tokenizer.parsing_error("Value will overflow");
            }

            if(sign == -1 && temp > std::numeric_limits<Type>::max() - value) {
                if(temp > std::numeric_limits<Type>::max() - value + 1) {
                    tokenizer.parsing_error("Value will overflow");
                }
                offByOne = true;
                temp -= 1;
            }

            value += temp;
        }

        value *= sign;
        if(offByOne) {
            value -= 1;
        }
        into.write(value);

        tokenizer.skip(std::distance(tokenizer.position(), position));
    }

    template<typename ClassType,
             enable_if<ClassType, is_numeric>>
    inline void from_json(Tokenizer& tokenizer, DataMember<ClassType>& into) {
        return atoi(tokenizer, into);
    }

    template<typename ClassType,
             enable_if<ClassType, is_bool>>
    inline void from_json(Tokenizer& tokenizer, DataMember<ClassType>& into) {
        static_assert(std::is_same<ClassType, bool>::value, "error in template declaration.");

        tokenizer.seek();

        if(memcmp("true", tokenizer.position(), 4) == 0) {
            into.write(true);
            tokenizer.skip(4);
            return;
        }

        if(memcmp("false", tokenizer.position(), 5) == 0) {
            into.write(false);
            tokenizer.skip(5);
            return;
        }

        tokenizer.parsing_error("Could not read bool");
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_floating_point>>
    inline void from_json(Tokenizer& tokenizer, DataMember<ClassType>& into) {
        std::pair<const char*, size_t> startAndLength = tokenizer.consume_number();

        try {
            into.write(boost::lexical_cast<ClassType>(startAndLength.first, startAndLength.second));
        } catch(boost::bad_lexical_cast& e) {
            tokenizer.parsing_error(e.what());
        }
    }
}
}
#endif
