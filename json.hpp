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

#include <cstdlib>

#include "json_common_defs.hpp"
#include "json_data_store.hpp"
#include "json_value_parser.hpp"
#include "json_user_class_parsers.hpp"
#include "json_keys_handler.hpp"
#include "json_member_mapper.hpp"

namespace tjson {
    template<typename ClassType>
    inline void to_json(const ClassType& classFrom, detail::Stringbuf& out) {
        out.push_back('{');
        detail::members_to_json(classFrom, out, MembersHolder<ClassType>::members());

        out.push_back('}');
    }

    template<typename ClassType>
    inline std::string to_json(const ClassType& classFrom) {
        detail::Stringbuf json;

        tjson::to_json(classFrom, json);

        return json.to_string();
    }

    template<typename ClassType>
    inline jsonIter from_json(jsonIter iter, detail::DataStore<ClassType>& into) {
        iter = parse_object_start(iter);

        iter = detail::members_from_json(into, iter, MembersHolder<ClassType>::members());

        return parse_object_end(iter);
    }

    template<typename ClassType>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<ClassType>& into) {
        detail::DataStore<ClassType> data;

        iter = from_json(iter, data);

        data.transfer_to(into);

        return iter;
    }

    template<typename ClassType>
    inline ClassType from_json(const std::string& jsonData) {
        detail::DataStore<ClassType> data;

        auto iter = jsonData.c_str();

        from_json(iter, data);

        return data.realize();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                \
    namespace tjson {                                                \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                   \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                \
                                                                    \
        template std::string to_json<CLASS_NAME>(const CLASS_NAME&);     \
        template CLASS_NAME from_json<CLASS_NAME>(const std::string&);   \
    }

#define JSON_ENABLE_ENUM(ENUM_NAME, ...)                            \
    namespace tjson {                                                \
        template<>                                                  \
        struct EnumValidator<ENUM_NAME> {                           \
            constexpr static EnumValueList<ENUM_NAME, __VA_ARGS__>  \
            values() {                                              \
                return EnumValueList<ENUM_NAME, __VA_ARGS__>();     \
            }                                                       \
        };                                                          \
    }

#define JSON_ENABLE_CONTIGUOUS_ENUM(ENUM_NAME, ...)                         \
    namespace tjson {                                                        \
        template<>                                                          \
        struct EnumValidator<ENUM_NAME> {                                   \
            constexpr static ContiguousEnumValueList<ENUM_NAME, __VA_ARGS__>\
            values() {                                                      \
                return ContiguousEnumValueList<ENUM_NAME, __VA_ARGS__>();   \
            }                                                               \
        };                                                                  \
    }

#endif
