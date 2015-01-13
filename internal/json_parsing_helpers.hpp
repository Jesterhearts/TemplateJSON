#pragma once
#ifndef __JSON_PARSING_HELPERS_HPP__
#define __JSON_PARSING_HELPERS_HPP__

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

#include "json_internal_declarations.hpp"
#include "json_tokenizer.hpp"

namespace tjson {
namespace detail {

struct Stringbuf {
    Stringbuf() {
        m_size = 1024;
        m_buf = static_cast<char*>(std::malloc(m_size));
        index = m_buf;
    }

    ~Stringbuf() {
        std::free(m_buf);
    }

    void push_back(char c) {
        allocate(1);
        *(index++) = c;
    }

    void append(const char* str, size_t len) {
        allocate(len);
        std::memcpy(index, str, len);
        index += len;
    }

    void append(const std::string& str) {
        append(str.c_str(), str.length());
    }

    std::string to_string() {
        return std::string(m_buf, std::distance(m_buf, index));
    }

private:
    char* index;
    char* m_buf;
    size_t m_size;

    void allocate(size_t size) {
        const size_t index_offset = std::distance(m_buf, index);
        size_t newsize = index_offset + size;
        if(m_size < newsize) {
            newsize *= 1.2;
            char* new_mem = static_cast<char*>(std::realloc(m_buf, newsize));
            if (!new_mem) {
                throw std::bad_alloc();
            }
            m_buf = new_mem;
            index = m_buf + index_offset;
            m_size = newsize;
        }
    }
};

} /* detail */
} /* tjson */
#endif
