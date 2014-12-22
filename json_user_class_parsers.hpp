#pragma once
#ifndef __JSON_USER_CLASS_PARSERS_HPP__
#define __JSON_USER_CLASS_PARSERS_HPP__

#include "json_member_mapper.hpp"
namespace JSON {
namespace detail {
    template<typename classFor, typename underlyingType>
    json_finline void MemberToJSON(const classFor& classFrom, underlyingType classFor::* member,
                                   std::string& out) {
        detail::ToJSON(classFrom.*member, out);
    }

    template<typename classFor, typename underlyingType>
    json_finline void MemberToJSON(const classFor& classFrom, underlyingType* member,
                                   std::string& out) {
        detail::ToJSON(*member, out);
    }

    template<typename classFor,
             typename underlyingType, underlyingType member,
             template<typename UT, UT MT> class Member>
    json_finline void MemberToJSON(const classFor& classFrom, Member<underlyingType, member>&&,
                                   std::string& out) {
        MemberToJSON(classFrom, member, out);
    }

#ifndef _MSC_VER
    template<typename classFor,
             template<typename... M> class ML>
    json_finline void MembersToJSON(const classFor& classFrom, std::string& out, ML<>&&) {
#else
    template<typename classFor,
             typename... members,
             template<typename... M> class ML>
    json_finline void MembersToJSON(const classFor& classFrom, std::string& out, ML<members...>&&) {
#endif
    }

    template<typename classFor,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline void MembersToJSON(const classFor& classFrom, std::string& out,
                                    ML<member, members...>&&) {
        out.append(1, '\"');
        out.append(member::key);
        out.append("\":", 2);

        MemberToJSON(classFrom, member(), out);
        if(sizeof...(members) > 0) {
            out.append(1, ',');
        }
        MembersToJSON(classFrom, out, ML<members...>());
    }

    template<typename classFor, size_t membersRemaining>
    struct JSONReader {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter) {
            std::string nextKey;
            iter = ParseNextKey(iter, nextKey);
            iter = ValidateKeyValueMapping(iter);

            iter = AdvancePastWhitespace(iter);

            iter = MemberFromJSON(classInto, nextKey, iter, MembersHolder<classFor>::members());
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
            std::string nextKey;
            iter = ParseNextKey(iter, nextKey);
            iter = ValidateKeyValueMapping(iter);

            iter = AdvancePastWhitespace(iter);

            iter = MemberFromJSON(classInto, nextKey, iter, MembersHolder<classFor>::members());
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
