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
        json_force_inline void validate_enum(Tokenizer&                                    tokenizer,
                                             typename std::underlying_type<EnumType>::type value,
                                             EnumValueList<EnumType>&&) {
            tokenizer.parsing_error("Value not in enum");
        }

        template<typename EnumType, EnumType member, EnumType... members>
        json_force_inline void validate_enum(Tokenizer&                                    tokenizer,
                                             typename std::underlying_type<EnumType>::type value,
                                             EnumValueList<EnumType, member, members...>&&) {
            if (value != member) {
                validate_enum(tokenizer, value, EnumValueList<EnumType, members...>());
            }
        }

        template<typename EnumType, EnumType base, EnumType max>
        json_force_inline void validate_enum(Tokenizer&                                    tokenizer,
                                             typename std::underlying_type<EnumType>::type value,
                                             ContiguousEnumValueList<EnumType, base, max>&&) {
            if(value < base || value > max) {
                tokenizer.parsing_error("Value not in enum");
            }
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_enum>>
        json_force_inline void to_json(ClassType from, detail::Stringbuf& out) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            detail::to_json(static_cast<underlying_type>(from), out);
        }

        template<typename ClassType, enable_if<ClassType, std::is_enum>>
        json_force_inline void from_json(Tokenizer& tokenizer, DataMemberBase<ClassType>& into) {
            using underlying_type = typename std::underlying_type<ClassType>::type;
            DataMember<underlying_type, detail::data_emplace_store_tag> value(
                        reinterpret_cast<underlying_type*>(into.storage_ptr));

            detail::from_json(tokenizer, value);
            value.consume();
            into.set_should_destroy_storage();
            validate_enum(tokenizer, *static_cast<DataMemberBase<underlying_type>&>(value).storage_ptr,
                          EnumValidator<ClassType>::values());
        }
    }
}
#endif
