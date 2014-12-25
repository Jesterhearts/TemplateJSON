#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <sstream>
#include <string>
#include <type_traits>

#include "json_parsing_helpers.hpp"
#include "json_number_parsers.hpp"
#include "json_enum_parsers.hpp"
#include "json_iterable_parser.hpp"
#include "json_array_parser.hpp"
#include "json_tuple_parser.hpp"
#include "json_pointer_parsers.hpp"

namespace tjson {
    template<>
    void to_json(const std::string& from, detail::Stringbuf& out) {
        out.push_back('\"');
        out.append(from);
        out.push_back('\"');
    }

    template<>
    jsonIter from_json(jsonIter iter, std::string& classInto) {
        iter = advance_past_whitespace(iter);
        if(*iter != '\"') {
            json_parsing_error(iter, "Not a valid string begin token");
        }

        ++iter;
        auto endOfString = find_end_of_string(iter);
        if(*endOfString != '\"') {
            json_parsing_error(iter, "Not a valid string end token");
        }

        size_t len = std::distance(iter, endOfString);
        classInto.clear();
        classInto.append(iter, len);

        ++endOfString;
        return endOfString;
    }

    template<>
    void to_json(const std::wstring& from, detail::Stringbuf& out) {
        out.push_back('\"');

        std::string narrowString(from.begin(), from.end());
        out.append(narrowString);

        out.push_back('\"');
    }


    template<>
    jsonIter from_json(jsonIter iter, std::wstring& classInto) {
        iter = advance_past_whitespace(iter);
        if(*iter != '\"') {
            json_parsing_error(iter, "Not a valid string begin token");
        }

        ++iter;
        auto endOfString = find_end_of_string(iter);
        if(*endOfString != '\"') {
            json_parsing_error(iter, "Not a valid string end token");
        }

        classInto = std::wstring(iter, endOfString);
        ++endOfString;
        return endOfString;
    }

    template<typename T1, typename T2>
    void to_json(const std::pair<T1, T2>& from, detail::Stringbuf& out) {
        out.push_back('[');
        detail::to_json(from.first, out);
        out.push_back(',');
        detail::to_json(from.second, out);
        out.push_back(']');
    }

    template<typename T1, typename T2>
    jsonIter from_json(jsonIter iter, std::pair<T1, T2>& into) {
        iter = advance_past_whitespace(iter);

        if(*iter != '[') {
            json_parsing_error(iter, "No array start token");
        }
        ++iter;

        typedef typename std::remove_cv<T1>::type FirstType;
        typedef typename std::remove_cv<T2>::type SecondType;

        FirstType& first = const_cast<FirstType&>(into.first);
        SecondType& second = const_cast<SecondType&>(into.second);

        iter = detail::from_json(iter, first);

        iter = advance_past_whitespace(iter);
        if(*iter != ',') {
            json_parsing_error(iter, "Pair does not have two values");
        }
        ++iter;

        iter = detail::from_json(iter, second);

        iter = advance_past_whitespace(iter);
        if(*iter != ']') {
            json_parsing_error(iter, "No end to JSON pair");
        }

        ++iter;
        return iter;
    }

    namespace detail {
        template<>
        json_finline void to_json<char, true>(char from, detail::Stringbuf& out) {
            out.push_back('\"');
            out.push_back(from);
            out.push_back('\"');
        }

        template<>
        json_finline jsonIter from_json<char, true>(jsonIter iter, char& into) {
            iter = advance_past_whitespace(iter);

            if(*iter != '\"') {
                json_parsing_error(iter, "Not a valid string begin token");
            }
            ++iter;

            if(!*iter) {
                json_parsing_error(iter,"No string data");
            }

            //Todo, handle unicode escape sequences
            auto endOfString = iter + 1;
            if(*endOfString != '\"') {
                json_parsing_error(iter,"No string end token");
            }

            into = *iter;

            ++endOfString;
            return endOfString;
        }

        template<>
        json_finline void to_json<wchar_t, true>(wchar_t from, detail::Stringbuf& out) {
            out.push_back('\"');

            std::wstring wideChar(1, from);
            std::string narrowChar(wideChar.begin(), wideChar.end());

            out.append(narrowChar);
            out.push_back('\"');
        }

        template<>
        json_finline jsonIter from_json<wchar_t, true>(jsonIter iter, wchar_t& into) {
            iter = advance_past_whitespace(iter);
            if(*iter != '\"') {
                json_parsing_error(iter, "Not a valid string begin token");
            }

            ++iter;

            if(!*iter) {
                json_parsing_error(iter,"No string data");
            }

            //Todo, handle unicode escape sequences
            auto endOfString = iter + 1;
            if(*endOfString != '\"') {
                json_parsing_error(iter,"No string end token");
            }

            into = std::wstring(iter, endOfString)[0];

            ++endOfString;
            return endOfString;
        }

        template<typename ClassType,
                 enable_if_const<ClassType> = true>
        json_deserialize_const_warning
        json_finline jsonIter from_json(jsonIter iter, ClassType& into) {
            //TODO: allow advancing without generating data
            typename std::remove_const<ClassType>::type shadow;
            return detail::from_json(iter, shadow);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_class> = true>
        json_finline void to_json(const ClassType& from, detail::Stringbuf& out) {
            tjson::to_json(from, out);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_class> = true>
        json_finline jsonIter from_json(jsonIter iter, ClassType& classInto) {
            return tjson::from_json(iter, classInto);
        }
    }
}
#endif
