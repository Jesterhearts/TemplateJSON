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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4624)
#endif

#include "internal/json_common_defs.hpp"
#include "internal/json_data_store.hpp"
#include "internal/json_object_hints.hpp"
#include "internal/json_keys_handler.hpp"
#include "internal/json_member_mapper.hpp"
#include "internal/json_user_class_parsers.hpp"
#include "internal/json_value_parser.hpp"

namespace tjson {
    template<typename ClassType>
    inline void to_json(const ClassType& classFrom, detail::Stringbuf& out) {
        out.push_back('{');
        detail::members_to_json(classFrom, out, MembersHolder<ClassType>::members());

        out.push_back('}');
    }

    template<typename ClassType>
    json_force_inline std::string to_json(const ClassType& classFrom) {
        detail::Stringbuf json;

        tjson::to_json(classFrom, json);

        return json.to_string();
    }

    template<typename ClassType>
    inline void from_json(detail::Tokenizer& tokenizer,
                          detail::DataStore<ClassType>& into)
    {
        tokenizer.consume_object_start();
        detail::members_from_json(into, tokenizer, MembersHolder<ClassType>::members());
    }

    template<typename ClassType>
    inline void from_json(detail::Tokenizer& tokenizer,
                          detail::DataMember<ClassType>& into) {
        detail::DataStoreImpl<ClassType, detail::data_emplace_store_tag> data(into.storage_ptr());

        from_json(tokenizer, data);

        data.transfer_storage(into);
    }

    template<typename ClassType>
    json_force_inline ClassType from_json(const std::string& jsonData) {
        detail::DataStoreImpl<ClassType> data;

        detail::Tokenizer tokenizer(jsonData);

        from_json(tokenizer, data);

        return data.realize();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_ENABLE(CLASS_NAME, ...)                                    \
    namespace tjson {                                                   \
        JSON_CREATE_KEYS(CLASS_NAME, __VA_ARGS__)                       \
        JSON_CREATE_MEMBERS(CLASS_NAME, __VA_ARGS__)                    \
                                                                        \
        template std::string to_json<CLASS_NAME>(const CLASS_NAME&);    \
        template CLASS_NAME from_json<CLASS_NAME>(const std::string&);  \
    }

#define JSON_ENABLE_ENUM(ENUM_NAME, ...)                        \
    namespace tjson {                                           \
        template<>                                              \
        struct EnumValidator<ENUM_NAME> {                       \
            static EnumValueList<ENUM_NAME, __VA_ARGS__>        \
            values() {                                          \
                return EnumValueList<ENUM_NAME, __VA_ARGS__>(); \
            }                                                   \
        };                                                      \
    }

#define JSON_ENABLE_CONTIGUOUS_ENUM(ENUM_NAME, ...)                         \
    namespace tjson {                                                       \
        template<>                                                          \
        struct EnumValidator<ENUM_NAME> {                                   \
            static ContiguousEnumValueList<ENUM_NAME, __VA_ARGS__>          \
            values() {                                                      \
                return ContiguousEnumValueList<ENUM_NAME, __VA_ARGS__>();   \
            }                                                               \
        };                                                                  \
    }

#ifndef _MSC_VER
#define JSON_HINT_CAN_BUILD_IN_PLACE(CLASS_NAME)                        \
    namespace tjson {                                                   \
    template<>                                                          \
    struct ConstructHint<CLASS_NAME> : detail::reference_only {         \
        using construction_type = object_hints::trivially_constructible;\
    };                                                                  \
    }
#else
#define JSON_HINT_CAN_BUILD_IN_PLACE(CLASS_NAME)                        \
    namespace tjson {                                                   \
    template<>                                                          \
    struct ConstructHint<CLASS_NAME> {                                  \
        using construction_type = object_hints::trivially_constructible;\
    };                                                                  \
    }
#endif


#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
