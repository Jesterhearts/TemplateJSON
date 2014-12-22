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

namespace JSON {
    template<>
    json_finline void ToJSON(const std::string& from, std::string& out) {
        out.append(1, '\"');
        out.append(from);
        out.append(1, '\"');
    }

    template<>
    json_finline jsonIter FromJSON(jsonIter iter, std::string& classInto) {
        iter = AdvancePastWhitespace(iter);
        if(*iter != '\"') {
            ThrowBadJSONError(iter, "Not a valid string begin token");
        }

        ++iter;
        auto endOfString = AdvanceToEndOfString(iter);
        if(*endOfString != '\"') {
            ThrowBadJSONError(iter, "Not a valid string end token");
        }

        size_t len = std::distance(iter, endOfString);
        classInto.clear();
        classInto.append(iter, len);

        ++endOfString;
        return endOfString;
    }

    template<>
    json_finline void ToJSON(const std::wstring& from, std::string& out) {
        out.append(1, '\"');

        std::string narrowString(from.begin(), from.end());
        out.append(narrowString);

        out.append(1, '\"');
    }


    template<>
    json_finline jsonIter FromJSON(jsonIter iter, std::wstring& classInto) {
        iter = AdvancePastWhitespace(iter);
        if(*iter != '\"') {
            ThrowBadJSONError(iter, "Not a valid string begin token");
        }

        ++iter;
        auto endOfString = AdvanceToEndOfString(iter);
        if(*endOfString != '\"') {
            ThrowBadJSONError(iter, "Not a valid string end token");
        }

        classInto = std::wstring(iter, endOfString);
        ++endOfString;
        return endOfString;
    }

    template<typename T1, typename T2>
    json_finline void ToJSON(const std::pair<T1, T2>& from, std::string& out) {
        out.append(1, '[');
        detail::ToJSON(from.first, out);
        out.append(1, ',');
        detail::ToJSON(from.second, out);
        out.append(1, ']');
    }

    template<typename T1, typename T2>
    json_finline jsonIter FromJSON(jsonIter iter, std::pair<T1, T2>& into) {
        iter = AdvancePastWhitespace(iter);

        if(*iter != '[') {
            ThrowBadJSONError(iter, "No array start token");
        }
        ++iter;

        typedef typename std::remove_cv<T1>::type FirstType;
        typedef typename std::remove_cv<T2>::type SecondType;

        FirstType& first = const_cast<FirstType&>(into.first);
        SecondType& second = const_cast<SecondType&>(into.second);

        iter = detail::FromJSON(iter, first);

        iter = AdvancePastWhitespace(iter);
        if(*iter != ',') {
            ThrowBadJSONError(iter, "Pair does not have two values");
        }
        ++iter;

        iter = detail::FromJSON(iter, second);

        iter = AdvancePastWhitespace(iter);
        if(*iter != ']') {
            ThrowBadJSONError(iter, "No end to JSON pair");
        }

        ++iter;
        return iter;
    }

    namespace detail {
        template<>
        json_finline void ToJSON<char, true>(char from, std::string& out) {
            out.append(1, '\"');
            out.append(1, from);
            out.append(1, '\"');
        }

        template<>
        json_finline jsonIter FromJSON<char, true>(jsonIter iter, char& into) {
            iter = AdvancePastWhitespace(iter);

            if(*iter != '\"') {
                ThrowBadJSONError(iter, "Not a valid string begin token");
            }
            ++iter;

            if(!*iter) {
                ThrowBadJSONError(iter,"No string data");
            }

            //Todo, handle unicode escape sequences
            auto endOfString = iter + 1;
            if(*endOfString != '\"') {
                ThrowBadJSONError(iter,"No string end token");
            }

            into = *iter;

            ++endOfString;
            return endOfString;
        }

        template<>
        json_finline void ToJSON<wchar_t, true>(wchar_t from, std::string& out) {
            out.append(1, '\"');

            std::wstring wideChar(1, from);
            std::string narrowChar(wideChar.begin(), wideChar.end());

            out.append(narrowChar);
            out.append(1, '\"');
        }

        template<>
        json_finline jsonIter FromJSON<wchar_t, true>(jsonIter iter, wchar_t& into) {
            iter = AdvancePastWhitespace(iter);
            if(*iter != '\"') {
                ThrowBadJSONError(iter, "Not a valid string begin token");
            }

            ++iter;

            if(!*iter) {
                ThrowBadJSONError(iter,"No string data");
            }

            //Todo, handle unicode escape sequences
            auto endOfString = iter + 1;
            if(*endOfString != '\"') {
                ThrowBadJSONError(iter,"No string end token");
            }

            into = std::wstring(iter, endOfString)[0];

            ++endOfString;
            return endOfString;
        }

        template<typename ClassType,
                 enable_if_const<ClassType> = true>
        json_deserialize_const_warning
        json_finline jsonIter FromJSON(jsonIter iter, ClassType& into) {
            //TODO: allow advancing without generating data
            typename std::remove_const<ClassType>::type shadow;
            return detail::FromJSON(iter, shadow);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_class> = true>
        json_finline void ToJSON(const ClassType& from, std::string& out) {
            JSON::ToJSON(from, out);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_class> = true>
        json_finline jsonIter FromJSON(jsonIter iter, ClassType& classInto) {
            return JSON::FromJSON(iter, classInto);
        }
    }
}
#endif
