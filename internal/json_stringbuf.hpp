#pragma once
#ifndef __JSON_STRINGBUF_HPP__
#define __JSON_STRINGBUF_HPP__

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

#include "json_internal_declarations.hpp"
#include "json_tokenizer.hpp"

namespace tjson {
namespace detail {

struct Stringbuf {
    json_force_inline Stringbuf() :
        m_size(1024),
        m_buf(static_cast<char*>(std::malloc(m_size))),
        index(m_buf)
    {}

    json_force_inline ~Stringbuf() {
        std::free(m_buf);
    }

    json_force_inline void push_back(char c) {
        allocate(1);
        *(index++) = c;
    }

    json_force_inline void append(const char* str, size_t len) json_nonull_args {
        allocate(len);
        std::memcpy(index, str, len);
        index += len;
    }

    json_force_inline void append(const std::string& str) {
        append(str.c_str(), str.length());
    }

    json_force_inline void append_and_escape(const std::string& str) {
        allocate(2 * str.length());
        size_t length = str.length();
        const char* chars = str.c_str();

        for(size_t i = 0; i < length; ++i) {
            char c = chars[i];
            if(c == '"' || c =='\\') {
                *(index++) = '\\';
            }
            *(index++) = c;
        }
    }

    std::string to_string() {
        return std::string(m_buf, std::distance(m_buf, index));
    }

private:
    size_t m_size;
    char* m_buf;
    char* index;

    json_force_inline void allocate(size_t size) {
        const size_t index_offset = std::distance(m_buf, index);
        size_t newsize = index_offset + size;
        if(json_expect_true(m_size >= newsize)) {
            return;
        }

        newsize *= 1.2;
        char* new_mem = static_cast<char*>(std::realloc(m_buf, newsize));
        if (json_expect_false(!new_mem)) {
            throw std::bad_alloc();
        }

        m_buf = new_mem;
        index = m_buf + index_offset;
        m_size = newsize;
    }
};

} /* detail */
} /* tjson */
#endif
