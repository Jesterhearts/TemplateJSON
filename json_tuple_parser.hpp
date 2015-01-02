#pragma once
#ifndef __JSON_TUPLE_PARSER_HPP__
#define __JSON_TUPLE_PARSER_HPP__

/* I want to include as few headers as possible, but tuples suck to set up properly */
#include <tuple>

#include "json_internal_declarations.hpp"

namespace tjson {
    namespace detail {
        template<typename TupleType,
                 size_t curIndex,
                 bool lastValue,
                 typename curType,
                 typename... Types>
        struct TupleHandler : reference_only {

            static void to_json(const TupleType& classFrom, Stringbuf& out) {
                detail::to_json(std::get<curIndex>(classFrom), out);
                out.push_back(',');
                TupleHandler<TupleType, curIndex + 1, sizeof...(Types) == 1, Types...>
                    ::to_json(classFrom, out);
            }

            static void from_json(Tokenizer& tokenizer, DataMember<TupleType>& into) {
                DataMember<curType> data;

                detail::from_json(tokenizer, data);
                std::get<curIndex>(into.access()) = data.consume();

                tokenizer.advance_past_or_fail_if_not<','>("Not a valid tuple value");

                TupleHandler<TupleType, curIndex + 1, sizeof...(Types) == 1, Types...>
                    ::from_json(tokenizer, into);
            }
        };

        template<typename TupleType,
                 size_t curIndex,
                 typename curType,
                 typename... Types>
        struct TupleHandler<TupleType, curIndex, true, curType, Types...> : reference_only {

            static void to_json(const TupleType& classFrom, Stringbuf& out) {
                detail::to_json(std::get<curIndex>(classFrom), out);
            }

            static void from_json(Tokenizer& tokenizer, DataMember<TupleType>& into) {
                DataMember<curType> data;

                detail::from_json(tokenizer, data);
                std::get<curIndex>(into.access()) = data.consume();

                tokenizer.advance_past_or_fail_if_not<']'>("No tuple end token");
            }
        };
    } /* detail */

    template<typename... Types>
    void to_json(const std::tuple<Types...>& from, detail::Stringbuf& out) {
        out.push_back('[');
        detail::TupleHandler<std::tuple<Types...>,
                             0,
                             sizeof...(Types) == 1,
                             Types...
                            >::to_json(from, out);
        out.push_back(']');
    }

    template<typename... Types>
    void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::tuple<Types...>>& into) {
        tokenizer.consume_array_start();

        into.write();
        detail::TupleHandler<std::tuple<Types...>, 0, sizeof...(Types) == 1, Types...>
            ::from_json(tokenizer, into);
    }
} /* tjson */
#endif
