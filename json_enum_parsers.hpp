#pragma once
#ifndef __JSON_ENUM_PARSERS_HPP__
#define __JSON_ENUM_PARSERS_HPP__

namespace tjson {
    template<typename EnumType, EnumType... T>
    struct EnumValueList {};

    template<typename EnumType, EnumType base, EnumType max>
    struct ContiguousEnumValueList {};

    template<typename EnumType>
    struct EnumValidator {
        constexpr static EnumValueList<EnumType> values() {
            return EnumValueList<EnumType>();
        }
    };

    namespace detail {
        template<typename EnumType>
        json_no_return
        json_finline EnumType validate_enum(jsonIter iter, typename std::underlying_type<EnumType>::type value,
                                           EnumValueList<EnumType>&&) {
            json_parsing_error(iter, "Value not in enum");
        }

        template<typename EnumType, EnumType member, EnumType... members>
        json_finline EnumType validate_enum(jsonIter iter,
                                           typename std::underlying_type<EnumType>::type value,
                                           EnumValueList<EnumType, member, members...>&&) {
            if (value == member) {
                return static_cast<EnumType>(value);
            }
            else {
                return validate_enum(iter, value, EnumValueList<EnumType, members...>());
            }
        }

        template<typename EnumType, EnumType base, EnumType max>
        json_finline EnumType validate_enum(jsonIter iter,
                                           typename std::underlying_type<EnumType>::type value,
                                           ContiguousEnumValueList<EnumType, base, max>&&) {
            if(value < base || value > max) {
                json_parsing_error(iter, "Value not in enum");
            }
            else {
                return static_cast<EnumType>(value);
            }
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum> = true>
        json_finline void to_json(ClassType from, detail::Stringbuf& out) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            detail::to_json(static_cast<underlying_type>(from), out);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum> = true>
        json_finline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            DataMember<underlying_type> value;
            iter = detail::from_json(iter, value);
            into.write(validate_enum(iter, value.consume(), EnumValidator<ClassType>::values()));
            return iter;
        }
    }
}
#endif
