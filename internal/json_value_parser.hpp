#pragma once
#ifndef __JSON_VALUE_PARSER_HPP__
#define __JSON_VALUE_PARSER_HPP__

#include <sstream>
#include <string>
#include <type_traits>

#include "json_stringbuf.hpp"
#include "json_data_store.hpp"
#include "json_number_parsers.hpp"
#include "json_enum_parsers.hpp"
#include "json_iterable_parser.hpp"
#include "json_tuple_parser.hpp"
#include "json_pointer_parsers.hpp"

namespace tjson {
    template<typename ClassType>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataStore<ClassType>& into);

    template<typename ClassType>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<ClassType>& into);

    //UTF8 todo
    template<>
    inline void to_json(const std::string& from, detail::Stringbuf& out) {
        out.push_back('\"');
        out.append_and_escape(from);
        out.push_back('\"');
    }

    //UTF8 todo
    template<>
    json_never_inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::string>& into) {
        detail::Tokenizer::UnescapedString unescaped = tokenizer.consume_string_token();
        into.write(unescaped.data, unescaped.length);
    }

    //UTF8 todo
    template<>
    inline void to_json(const std::wstring& from, detail::Stringbuf& out) {
        out.push_back('\"');

        std::string narrowString(from.begin(), from.end());
        out.append_and_escape(narrowString);

        out.push_back('\"');
    }


    //UTF8 todo
    template<>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::wstring>& into) {
        detail::Tokenizer::UnescapedString unescaped = tokenizer.consume_string_token();
        into.write(unescaped.data, unescaped.data + unescaped.length);
    }

    template<typename T1, typename T2>
    inline void to_json(const std::pair<T1, T2>& from, detail::Stringbuf& out) {
        out.push_back('[');
        detail::to_json(from.first, out);
        out.push_back(',');
        detail::to_json(from.second, out);
        out.push_back(']');
    }

    template<typename T1, typename T2>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::pair<T1, T2>>& into) {
        tokenizer.consume_array_start();

        typedef typename std::remove_cv<T1>::type FirstType;
        typedef typename std::remove_cv<T2>::type SecondType;

        detail::DataMemberImpl<FirstType, detail::data_emplace_store_tag> first(
            const_cast<FirstType*>(&into.storage_ptr()->first));
        detail::DataMemberImpl<SecondType, detail::data_emplace_store_tag> second(
            const_cast<SecondType*>(&into.storage_ptr()->second));

        detail::from_json(tokenizer, first);
        tokenizer.advance_past_or_fail_if_not<','>("Pair does not have two values");

        detail::from_json(tokenizer, second);
        tokenizer.advance_past_or_fail_if_not<']'>("No end to JSON pair");

        first.consume();
        second.consume();
        into.set_should_destroy_storage();
    }

    namespace detail {
        //UTF8 todo
        template<typename ClassType,
                 JSON_ENABLE_IF(ClassType, is_char)>
        inline void to_json(ClassType from, detail::Stringbuf& out) {
            out.push_back('\"');
            out.push_back(from);
            out.push_back('\"');
        }

        //UTF8 todo
        template<typename ClassType,
                 JSON_ENABLE_IF(ClassType, is_char)>
        inline void from_json(Tokenizer& tokenizer, DataMember<ClassType>& into) {
            tokenizer.advance_past_or_fail_if_not<'\"'>( "Not a valid string begin token");

            into.write(tokenizer.take());

            tokenizer.advance_or_fail_if_not<'\"'>("No string end token");
        }

        //UTF8 todo
        template<typename ClassType,
                 JSON_ENABLE_IF(ClassType, is_wchar)>
        inline void to_json(ClassType from, detail::Stringbuf& out) {
            out.push_back('\"');

            std::wstring wideChar(1, from);
            std::string narrowChar(wideChar.begin(), wideChar.end());

            out.append(narrowChar);
            out.push_back('\"');
        }

        //UTF8 todo
        template<typename ClassType,
                 JSON_ENABLE_IF(ClassType, is_wchar)>
        inline void from_json(Tokenizer& tokenizer, DataMember<ClassType>& into) {
            tokenizer.advance_past_or_fail_if_not<'\"'>( "Not a valid string begin token");

            const char* startOfString = tokenizer.position();
            tokenizer.skip(1);
            const char* endOfString = tokenizer.position();

            into.write(std::wstring(startOfString, endOfString)[0]);

            tokenizer.advance_or_fail_if_not<'\"'>("No string end token");
        }

        template<typename ClassType,
                 JSON_ENABLE_IF(ClassType, std::is_class)>
        inline void to_json(const ClassType& from, detail::Stringbuf& out) {
            tjson::to_json(from, out);
        }

        template<typename ClassType,
                 JSON_ENABLE_IF(ClassType, std::is_class)>
        inline void from_json(detail::Tokenizer& tokenizer, DataMember<ClassType>& into) {
            tjson::from_json(tokenizer, into);
        }
    }
}
#endif
