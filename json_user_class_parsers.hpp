#pragma once
#ifndef __JSON_USER_CLASS_PARSERS_HPP__
#define __JSON_USER_CLASS_PARSERS_HPP__

#include "json_member_mapper.hpp"
namespace JSON {
namespace detail {
    template<typename classFor>
    struct MemberMap {
        static const MapTypes::maptype<classFor> map;

        MemberMap() = delete;
        ~MemberMap() = delete;
    };

    template<typename classFor>
    const MapTypes::maptype<classFor> MemberMap<classFor>::map = CreateMap<classFor>(
        MembersHolder<classFor>::members()
    );

    template<typename classFor, typename underlyingType>
    json_finline std::string MemberToJSON(const classFor& classFrom, underlyingType classFor::* member) {
        return detail::ToJSON(classFrom.*member);
    }

    template<typename classFor, typename underlyingType>
    json_finline std::string MemberToJSON(const classFor& classFrom, underlyingType* member) {
        return detail::ToJSON(*member);
    }

    template<typename classFor,
             typename underlyingType, underlyingType member,
             template<typename UT, UT MT> class Member>
    json_finline std::string MemberToJSON(const classFor& classFrom,
                                      Member<underlyingType, member>&&) {
        return MemberToJSON(classFrom, member);
    }

#ifndef _MSC_VER
    template<typename classFor,
             template<typename... M> class ML>
    json_finline std::string MembersToJSON(const classFor& classFrom, ML<>&&) {
#else
    template<typename classFor,
             typename... members,
             template<typename... M> class ML>
    json_finline std::string MembersToJSON(const classFor& classFrom, ML<members...>&&) {
#endif
        return "";
    }

    template<typename classFor,
             typename member, typename... members,
             template<typename... M> class ML>
    json_finline std::string MembersToJSON(const classFor& classFrom, ML<member, members...>&&) {
        std::string json(1, '\"');
        json.append(member::key);
        json.append("\":");

        json.append(MemberToJSON(classFrom, member()));
        if(sizeof...(members) > 0) {
            json.append(1, ',');
        }
        json.append(MembersToJSON(classFrom, ML<members...>()));
        return json;
    }

    template<typename classFor, size_t membersRemaining>
    struct JSONReader {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end) {
            std::string nextKey;
            iter = ParseNextKey(iter, end, nextKey);
            iter = ValidateKeyValueMapping(iter, end);

            iter = AdvancePastWhitespace(iter, end);

            auto insertAt = MemberMap<classFor>::map.find(nextKey);
            if(insertAt == MemberMap<classFor>::map.end()) {
                ThrowBadJSONError(iter, end, "No key in object");
            }

            iter = insertAt->second(classInto, iter, end);

            iter = AdvancePastWhitespace(iter, end);
            if(iter != end && *iter == ',')  {
                ++iter;
            }
            else {
                ThrowBadJSONError(iter, end, "Missing key separator");
            }

            return JSONReader<classFor, membersRemaining - 1>::MembersFromJSON(classInto, iter, end);
        }
    };

    template<typename classFor>
    struct JSONReader<classFor, 1> {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end) {
            std::string nextKey;
            iter = ParseNextKey(iter, end, nextKey);
            iter = ValidateKeyValueMapping(iter, end);

            iter = AdvancePastWhitespace(iter, end);

            auto insertAt = MemberMap<classFor>::map.find(nextKey);
            if(insertAt == MemberMap<classFor>::map.end()) {
                ThrowBadJSONError(iter, end, "No key in object");
            }

            iter = insertAt->second(classInto, iter, end);

            iter = AdvancePastWhitespace(iter, end);
            if(iter != end && *iter == ',') {
                ++iter;
            }
            else if(*iter != '}') {
                ThrowBadJSONError(iter, end, "Missing key separator");
            }

            return iter;
        }
    };

    template<typename classFor, typename... types, template<typename... M> class ML>
    json_finline jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end,
                                          ML<types...>&&) {
        return JSONReader<classFor, sizeof...(types)>::MembersFromJSON(classInto, iter, end);
    }
} /* detail */
} /* JSON */

#endif
