#pragma once
#ifndef __JSON_TOKENIZER_HPP__
#define __JSON_TOKENIZER_HPP__

#include <cstring>
#include <string>

namespace tjson {
namespace detail {

//UTF8 todo
struct Tokenizer {
    Tokenizer(const std::string& string) :
        current(string.data()),
        end(string.data() + string.length()) {}

    json_force_inline
    char peek() {
        return *current;
    }

    json_force_inline
    char seek() {
        advance_while_whitespace();
        return peek();
    }

    json_force_inline
    char take() {
        return *(current++);
    }

    json_force_inline
    void skip(size_t count) {
        current += count;
    }

    json_force_inline
    const char* position() {
        return current;
    }

    template<char value>
    void advance_past() {
        advance_while_whitespace();
        advance_if<value>();
    }

    template<char value>
    void advance_past_or_fail_if_not(const char* message) {
        advance_while_whitespace();
        advance_or_fail_if_not<value>(message);
    }

    template<char value>
    json_force_inline
    void advance_if() {
        if(peek() == value) {
            ++current;
        }
    }

    template<char value1, char value2>
    json_force_inline
    bool advance_if_either() {
        if(peek() == value1 || peek() == value2) {
            ++current;
            return true;
        }

        return false;
    }

    template<char value>
    json_force_inline
    void advance_or_fail_if_not(const char* message) {
        if(take() != value) {
            parsing_error(message);
        }
    }

    struct UnescapedString {
        const char* const data;
        const size_t length;

        json_force_inline UnescapedString(const char* data, size_t length, bool shared) :
            data(data),
            length(length),
            shared(shared)
        {}

        json_force_inline UnescapedString(UnescapedString&& other) :
            data(other.data),
            length(other.length),
            shared(other.shared)
        {
            other.shared = true;
        }

        json_force_inline ~UnescapedString() {
            if(!shared)
                std::free(const_cast<char*>(data));
        }

    private:
        bool shared;
    };

    //return is the set (opening quote, closing quote)
    UnescapedString consume_string_token() {
        const char* string_start = consume_string_start();
        const char* string_end = consume_string_remainder();

        size_t length = std::distance(string_start, string_end);

        const char* backslash = static_cast<char*>(std::memchr(string_start, '\\', length));
        if(!backslash) {
            return {string_start, length, /* shared */true};
        }

        char* data = static_cast<char*>(std::malloc(length * sizeof(char)));
        std::memcpy(data, string_start, std::distance(string_start, backslash));

        char* write_i = data + std::distance(string_start, backslash);
        for(; backslash != (string_end - 1) && backslash != string_end;
            ++backslash, ++write_i)
        {
            if(*backslash != '\\') {
                *write_i = *backslash;
            }
            else {
                switch(*(backslash + 1)) {
                case '\\':
                    /* fallthru */
                case '\"':
                    /* fallthru */
                case '\'':
                    *write_i = *(backslash + 1);
                    break;

                case 'b':
                    *write_i = '\b';
                    break;
                case 'f':
                    *write_i = '\f';
                    break;
                case 'n':
                    *write_i = '\n';
                    break;
                case 'r':
                    *write_i = '\r';
                    break;
                case 't':
                    *write_i = '\t';
                    break;
                default:
                    *write_i = *backslash;
                    *write_i = *(backslash + 1);
                }
                ++backslash;
            }
        }

        if(backslash != string_end) {
            *write_i = *backslash;
            ++write_i;
        }

        UnescapedString result {
            data,
            static_cast<size_t>(std::distance(data, write_i)),
            /* shared */false
        };
        return result;
    }

    json_force_inline
    void consume_kv_mapping() {
        advance_past_or_fail_if_not<':'>("Missing : in k-v mapping");
    }

    json_force_inline
    void consume_object_start() {
        advance_past_or_fail_if_not<'{'>("Missing { for object start");
    }

    json_force_inline
    void consume_object_end() {
        advance_past_or_fail_if_not<'}'>("Missing } for object end");
    }

    json_force_inline
    void consume_array_start() {
        advance_past_or_fail_if_not<'['>("Missing [ for array start");
    }

    std::pair<const char*, size_t> consume_number() {
        advance_while_whitespace();

        const char* start = current;

        advance_if<'-'>();
        advance_while<&isdigit>();

        advance_if<'.'>();
        advance_while<&isdigit>();

        if(advance_if_either<'e', 'E'>()) {
            advance_if_either<'-', '+'>();
        }

        advance_while<&isdigit>();

        return std::make_pair(start, std::distance(start, current));
    }

    json_never_inline
    json_no_return
    void parsing_error(const char* description) json_cold_function {
        size_t context_length = std::min<size_t>(std::distance(current, end), 1000);

        std::string error_message(description);
        error_message.append(": ");
        error_message.append(current, context_length);

        throw std::invalid_argument(error_message);
    }

    json_never_inline
    json_no_return
    void parsing_error(std::string&& message) json_cold_function {
        parsing_error(message.c_str());
    }

private:
    const char* current;
    const char* end;

    void advance_while_whitespace() {
        while (true) {
            switch(peek()) {
            case ' ':
                /* fallthru */
            case '\n':
                /* fallthru */
            case '\r':
                /* fallthru */
            case '\t':
                /* fallthru */
            case '\v':
                ++current;
                continue;
            }
            break;
        }
    }

    template<int (test)(int)>
    void advance_while() {
        while(test(peek())) {
            ++current;
        }
    }

    json_force_inline
    const char* consume_string_start() {
        advance_past_or_fail_if_not<'\"'>("Missing opening \" for string");
        return current;
    }

    const char* consume_string_remainder() {
        const char* string_end = static_cast<const char*>(std::memchr(current, '\"', std::distance(current, end)));
        if(!string_end) {
            parsing_error("Missing closing \" for string");
        }

        current = string_end + 1;

        const char* escape_index = string_end - 1;
        bool escaped = false;

        while(json_expect_false(*escape_index == '\\')) {
            escaped = !escaped;
            --escape_index;
        }

        if(json_expect_false(escaped)) {
            return consume_string_remainder();
        }

        return string_end;
    }

    Tokenizer(const Tokenizer&) = delete;
    Tokenizer& operator=(const Tokenizer&) = delete;
};

}
}
#endif
