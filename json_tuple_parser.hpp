#pragma once
#ifndef __JSON_TUPLE_PARSER_HPP__
#define __JSON_TUPLE_PARSER_HPP__

/* I want to include as few headers as possible, but tuples suck to set up properly */
#include <tuple>

namespace tjson {
    template<typename TupleType,
             size_t curIndex,
             bool lastValue,
             typename curType,
             typename... Types>
    struct TupleHandler {
        json_finline static void to_json(const TupleType& classFrom, detail::Stringbuf& out) {
            detail::to_json(std::get<curIndex>(classFrom), out);
            out.push_back(',');
            TupleHandler<TupleType,
                         curIndex + 1,
                         sizeof...(Types) == 1,
                         Types...
                        >::to_json(classFrom, out);
        }

        json_finline static jsonIter from_json(jsonIter iter, TupleType* into) {
            curType& value = std::get<curIndex>(*into);

            iter = detail::from_json(iter, &value);
            iter = advance_past_whitespace(iter);
            if(*iter != ',') {
                json_parsing_error(iter, "Not a valid tuple value");
            }
            ++iter;
            return TupleHandler<TupleType,
                                curIndex + 1,
                                sizeof...(Types) == 1,
                                Types...
                               >::from_json(iter, into);
        }
    };

    template<typename TupleType,
             size_t curIndex,
             typename curType,
             typename... Types>
    struct TupleHandler <TupleType,
                         curIndex,
                         true,
                         curType,
                         Types...> {
        json_finline static void to_json(const TupleType& classFrom, detail::Stringbuf& out) {
            detail::to_json(std::get<curIndex>(classFrom), out);
        }

        json_finline static jsonIter from_json(jsonIter iter, TupleType* into) {
            curType& value = std::get<curIndex>(*into);
            iter = detail::from_json(iter, &value);
            iter = advance_past_whitespace(iter);
            if(*iter != ']') {
                json_parsing_error(iter, "No tuple end token");
            }
            ++iter;
            return iter;
        }
    };

    template<typename... Types>
    void to_json(const std::tuple<Types...>& from, detail::Stringbuf& out) {
        out.push_back('[');
        TupleHandler<std::tuple<Types...>,
                     0,
                     sizeof...(Types) == 1,
                     Types...
                    >::to_json(from, out);
        out.push_back(']');
    }

    template<typename... Types>
    jsonIter from_json(jsonIter iter, std::tuple<Types...>* into) {
        if(*iter != '[') {
            json_parsing_error(iter, "No tuple start token");
        }
        ++iter;

        return TupleHandler<std::tuple<Types...>,
                            0,
                            sizeof...(Types) == 1,
                            Types...
                           >::from_json(iter, into);
    }
}
#endif
