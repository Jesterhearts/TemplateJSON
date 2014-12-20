#pragma once
#ifndef __JSON_ITERABLE_PARSER_HPP__
#define __JSON_ITERABLE_PARSER_HPP__

namespace std {
#ifdef _MSC_VER
    template<typename T, size_t N> class array;
#else
    template<typename T, size_t N> struct array;
#endif
    template<typename T, typename A> class deque;
    template<typename T, typename A> class forward_list;
    template<typename T, typename A> class list;
    template<typename T, typename A> class vector;

    template<typename T1, typename T2> struct pair;

    template<typename K, typename C, typename A> class set;
    template<typename K, typename C, typename A> class multiset;
    template<typename K, typename H, typename KE, typename A> class unordered_set;

    template<typename K, typename T, typename C, typename A> class map;
}

namespace JSON {
    namespace detail {
        namespace iterables {
            template<typename Type, typename VType>
            struct IterableInserter {
                json_finline static void Insert(Type& type, VType&& input) {
                    type.emplace(input);
                }
            };

            template<typename VType, typename A>
            struct IterableInserter<std::vector<VType, A>, VType> {
                json_finline static void Insert(std::vector<VType, A>& type, VType&& input) {
                    type.emplace_back(input);
                }
            };

            template<typename... T, template<typename... T> class Container>
            json_finline void ToJSON(const Container<T...>& from, std::string& out) {
                out.append(1, '[');

                if(!from.empty()) {
                    auto iter = std::begin(from);
                    auto endItr = std::prev(std::end(from));

                    for(; iter != endItr; ++iter) {
                        detail::ToJSON(*iter, out);
                        out.append(1, ',');
                    }
                    detail::ToJSON(*iter, out);
                }

                out.append(1, ']');
            }

            template<typename... T, template<typename... T> class Container>
            json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, Container<T...>& into) {
                if(end - iter < 2) {
                    ThrowBadJSONError(iter, end, "No array tokens");
                }

                iter = AdvancePastWhitespace(iter, end);
                if(*iter != L'[') {
                    ThrowBadJSONError(iter, end, "No array start token");
                }
                ++iter;

                using value_type = typename Container<T...>::value_type;
                using inserter = IterableInserter<Container<T...>, value_type>;
                while(iter != end && *iter != L']') {
                    value_type input;
                    iter = detail::FromJSON(iter, end, input);

                    inserter::Insert(into, std::move(input));
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
        }
    }

    template<typename T, std::size_t A>
    json_finline void ToJSON(const std::array<T, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename T, std::size_t A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::array<T, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename T, typename A>
    json_finline void ToJSON(const std::deque<T, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename T, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::deque<T, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename T, typename A>
    json_finline void ToJSON(const std::forward_list<T, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename T, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::forward_list<T, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename T, typename A>
    json_finline void ToJSON(const std::list<T, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename T, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::list<T, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename T, typename A>
    json_finline void ToJSON(const std::vector<T, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename T, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::vector<T, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename K, typename C, typename A>
    json_finline void ToJSON(const std::set<K, C, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename K, typename C, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::set<K, C, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename K, typename C, typename A>
    json_finline void ToJSON(const std::multiset<K, C, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename K, typename C, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::multiset<K, C, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename K, typename H, typename KE, typename A>
    json_finline void ToJSON(const std::unordered_set<K, H, KE, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename K, typename H, typename KE, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::unordered_set<K, H, KE, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }

    template<typename K, typename T, typename C, typename A>
    json_finline void ToJSON(const std::map<K, T, C, A>& from, std::string& out) {
        detail::iterables::ToJSON(from, out);
    }

    template<typename K, typename T, typename C, typename A>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::map<K, T, C, A>& into) {
        return detail::iterables::FromJSON(iter, end, into);
    }
}
#endif
