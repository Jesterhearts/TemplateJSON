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
        json_finline EnumType ValidateEnum(jsonIter iter, jsonIter end,
                                           typename std::underlying_type<EnumType>::type value,
                                           EnumValueList<EnumType>&&) {
            ThrowBadJSONError(iter, end, "Value not in enum");
        }

        template<typename EnumType, EnumType member, EnumType... members>
        json_finline EnumType ValidateEnum(jsonIter iter, jsonIter end,
                                           typename std::underlying_type<EnumType>::type value,
                                           EnumValueList<EnumType, member, members...>&&) {
            if (value == member) {
                return static_cast<EnumType>(value);
            }
            else {
                return ValidateEnum(iter, end, value, EnumValueList<members...>());
            }
        }

        template<typename EnumType, EnumType base, EnumType max>
        json_finline EnumType ValidateEnum(jsonIter iter, jsonIter end,
                                           typename std::underlying_type<EnumType>::type value,
                                           ContiguousEnumValueList<EnumType, base, max>&&) {
            if(value < base || value > max) {
                ThrowBadJSONError(iter, end, "Value not in enum");
            }
            else {
                return static_cast<EnumType>(value);
            }
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum> = true>
        json_finline std::string ToJSON(const ClassType& from) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            return detail::ToJSON(static_cast<const underlying_type&>(from));
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum> = true>
        json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            underlying_type value;
            iter = detail::FromJSON(iter, end, value);
            into = ValidateEnum(iter, end, value, EnumValidator<ClassType>::values());
            return iter;
        }
    }
}
#endif
