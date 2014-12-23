#pragma once
#ifndef __JSON_ENUM_PARSERS_HPP__
#define __JSON_ENUM_PARSERS_HPP__

namespace JSON {
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
        json_finline EnumType ValidateEnum(jsonIter iter, typename std::underlying_type<EnumType>::type value,
                                           EnumValueList<EnumType>&&) {
            ThrowBadJSONError(iter, "Value not in enum");
        }

        template<typename EnumType, EnumType member, EnumType... members>
        json_finline EnumType ValidateEnum(jsonIter iter,
                                           typename std::underlying_type<EnumType>::type value,
                                           EnumValueList<EnumType, member, members...>&&) {
            if (value == member) {
                return static_cast<EnumType>(value);
            }
            else {
                return ValidateEnum(iter, value, EnumValueList<EnumType, members...>());
            }
        }

        template<typename EnumType, EnumType base, EnumType max>
        json_finline EnumType ValidateEnum(jsonIter iter,
                                           typename std::underlying_type<EnumType>::type value,
                                           ContiguousEnumValueList<EnumType, base, max>&&) {
            if(value < base || value > max) {
                ThrowBadJSONError(iter, "Value not in enum");
            }
            else {
                return static_cast<EnumType>(value);
            }
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum> = true>
        json_finline void ToJSON(ClassType from, detail::stringbuf& out) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            detail::ToJSON(static_cast<underlying_type>(from), out);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum> = true>
        json_finline jsonIter FromJSON(jsonIter iter, ClassType& into) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            underlying_type value;
            iter = detail::FromJSON(iter, value);
            into = ValidateEnum(iter, value, EnumValidator<ClassType>::values());
            return iter;
        }
    }
}
#endif
