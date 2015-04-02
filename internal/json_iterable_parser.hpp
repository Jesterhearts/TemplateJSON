#pragma once
#ifndef __JSON_ITERABLE_PARSER_HPP__
#define __JSON_ITERABLE_PARSER_HPP__

namespace std {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4099)
    template<typename T, size_t N> struct array;
#pragma warning(pop)
#endif

    template<typename T, typename A> class deque;
    template<typename T, typename A> class forward_list;
    template<typename T, typename A> class list;
    template<typename T, typename A> class vector;

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
            inline static void insert(Type& type, VType&& input) {
                type.emplace(input);
            }
        };

        template<typename VType, typename A>
        struct IterableInserter<std::vector<VType, A>, VType> : reference_only {
            inline static void insert(std::vector<VType, A>& type, VType&& input) {
                type.emplace_back(input);
            }
        };

        template<typename... T, template<typename... T> class Container>
        inline void to_json(const Container<T...>& from, detail::Stringbuf& out) {
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
        inline void from_json(Tokenizer& tokenizer, DataMemberBase<Container<T...>>& into) {
            tokenizer.consume_array_start();

            using value_type = typename Container<T...>::value_type;
            using inserter = IterableInserter<Container<T...>, value_type>;

            into.write();

            if(tokenizer.seek() != ']') {
                do {
                    tokenizer.advance_if<','>();

                    DataMember<value_type, detail::data_internal_store_tag> input;
                    detail::from_json(tokenizer, input);

                    inserter::insert(into.access(), input.consume());
                } while(tokenizer.seek() == ',');
            }

            tokenizer.advance_or_fail_if_not<']'>("No array end token");
        }
    } /* iterables */
    } /* detail */

    template<typename T, std::size_t A>
    inline void to_json(const std::array<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, std::size_t A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::array<T, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::deque<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::deque<T, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::forward_list<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::forward_list<T, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::list<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::list<T, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename T, typename A>
    inline void to_json(const std::vector<T, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename T, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::vector<T, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename K, typename C, typename A>
    inline void to_json(const std::set<K, C, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename C, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::set<K, C, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename K, typename C, typename A>
    inline void to_json(const std::multiset<K, C, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename C, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::multiset<K, C, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename K, typename H, typename KE, typename A>
    inline void to_json(const std::unordered_set<K, H, KE, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename H, typename KE, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::unordered_set<K, H, KE, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }

    template<typename K, typename T, typename C, typename A>
    inline void to_json(const std::map<K, T, C, A>& from, detail::Stringbuf& out) {
        detail::iterables::to_json(from, out);
    }

    template<typename K, typename T, typename C, typename A>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMemberBase<std::map<K, T, C, A>>& into) {
        return detail::iterables::from_json(tokenizer, into);
    }
}
#endif
