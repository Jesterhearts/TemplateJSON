#pragma once
#ifndef __JSON_TUPLE_PARSER_HPP__
#define __JSON_TUPLE_PARSER_HPP__

/* I want to include as few headers as possible, but tuples suck to set up properly */
#include <tuple>

namespace JSON {
    template<typename TupleType,
             size_t curIndex,
             bool lastValue,
             typename curType,
             typename... Types>
    struct TupleHandler {
        json_finline static void ToJSON(const TupleType* classFrom,
                                               stringt& jsonString) {
            jsonString += JSONFnInvoker<curType>::ToJSON(&std::get<curIndex>(*classFrom));
            jsonString += JSON_ST(",");
            TupleHandler<TupleType,
                         curIndex + 1,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, jsonString);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     TupleType& into) {
            curType& value = std::get<curIndex>(into);

            iter = JSONFnInvoker<curType>::FromJSON(iter, end, value);
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L',') {
                ThrowBadJSONError(iter, end, "Not a valid tuple value");
            }
            ++iter;
            return TupleHandler<TupleType,
                                curIndex + 1,
                                sizeof...(Types) == 1,
                                Types...
                               >::FromJSON(iter, end, into);
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
        json_finline static void ToJSON(const TupleType* classFrom,
                                               stringt& jsonString) {
            jsonString += JSONFnInvoker<curType>::ToJSON(&std::get<curIndex>(*classFrom));
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     TupleType& into) {
            curType& value = std::get<curIndex>(into);
            iter = JSONFnInvoker<curType>::FromJSON(iter, end, value);
            iter = AdvancePastWhitespace(iter, end);
            if(iter == end || *iter != L']') {
                ThrowBadJSONError(iter, end, "No tuple end token");
            }
            ++iter;
            return iter;
        }
    };

    template<typename... Types>
    struct JSONFnInvokerImpl<std::tuple<Types...>> {
        json_finline static stringt ToJSON(const std::tuple<Types...>* classFrom) {
            stringt json(JSON_ST("["));
            TupleHandler<std::tuple<Types...>,
                         0,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, json);
            json += JSON_ST("]");
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,
                                                     std::tuple<Types...>& into) {
            if(iter == end || *iter != L'[') {
                ThrowBadJSONError(iter, end, "No tuple start token");
            }
            ++iter;

            return TupleHandler<std::tuple<Types...>,
                                0,
                                sizeof...(Types) == 1,
                                Types...
                               >::FromJSON(iter, end, into);
        }
    };
}
#endif
