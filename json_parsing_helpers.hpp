#pragma once
#ifndef __JSON_PARSING_HELPERS_HPP__
#define __JSON_PARSING_HELPERS_HPP__

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

namespace JSON {
        namespace detail {
        template<typename ClassType>
        using basic_type = typename std::remove_reference<ClassType>::type;

        template<typename ClassType>
        using non_const = typename std::enable_if<!std::is_const<basic_type<ClassType>>::value, ClassType>::type;

        template<typename ClassType, template<typename C> class decider>
        using enable_if = typename std::enable_if<decider<non_const<ClassType>>::value, bool>::type;

        template<typename ClassType>
        using enable_if_const = typename std::enable_if<std::is_const<basic_type<ClassType>>::value, bool>::type;

        template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
        void ToJSON(ClassType from, detail::stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_integral> = true>
        void ToJSON(ClassType from, detail::stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_integral> = true>
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
        void ToJSON(ClassType from, detail::stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
        void ToJSON(ClassType from, detail::stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_array> = true>
        void ToJSON(const ClassType& from, detail::stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_array> = true>
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_class> = true>
        void ToJSON(const ClassType& from, detail::stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_class> = true>
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        template<typename ClassType, enable_if_const<ClassType> = true>
        json_deserialize_const_warning
        jsonIter FromJSON(jsonIter iter, ClassType& into);

        struct stringbuf {
            stringbuf() {
                m_size = 1024;
                m_buf = static_cast<char*>(std::malloc(m_size));
                index = m_buf;
            }

            ~stringbuf() {
                std::free(m_buf);
            }

            json_finline
            void push_back(char c) {
                allocate(1);
                *(index++) = c;
            }

            json_finline
            void append(const char* str, size_t len) {
                allocate(len);
                std::memcpy(index, str, len);
                index += len;
            }

            json_finline
            void append(const std::string& str) {
                append(str.c_str(), str.length());
            }

            json_finline
            std::string to_string() {
                return std::string(m_buf, std::distance(m_buf, index));
            }

        private:
            char* index;
            char* m_buf;
            size_t m_size;

            json_finline
            void allocate(size_t size) {
                size_t newsize = std::distance(m_buf, index) + size;
                if(m_size < newsize) {
                    newsize *= 1.2;
                    char* new_mem = static_cast<char*>(std::realloc(m_buf, newsize));
                    if (!new_mem) {
                        throw std::bad_alloc();
                    }
                    m_buf = new_mem;
                    m_size = newsize;
                }
            }
        };
    }

    json_no_return inline void ThrowBadJSONError(jsonIter iter, std::string&& errmsg) {
        jsonIter endIter = iter + std::min<size_t>(std::strlen(iter), 1000);
        std::string badJson(iter, endIter);
        throw std::invalid_argument(errmsg + ": " + badJson);
    }

    json_finline jsonIter AdvancePastWhitespace(jsonIter iter) {
        while(isspace(*iter)) {
            ++iter;
        }

        return iter;
    }

    json_finline jsonIter AdvancePastNumbers(jsonIter iter) {
        if(*iter == '-') {
            ++iter;
        }

        while(isdigit(*iter)) {
            ++iter;
        }

        if(*iter == '.') {
            ++iter;
        }

        while(isdigit(*iter)) {
            ++iter;
        }

        if(*iter == 'e' || *iter == 'E') {
            ++iter;
            if(*iter == '-' || *iter == '+') {
                ++iter;
            }
        }

        while(isdigit(*iter)) {
            ++iter;
        }

        return iter;
    }

    json_finline jsonIter AdvanceToEndOfString(jsonIter iter) {
        bool escaping = true;

        while(iter) {
            if(*iter == '\\' || escaping) {
                escaping = !escaping;
            }
            else if(!escaping && *iter == '\"') {
                return iter;
            }

            ++iter;
        }

        return iter;
    }

    json_finline jsonIter ValidateObjectStart(jsonIter iter) {
        iter = AdvancePastWhitespace(iter);
        if(*iter != '{') {
            ThrowBadJSONError(iter, "No object start token");
        }
        ++iter;

        return iter;
    }

    json_finline jsonIter ValidateObjectEnd(jsonIter iter) {
        iter = AdvancePastWhitespace(iter);
        if(*iter != '}') {
            ThrowBadJSONError(iter, "No object end token");
        }
        ++iter;
        return iter;
    }

    json_finline jsonIter ValidateKeyValueMapping(jsonIter iter) {
        iter = AdvancePastWhitespace(iter);
        if(*iter != ':') {
            ThrowBadJSONError(iter, "Not a valid key-value mapping");
        }
        ++iter;
        return iter;
    }

    /* Advance past start quote of key string */
    json_finline jsonIter FindStartOfKey(jsonIter iter) {
        iter = AdvancePastWhitespace(iter);
        if(*iter != '\"') {
            ThrowBadJSONError(iter, "Missing key");
        }
        return iter;
    }

    /* Advance to end quote of key string */
    json_finline jsonIter FindEndOfKey(jsonIter iter) {
        iter = AdvanceToEndOfString(iter);
        if(*iter != '\"') {
            ThrowBadJSONError(iter, "No close \" for key");
        }

        return iter + 1;
    }
}
#endif
