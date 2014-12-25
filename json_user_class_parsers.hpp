#pragma once
#ifndef __JSON_USER_CLASS_PARSERS_HPP__
#define __JSON_USER_CLASS_PARSERS_HPP__

#include "json_member_mapper.hpp"
namespace tjson {
namespace detail {
    template<typename ClassType, typename UnderlyingType>
    json_finline void member_to_json(const ClassType& classFrom, UnderlyingType ClassType::* member,
                                   detail::Stringbuf& out) {
        detail::to_json<typename std::remove_const<UnderlyingType>::type>(classFrom.*member, out);
    }

    template<typename ClassType, typename UnderlyingType>
    json_finline void member_to_json(const ClassType& classFrom, UnderlyingType* member,
                                   detail::Stringbuf& out) {
        detail::to_json<typename std::remove_const<UnderlyingType>::type>(*member, out);
    }

    template<typename ClassType,
             typename UnderlyingType, UnderlyingType member,
             template<typename UT, UT MT> class Member>
    json_finline void member_to_json(const ClassType& classFrom, Member<UnderlyingType, member>&&,
                                   detail::Stringbuf& out) {
        member_to_json(classFrom, member, out);
    }

#ifndef _MSC_VER
    template<typename ClassType,
             template<typename... M> class ML>
    json_finline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out, ML<>&&) {}
#else
    template<typename ClassType,
             typename... members,
             template<typename... M> class ML>
    json_finline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out, ML<members...>&&) {}
#endif

    template<typename ClassType,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline void members_to_json(const ClassType& classFrom, detail::Stringbuf& out,
                                    ML<member, members...>&&) {
        out.append(member::key, sizeof(member::key) - 1);
        out.push_back(':');

        member_to_json(classFrom, member(), out);
        if(sizeof...(members) > 0) {
            out.push_back(',');
        }
        members_to_json(classFrom, out, ML<members...>());
    }

    template<typename ClassType, typename UnderlyingType>
    json_finline jsonIter member_from_json(ClassType* into, jsonIter iter,
                                           UnderlyingType ClassType::* member) {
        return detail::from_json(iter, &(into->*member));
    }

    template<typename ClassType, typename UnderlyingType>
    json_finline jsonIter member_from_json(ClassType* into, jsonIter iter,
                                           UnderlyingType* member) {
        return detail::from_json(iter, member);
    }

    template<typename ClassType,
             typename UnderlyingType, UnderlyingType member,
             template<typename UT, UT MT> class MemberInfo>
    json_finline jsonIter member_from_json(ClassType* into, jsonIter iter,
                                           MemberInfo<UnderlyingType, member>&&) {
        return member_from_json(into, iter, member);
    }

    template<typename ClassType, typename memberType>
    json_finline jsonIter member_from_json(ClassType* into, jsonIter iter) {
        return member_from_json(into, iter, memberType());
    }

#ifndef _MSC_VER
    template<typename ClassType,
             template<typename... M> class ML,
             typename... value_types>
    json_finline jsonIter member_from_json(ClassType* on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                           ML<>&&) {
#else
    template<typename ClassType,
        typename... members,
        template<typename... M> class ML,
        typename... value_types>
    json_finline jsonIter member_from_json(ClassType* on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<members...>&&) {
#endif
        json_parsing_error(startOfKey, "No key in object");
    }

    template<typename ClassType,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline jsonIter member_from_json(ClassType* on, jsonIter startOfKey, size_t keylen, jsonIter iter,
                                         ML<member, members...>&&) {
        constexpr const size_t len = sizeof(member::key) - 1;
        if(keylen == len && std::memcmp(startOfKey, member::key, len) == 0) {
            return member_from_json<ClassType, member>(on, iter);
        }
        else {
            return member_from_json(on, startOfKey, keylen, iter, ML<members...>());
        }
    }

    template<typename ClassType, size_t membersRemaining>
    struct Reader {
        json_finline static jsonIter members_from_json(ClassType* into, jsonIter iter) {
            jsonIter startOfKey = find_key_begin(iter);
            size_t keylen = std::distance(startOfKey, find_key_end(startOfKey + 1));

            iter = parse_key_value_mapping(startOfKey + keylen);

            iter = member_from_json(into, startOfKey, keylen, iter, MembersHolder<ClassType>::members());

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
        json_finline static jsonIter members_from_json(ClassType* into, jsonIter iter) {
            jsonIter startOfKey = find_key_begin(iter);
            size_t keylen = std::distance(startOfKey, find_key_end(startOfKey + 1));

            iter = parse_key_value_mapping(startOfKey + keylen);

            iter = member_from_json(into, startOfKey, keylen, iter, MembersHolder<ClassType>::members());

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
    json_finline jsonIter members_from_json(ClassType* into, jsonIter iter, ML<types...>&&) {
        return Reader<ClassType, sizeof...(types)>::members_from_json(into, iter);
    }
} /* detail */
} /* JSON */

#endif
