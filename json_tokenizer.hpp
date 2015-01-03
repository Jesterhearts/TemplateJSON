#pragma once
#ifndef __JSON_TOKENIZER_HPP__
#define __JSON_TOKENIZER_HPP__

#include <cstring>
#include <string>

namespace tjson {
namespace detail {

//UTF8 todo
struct Tokenizer {
    Tokenizer(const std::string& string)
        : current(string.data()), end(string.data() + string.length()) { }

    char peek() {
        return *current;
    }

    char seek() {
        advance_if<&isspace>();
        return peek();
    }

    char take() {
        return *(current++);
    }

    void skip(size_t count) {
        current += count;
    }

    const char* position() {
        return current;
    }

    template<char value>
    void advance_past() {
        advance_if<&isspace>();
        advance_if<value>();
    }

    template<char value>
    void advance_past_or_fail_if_not(const char* message) {
        advance_if<&isspace>();
        advance_or_fail_if_not<value>(message);
    }

    template<char value>
    void advance_if() {
        if(peek() == value) {
            ++current;
        }
    }

    template<char value1, char value2>
    bool advance_if_either() {
        if(peek() == value1 || peek() == value2) {
            ++current;
            return true;
        }

        return false;
    }

    template<char value>
    void advance_or_fail_if_not(const char* message) {
        if(take() != value) {
            parsing_error(message);
        }
    }

    //return is the set (opening quote, closing quote)
    std::pair<const char*, size_t> consume_string_token() {
        const char* string_start = consume_string_start();
        const char* string_end = consume_string_remainder();

        return std::make_pair(string_start, std::distance(string_start, string_end));
    }

    void consume_kv_mapping() {
        advance_past_or_fail_if_not<':'>("Missing : in k-v mapping");
    }

    void consume_object_start() {
        advance_past_or_fail_if_not<'{'>("Missing { for object start");
    }

    void consume_object_end() {
        advance_past_or_fail_if_not<'}'>("Missing } for object end");
    }

    void consume_array_start() {
        advance_past_or_fail_if_not<'['>("Missing [ for array start");
    }

    std::pair<const char*, size_t> consume_number() {
        advance_if<&isspace>();

        const char* start = current;

        advance_if<'-'>();
        advance_if<&isdigit>();

        advance_if<'.'>();
        advance_if<&isdigit>();

        if(advance_if_either<'e', 'E'>()) {
            advance_if_either<'-', '+'>();
        }

        advance_if<&isdigit>();

        return std::make_pair(start, std::distance(start, current));
    }

    json_no_return void parsing_error(const char* description) {
        size_t context_length = std::min<size_t>(std::distance(current, end), 1000);

        std::string error_message(description);
        error_message.append(": ");
        error_message.append(current, context_length);

        throw std::invalid_argument(error_message);
    }

    void parsing_error(std::string&& message) {
        parsing_error(message.c_str());
    }

private:
    const char* current;
    const char* end;

    template<int (test)(int)>
    void advance_if() {
        while(test(peek())) {
            ++current;
        }
    }

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

        while(*escape_index == '\\') {
            escaped = !escaped;
            --escape_index;
        }

        if(escaped) {
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
