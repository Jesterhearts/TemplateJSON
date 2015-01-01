#pragma once
#ifndef __JSON_ITERABLE_PARSER_HPP__
#define __JSON_ITERABLE_PARSER_HPP__

namespace std {
    template<typename T, size_t N> struct array;

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

namespace tjson {
    namespace detail {
    namespace iterables {
        template<typename Type, typename VType>
        struct IterableInserter : reference_only {
            json_finline static void insert(Type& type, VType&& input) {
                type.emplace(input);
            }
        };

        template<typename VType, typename A>
        struct IterableInserter<std::vector<VType, A>, VType> : reference_only {
            json_finline static void insert(std::vector<VType, A>& type, VType&& input) {
                type.emplace_back(input);
            }
        };

        template<typename... T, template<typename... T> class Container>
        json_finline void to_json(const Container<T...>& from, detail::Stringbuf& out) {
            out.push_back('[');

            if(!from.empty()) {
                auto iter = std::begin(from);
                auto endItr = std::prev(std::end(from));

                for(; iter != endItr; ++iter) {
                    detail::to_json(*iter, out);
                    out.push_back(',');
                }
                detail::to_json(*iter, out);
            }

            out.push_back(']');
        }

        template<typename... T, template<typename... T> class Container>
        json_finline jsonIter from_json(jsonIter iter, DataMember<Container<T...>>& into) {
            //Placement TODO
            iter = advance_past_whitespace(iter);
            if(*iter != '[') {
                json_parsing_error(iter, "No array start token");
            }
            ++iter;

            using value_type = typename Container<T...>::value_type;
            using inserter = IterableInserter<Container<T...>, value_type>;

            into.write();
            while(iter && *iter != ']') {
                DataMember<value_type> input;
                iter = detail::from_json(iter, input);

                inserter::insert(into.access(), input.consume());
                iter = advance_past_whitespace(iter);

                if(*iter == ',') {
                    ++iter;
                }
            }

            if(*iter != ']') {
                json_parsing_error(iter, "No end to JSON array");
            }

            ++iter;
            return iter;
        }
    } /* iterables */
    } /* detail */

    template<typename T, std::size_t A>
    inline void to_json(const std::array<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, std::size_t A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::array<T, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::deque<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::deque<T, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::forward_list<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::forward_list<T, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::list<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::list<T, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::vector<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::vector<T, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename K, typename C, typename A>
    inline void to_json(const std::set<K, C, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename C, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::set<K, C, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename K, typename C, typename A>
    inline void to_json(const std::multiset<K, C, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename C, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::multiset<K, C, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename K, typename H, typename KE, typename A>
    inline void to_json(const std::unordered_set<K, H, KE, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename H, typename KE, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::unordered_set<K, H, KE, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }

    template<typename K, typename T, typename C, typename A>
    inline void to_json(const std::map<K, T, C, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename T, typename C, typename A>
    inline jsonIter from_json(jsonIter iter, detail::DataMember<std::map<K, T, C, A>>& into) {
        return detail::iterables::from_json(iter, into);
    }
}
#endif
