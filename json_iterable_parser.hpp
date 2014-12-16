#pragma once
#ifndef __JSON_ITERABLE_PARSER_HPP__
#define __JSON_ITERABLE_PARSER_HPP__

namespace JSON {

#define JSON_ITERABLE_PARSER(STL_TYPE, ...)                                                 \
    struct JSONFnInvokerImpl<std::STL_TYPE<__VA_ARGS__>> {                                  \
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
        json_finline static void Insert(Type& type, VType&& input) {
            type.emplace(std::move(input));
        }
    };

    template<typename VType, typename A>
    struct IterableInserter<std::vector<VType, A>, VType> {
        json_finline static void Insert(std::vector<VType, A>& type, VType&& input) {
            type.emplace_back(std::move(input));
        }
    };

    template<typename Type>
    struct IterableParser {
        json_finline static std::string ToJSON(const Type& value) {
            std::string result("[");

            if(!value.empty()) {
                auto iter = std::begin(value);
                auto endItr = std::prev(std::end(value));
                typedef decltype(*iter) valtype;

                for(; iter != endItr; ++iter) {
                    result.append(detail::ToJSON(*iter));
                    result.append(",");
                }
                result += detail::ToJSON(*iter);
            }

            result.append("]");
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
                IterableInserter<Type, typename Type::value_type>::Insert(into, std::move(input));
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

    namespace detail {
        template<typename T, typename... O,
                 template<typename T, typename... D> class Container>
        json_finline static std::string ContainerToJSON(const Container<T, O...>& from) {
            return IterableParser<Container<T, O...>>::ToJSON(from);
        }
    }

    template<typename T, std::size_t A>
    json_finline std::string ToJSON(const std::array<T, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename T, typename A>
    json_finline std::string ToJSON(const std::deque<T, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename T, typename A>
    json_finline std::string ToJSON(const std::forward_list<T, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename T, typename A>
    json_finline std::string ToJSON(const std::list<T, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename T, typename A>
    json_finline std::string ToJSON(const std::vector<T, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename K, typename C, typename A>
    json_finline std::string ToJSON(const std::set<K, C, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename K, typename C, typename A>
    json_finline std::string ToJSON(const std::multiset<K, C, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename K, typename H, typename KE, typename A>
    json_finline std::string ToJSON(const std::unordered_set<K, H, KE, A>& from) {
        return detail::ContainerToJSON(from);
    }

    template<typename K, typename T, typename C, typename A>
    json_finline std::string ToJSON(const std::map<K, T, C, A>& from) {
        return detail::ContainerToJSON(from);
    }
}
#endif
