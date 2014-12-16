#pragma once
#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/expand.hpp>

#include <boost/preprocessor/stringize.hpp>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/variadic/elem.hpp>

#include "json_common_defs.hpp"
#include "json_value_parser.hpp"
#include "json_keys_handler.hpp"
#include "json_member_mapper.hpp"

namespace JSON {

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

    template<typename classFor>
    std::string ToJSON(const classFor& classFrom) {
        std::string json("{");

        json.append(MembersToJSON(classFrom, MembersHolder<classFor>::members()));

        json.append("}");
        return json;
    }

    template<typename classFor>
    jsonIter FromJSON(jsonIter iter, jsonIter end, classFor& classInto) {
        iter = ValidateObjectStart(iter, end);

        iter = MembersFromJSON<classFor>(classInto, iter, end, MembersHolder<classFor>::members());

        return ValidateObjectEnd(iter, end);
    }

    template<typename classFor>
    classFor FromJSON(const std::string& jsonData) {
        classFor classInto;
        auto iter = jsonData.begin();
        auto end = jsonData.end();

        FromJSON(iter, end, classInto);

        return classInto;
    }

    namespace detail {
        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _class&&) {
            return JSON::ToJSON(from);
        }

        template<typename ClassType>
        json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& classInto, _class&&) {
            return JSON::FromJSON(iter, end, classInto);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                \
    namespace JSON {                                                \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                   \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                \
                                                                    \
        template struct MemberMap<CLASS_NAME>;                      \
                                                                    \
        template std::string ToJSON<CLASS_NAME>(const CLASS_NAME&);     \
        template CLASS_NAME FromJSON<CLASS_NAME>(const std::string&);   \
    }

#endif
