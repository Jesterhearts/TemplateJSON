#pragma once
#ifndef __JSON_ITERABLE_PARSER_HPP__
#define __JSON_ITERABLE_PARSER_HPP__

namespace JSON {

#define JSON_ITERABLE_PARSER(STL_TYPE, ...)                                                 \
    struct JSONFnInvokerImpl<std::STL_TYPE<__VA_ARGS__>> {                                  \
        json_finline static stringt ToJSON(const std::STL_TYPE<__VA_ARGS__>* classFrom) {   \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::ToJSON(classFrom);           \
        }                                                                                   \
                                                                                            \
        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end,                  \
                                                     std::STL_TYPE<__VA_ARGS__>& into) {    \
            return IterableParser<std::STL_TYPE<__VA_ARGS__>>::FromJSON(iter, end, into);   \
        }                                                                                   \
    }

////////////////////////////////////////////////////////////////////////////////
// IterableParser implementation
// Transforms an iterable type into an array of its internal values
////
    template<typename Type, typename VType>
    struct IterableInserter {
        json_finline static void Insert(Type& type, VType& input) {
            type.insert(input);
        }
    };

    template<typename VType, typename A>
    struct IterableInserter<std::vector<VType, A>, VType> {
        json_finline static void Insert(std::vector<VType, A>& type, VType& input) {
            type.push_back(input);
        }
    };

    template<typename Type>
    struct IterableParser {
        json_finline static stringt ToJSON(const Type* value) {
            stringt result(JSON_ST("["));

            if(!value->empty()) {
                auto iter = std::begin(*value);
                auto endItr = std::prev(std::end(*value));
                typedef decltype(*iter) valtype;

                for(; iter != endItr; ++iter) {
                    result += JSONFnInvoker<valtype>::ToJSON(*iter);
                    result += JSON_ST(",");
                }
                result += JSONFnInvoker<valtype>::ToJSON(*iter);
            }

            result += JSON_ST("]");
            return result;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, Type& into) {
            if(end - iter < 2) {
                ThrowBadJSONError(iter, end, "No array tokens");
            }

            iter = AdvancePastWhitespace(iter, end);
            if(*iter != L'[') {
                ThrowBadJSONError(iter, end, "No array start token");
            }
            ++iter;

            while(iter != end && *iter != L']') {
                typename Type::value_type input;
                //Each call advances iter past the end of the token read by the call
                iter = JSONFnInvoker<typename Type::value_type>::FromJSON(iter, end, input);
                IterableInserter<Type, typename Type::value_type>::Insert(into, input);
                iter = AdvancePastWhitespace(iter, end);

                if(iter != end && *iter == L',') {
                    ++iter;
                }
            }

            if(iter == end) {
                ThrowBadJSONError(iter, end, "No end to JSON array");
            }

            ++iter;
            return iter;
        }
    };
}
#endif
