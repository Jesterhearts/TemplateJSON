#pragma once
#ifndef __JSON_USER_CLASS_PARSERS_HPP__
#define __JSON_USER_CLASS_PARSERS_HPP__

#include "json_common_defs.hpp"
#include "json_data_store.hpp"
#include "json_member_mapper.hpp"
#include "json_parsing_helpers.hpp"

namespace tjson {
namespace detail {
    template<typename ClassType,
             typename UnderlyingType, UnderlyingType member>
    json_finline void member_to_json(const ClassType& classFrom,
                                     MemberInfo<UnderlyingType, member>&&, detail::Stringbuf& out) {
        using nonref = typename std::remove_reference<decltype(classFrom.*member)>::type;
        using nonconst = typename std::remove_const<nonref>::type;

        detail::to_json<nonconst>(classFrom.*member, out);
    }

    template<typename ClassType>
    json_finline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out,
                                      MemberList<>&&) {}

    template<typename ClassType,
             typename member, typename... members>
    json_finline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out,
                                      MemberList<member, members...>&&) {
        out.append(member::key, sizeof(member::key) - 1);
        out.push_back(':');

        member_to_json(classFrom, member(), out);
        if(sizeof...(members) > 0) {
            out.push_back(',');
        }
        members_to_json(classFrom, out, MemberList<members...>());
    }

    json_finline jsonIter member_from_json(DataList<>& into, jsonIter startOfKey, size_t keylen,
                                           jsonIter iter, MemberList<>&&) {
        json_parsing_error(startOfKey, "No key in object");
    }

    template<typename DataType, typename... DataTypes,
             typename member, typename... members>
    json_finline jsonIter member_from_json(DataList<DataType, DataTypes...>& into,
                                           jsonIter startOfKey, size_t keylen, jsonIter iter,
                                           MemberList<member, members...>&&) {
        constexpr const size_t len = sizeof(member::key) - 1;

        if(keylen == len && std::memcmp(startOfKey, member::key, len) == 0) {
            return detail::from_json(iter, into.data);
        }
        else {
            return member_from_json(into.next(), startOfKey, keylen, iter, MemberList<members...>());
        }
    }

    template<typename ClassType, size_t membersRemaining>
    struct Reader {
        json_finline static jsonIter members_from_json(DataStore<ClassType>& into, jsonIter iter) {
            jsonIter startOfKey = find_key_begin(iter);
            size_t keylen = std::distance(startOfKey, find_key_end(startOfKey + 1));

            iter = parse_key_value_mapping(startOfKey + keylen);

            iter = member_from_json(into.data_list, startOfKey, keylen, iter,
                                    MembersHolder<ClassType>::members());

            iter = advance_past_whitespace(iter);
            if(*iter == ',')  {
                ++iter;
            }
            else {
                json_parsing_error(iter, "Missing key separator");
            }

            return Reader<ClassType, membersRemaining - 1>::members_from_json(into, iter);
        }

        Reader() = delete;
        ~Reader() = delete;
    };

    template<typename ClassType>
    struct Reader<ClassType, 1> {
        json_finline static jsonIter members_from_json(DataStore<ClassType>& into, jsonIter iter) {
            jsonIter startOfKey = find_key_begin(iter);
            size_t keylen = std::distance(startOfKey, find_key_end(startOfKey + 1));

            iter = parse_key_value_mapping(startOfKey + keylen);

            iter = member_from_json(into.data_list, startOfKey, keylen, iter,
                                    MembersHolder<ClassType>::members());

            iter = advance_past_whitespace(iter);
            if(*iter == ',') {
                ++iter;
            }
            else if(*iter != '}') {
                json_parsing_error(iter, "Missing key separator");
            }

            return iter;
        }

        Reader() = delete;
        ~Reader() = delete;
    };

    template<typename ClassType, typename... types, template<typename... M> class ML>
    json_finline jsonIter members_from_json(DataStore<ClassType>& into, jsonIter iter,
                                            ML<types...>&&) {
        return Reader<ClassType, sizeof...(types)>::members_from_json(into, iter);
    }
} /* detail */
} /* JSON */

#endif
