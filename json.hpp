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
    template<typename classFor, typename underlyingType>
    json_finline stringt MemberToJSON(const classFor& classFrom, underlyingType classFor::* member) {
        return JSONFnInvoker<decltype(classFrom.*member)>::ToJSON(classFrom.*member);
    }

    template<typename classFor, typename underlyingType>
    json_finline stringt MemberToJSON(const classFor& classFrom, underlyingType* member) {
        return JSONFnInvoker<underlyingType>::ToJSON(member);
    }

    template<typename classFor,
             const char_t* key, const char_t*... keys,
             template<const char_t*...> class K,
             typename type, typename... types,
             template<typename... M> class ML>
    json_finline stringt MembersToJSON(const classFor& classFrom,
                                       const K<key, keys...>& k1,
                                       const ML<type, types...>& ml) {
        stringt json(1, JSON_ST('\"'));
        json.append(key);
        json.append(JSON_ST("\":"), 2);

        json.append(MemberToJSON(classFrom, type::value));
        if(sizeof...(keys) > 0) {
            json.append(1, JSON_ST(','));
        }
        json.append(MembersToJSON(classFrom, K<keys...>(), ML<types...>()));
        return json;
    }

    template<typename classFor,
             template<const char_t*...> class K,
             template<typename... M> class ML>
    json_finline stringt MembersToJSON(const classFor& classFrom, K<>&& k1, ML<>&& ml) {
        return JSON_ST("");
    }

    template<typename classFor, size_t membersRemaining>
    struct JSONReader {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end) {
            stringt nextKey;
            iter = ParseNextKey(iter, end, nextKey);
            iter = ValidateKeyValueMapping(iter, end);

            iter = AdvancePastWhitespace(iter, end);

            auto insertAt = MemberMap<classFor>::mapping.values.find(nextKey.c_str());
            if(insertAt == MemberMap<classFor>::mapping.values.end()) {
                ThrowBadJSONError(iter, end, JSON_ST("No key in object"));
            }

            iter = insertAt->second.MemberToJSON(classInto, iter, end);

            iter = AdvancePastWhitespace(iter, end);
            if(iter != end && *iter == JSON_ST(',') ) {
                ++iter;
            }
            else {
                ThrowBadJSONError(iter, end, JSON_ST("Missing key separator"));
            }

            return JSONReader<classFor, membersRemaining - 1>::MembersFromJSON(classInto, iter, end);
        }
    };

    template<typename classFor>
    struct JSONReader<classFor, 1> {
        json_finline static jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end) {
            stringt nextKey;
            iter = ParseNextKey(iter, end, nextKey);
            iter = ValidateKeyValueMapping(iter, end);

            iter = AdvancePastWhitespace(iter, end);

            auto insertAt = MemberMap<classFor>::mapping.values.find(nextKey.c_str());
            if(insertAt == MemberMap<classFor>::mapping.values.end()) {
                ThrowBadJSONError(iter, end, JSON_ST("No key in object"));
            }

            iter = insertAt->second.MemberToJSON(classInto, iter, end);

            iter = AdvancePastWhitespace(iter, end);
            if(iter != end && *iter == JSON_ST(',')) {
                ++iter;
            }
            else if(*iter != JSON_ST('}')) {
                ThrowBadJSONError(iter, end, JSON_ST("Missing key separator"));
            }

            return iter;
        }
    };

    template<typename classFor, typename... types, template<typename... M> class ML>
    json_finline jsonIter MembersFromJSON(classFor& classInto, jsonIter iter, jsonIter end,
                                          const ML<types...> ml) {
        return JSONReader<classFor, sizeof...(types)>::MembersFromJSON(classInto, iter, end);
    }

    template<typename classFor>
    stringt ToJSON(const classFor& classFrom) {
        stringt json(JSON_ST("{"));

        json.append(MembersToJSON(classFrom,
                                  KeysHolder<classFor>::keys,
                                  MembersHolder<classFor>::members
            )
        );

        json.append(JSON_ST("}"));
        return json;
    }

    template<typename classFor>
    jsonIter FromJSON(jsonIter iter, jsonIter end, classFor& classInto) {
        iter = ValidateObjectStart(iter, end);

        iter = MembersFromJSON<classFor>(classInto, iter, end, MembersHolder<classFor>::members);

        return ValidateObjectEnd(iter, end);
    }

    template<typename classFor>
    classFor FromJSON(const stringt& jsonData) {
        classFor classInto;
        auto iter = jsonData.begin();
        auto end = jsonData.end();

        FromJSON(iter, end, classInto);

        return classInto;
    }

    template<typename classFor>
    struct ClassMap {
        ClassMap()
            : values(CreateMap<classFor>(KeysHolder<classFor>::keys, MembersHolder<classFor>::members))
        {}

        const MapTypes::maptype<classFor> values;
    };

    template<typename classFor>
    struct MemberMap {
        static const ClassMap<classFor> mapping;
    };

    template<typename classFor>
    const ClassMap<classFor> MemberMap<classFor>::mapping;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                \
    namespace JSON {                                                \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                   \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                \
                                                                    \
        template struct MemberMap<CLASS_NAME>;                      \
                                                                    \
        template stringt ToJSON<CLASS_NAME>(const CLASS_NAME&);     \
        template CLASS_NAME FromJSON<CLASS_NAME>(const stringt&);   \
    }

#endif
