#pragma once
#ifndef __JSON_USER_CLASS_PARSERS_HPP__
#define __JSON_USER_CLASS_PARSERS_HPP__

#include "json_common_defs.hpp"
#include "json_data_store.hpp"
#include "json_member_mapper.hpp"
#include "json_parsing_helpers.hpp"
#include "json_tokenizer.hpp"

namespace tjson {
namespace detail {
    template<typename ClassType,
             typename UnderlyingType, UnderlyingType member>
    inline void member_to_json(const ClassType& classFrom,
                                     MemberInfo<UnderlyingType, member>&&, detail::Stringbuf& out) {
        using type = basic_type<decltype(classFrom.*member)>;
        detail::to_json<type>(classFrom.*member, out);
    }

    template<typename ClassType>
    inline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out,
                                      MemberList<>&&) {}

    template<typename ClassType,
             typename member, typename... members>
    inline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out,
                                      MemberList<member, members...>&&) {
        out.append(member::key, sizeof(member::key) - 1);
        out.push_back(':');

        member_to_json(classFrom, member(), out);
        if(sizeof...(members) > 0) {
            out.push_back(',');
        }
        members_to_json(classFrom, out, MemberList<members...>());
    }

    inline void member_from_json(DataList<>& into, const char* startOfKey, size_t keylen,
                                 Tokenizer& tokenizer, MemberList<>&&) {
        std::string message("Invalid key for object: (");
        message.append(startOfKey, keylen);
        message.append(")");
        tokenizer.parsing_error(std::move(message));
    }

    template<typename DataType, typename... DataTypes,
             typename member, typename... members>
    inline void member_from_json(DataList<DataType, DataTypes...>& into, const char* startOfKey,
                                 size_t keylen, Tokenizer& tokenizer,
                                 MemberList<member, members...>&&) {

        //-1 for opening ", -1 for closing ", -1 for \0
        constexpr const size_t extraneous_chars_in_key = 3;
        constexpr const size_t len = sizeof(member::key) - extraneous_chars_in_key;

        //+1 to pass opening "
        if(keylen == len && std::memcmp(startOfKey, member::key + 1, len) == 0) {
            detail::from_json(tokenizer, into.data);
        }
        else {
            member_from_json(data_list_next(into), startOfKey, keylen, tokenizer, MemberList<members...>());
        }
    }

    template<typename ClassType, size_t membersRemaining>
    struct Reader : reference_only {
        static void members_from_json(DataStore<ClassType>& into, Tokenizer& tokenizer) {
            std::pair<const char*, size_t> keyBeginAndLength = tokenizer.consume_string_token();

            tokenizer.consume_kv_mapping();

            member_from_json(into.data_list, keyBeginAndLength.first, keyBeginAndLength.second, tokenizer,
                             MembersHolder<ClassType>::members());

            tokenizer.advance_or_fail_if_not<','>("Incomplete object");

            Reader<ClassType, membersRemaining - 1>::members_from_json(into, tokenizer);
        }
    };

    template<typename ClassType>
    struct Reader<ClassType, 1> : reference_only {
        static void members_from_json(DataStore<ClassType>& into, Tokenizer& tokenizer) {
            std::pair<const char*, size_t> keyBeginAndLength = tokenizer.consume_string_token();

            tokenizer.consume_kv_mapping();

            member_from_json(into.data_list, keyBeginAndLength.first, keyBeginAndLength.second, tokenizer,
                             MembersHolder<ClassType>::members());

            tokenizer.advance_if<','>();
            tokenizer.consume_object_end();
        }
    };

    template<typename ClassType, typename... types, template<typename... M> class ML>
    inline void members_from_json(DataStore<ClassType>& into, Tokenizer& tokenizer, ML<types...>&&) {
        Reader<ClassType, sizeof...(types)>::members_from_json(into, tokenizer);
    }
} /* detail */
} /* JSON */

#endif
