#pragma once
#ifndef __JSON_USER_CLASS_PARSERS_HPP__
#define __JSON_USER_CLASS_PARSERS_HPP__

#include "json_member_mapper.hpp"
namespace JSON {
namespace detail {
    template<typename classFor, typename underlyingType>
    json_finline void MemberToJSON(const classFor& classFrom, underlyingType classFor::* member,
                                   detail::stringbuf& out) {
        detail::ToJSON<typename std::remove_const<underlyingType>::type>(classFrom.*member, out);
    }

    template<typename classFor, typename underlyingType>
    json_finline void MemberToJSON(const classFor& classFrom, underlyingType* member,
                                   detail::stringbuf& out) {
        detail::ToJSON<typename std::remove_const<underlyingType>::type>(*member, out);
    }

    template<typename classFor,
             typename underlyingType, underlyingType member,
             template<typename UT, UT MT> class Member>
    json_finline void MemberToJSON(const classFor& classFrom, Member<underlyingType, member>&&,
                                   detail::stringbuf& out) {
        MemberToJSON(classFrom, member, out);
    }

#ifndef _MSC_VER
    template<typename classFor,
             template<typename... M> class ML>
    json_finline void MembersToJSON(const classFor& classFrom, detail::stringbuf& out, ML<>&&) {}
#else
    template<typename classFor,
             typename... members,
             template<typename... M> class ML>
    json_finline void MembersToJSON(const classFor& classFrom, detail::stringbuf& out, ML<members...>&&) {}
#endif

    template<typename classFor,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline void MembersToJSON(const classFor& classFrom, detail::stringbuf& out,
                                    ML<member, members...>&&) {
        out.append(member::key, sizeof(member::key) - 1);
        out.push_back(':');

        MemberToJSON(classFrom, member(), out);
        if(sizeof...(members) > 0) {
            out.push_back(',');
        }
        MembersToJSON(classFrom, out, ML<members...>());
    }

    template<typename classFor, size_t membersRemaining>
    struct JSONReader {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter) {
            jsonIter startOfKey = FindStartOfKey(iter);
            size_t keylen = std::distance(startOfKey, FindEndOfKey(startOfKey + 1));

            iter = ValidateKeyValueMapping(startOfKey + keylen);

            iter = MemberFromJSON(classInto, startOfKey, keylen, iter, MembersHolder<classFor>::members());

            iter = AdvancePastWhitespace(iter);
            if(*iter == ',')  {
                ++iter;
            }
            else {
                ThrowBadJSONError(iter, "Missing key separator");
            }

            return JSONReader<classFor, membersRemaining - 1>::MembersFromJSON(classInto, iter);
        }

        JSONReader() = delete;
        ~JSONReader() = delete;
    };

    template<typename classFor>
    struct JSONReader<classFor, 1> {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter) {
            jsonIter startOfKey = FindStartOfKey(iter);
            size_t keylen = std::distance(startOfKey, FindEndOfKey(startOfKey + 1));

            iter = ValidateKeyValueMapping(startOfKey + keylen);

            iter = MemberFromJSON(classInto, startOfKey, keylen, iter, MembersHolder<classFor>::members());

            iter = AdvancePastWhitespace(iter);
            if(*iter == ',') {
                ++iter;
            }
            else if(*iter != '}') {
                ThrowBadJSONError(iter, "Missing key separator");
            }

            return iter;
        }

        JSONReader() = delete;
        ~JSONReader() = delete;
    };

    template<typename classFor, typename... types, template<typename... M> class ML>
    json_finline jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, ML<types...>&&) {
        return JSONReader<classFor, sizeof...(types)>::MembersFromJSON(classInto, iter);
    }
} /* detail */
} /* JSON */

#endif
