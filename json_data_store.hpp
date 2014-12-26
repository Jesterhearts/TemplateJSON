#pragma once
#ifndef __JSON_DATA_STORE_HPP__
#define __JSON_DATA_STORE_HPP__

#include <cassert>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "json_common_defs.hpp"
#include "json_functions.hpp"
#include "json_member_mapper.hpp"

namespace tjson {
namespace detail {
    template<typename ClassType>
    using raw_data = typename
        std::aligned_storage<sizeof(ClassType), std::alignment_of<ClassType>::value>::type;

    template<typename>
    struct underlying;

    template<typename ClassType, typename UnderlyingType, UnderlyingType ClassType::* member>
    struct underlying<MemberInfo<UnderlyingType ClassType::*, member>> {
        using type = UnderlyingType;
    };

    template<typename, typename enabled = bool>
    struct initialized_flag;

    template<typename ClassType>
    struct initialized_flag<ClassType, typename std::enable_if<!std::is_trivially_destructible<ClassType>::value, bool>::type> {
        constexpr initialized_flag() : initialized(false) {}

        json_finline void set_initialized() { initialized = true; }
        json_finline bool is_initialized() { return initialized; }

    private:
        bool initialized;
    };

    template<typename ClassType>
    struct initialized_flag<ClassType, typename std::enable_if<std::is_trivially_destructible<ClassType>::value, bool>::type> {
        json_finline void set_initialized() { }
        json_finline bool is_initialized() { return true; }
    };

    template<typename StoredType>
    struct DataMember : initialized_flag<StoredType> {
        using initialized_flag<StoredType>::is_initialized;

        template<typename... Args>
        json_finline void write(Args&&... args) {
            new (&storage) StoredType{ std::forward<Args>(args)... };
            set_initialized();
        }

        json_finline StoredType&& consume() {
            assert(is_initialized());
            return std::move(*static_cast<StoredType*>(static_cast<void*>(&storage)));
        }

        json_finline StoredType& access() {
            assert(is_initialized());
            return *static_cast<StoredType*>(static_cast<void*>(&storage));
        }

        ~DataMember() { 
            DestroyStorage<StoredType>();
        }

    private:
        using initialized_flag<StoredType>::set_initialized;

        template<typename Destroying,
                 typename std::enable_if<!std::is_trivially_destructible<Destroying>::value, bool>::type = true>
        json_finline
        void DestroyStorage() {
            if(is_initialized()) {
                static_cast<StoredType*>(static_cast<void*>(&storage))->~StoredType();
            }
        }

        template<typename Destroying,
                 typename std::enable_if<std::is_trivially_destructible<Destroying>::value, bool>::type = true>
        json_finline
        void DestroyStorage() { }

        raw_data<StoredType> storage;
    };

    template<typename... Types>
    struct DataList {};

    template<typename StoredType>
    struct DataList<StoredType> : DataList<> {
        DataMember<StoredType> data;
        json_finline DataList<>& next() {
            return static_cast<DataList<>&>(*this);
        }
    };

    template<typename StoredType, typename NextType, typename... Types>
    struct DataList<StoredType, NextType, Types...> : DataList<NextType, Types...> {
        DataMember<StoredType> data;
        json_finline DataList<NextType, Types...>& next() {
            return static_cast<DataList<NextType, Types...>&>(*this);
        }
    };

    template<typename... members>
    constexpr DataList<typename underlying<members>::type...> data_list_type(MemberList<members...>&&) {}

    template<typename ClassType>
    struct DataStore {
    public:
        /**
         * Data store is no longer valid after this call
         */
        ClassType realize() {
            return realize(data_list);
        }

        /**
         * Data store is no longer valid after this call
         */
        void transfer_to(DataMember<ClassType>& into) {
            transfer_to(into, data_list);
        }

        decltype(data_list_type(MembersHolder<ClassType>::members())) data_list;

    private:
        //For constructing/returning an object
        template<typename DataType, typename... Values>
        json_finline ClassType realize(DataList<DataType>& list, Values&&... values) {
            return ClassType{ std::forward<Values>(values)..., list.data.consume() };
        }

        template<typename DataType, typename... DataTypes, typename... Values>
        json_finline ClassType realize(DataList<DataType, DataTypes...>& list, Values&&... values) {
            return realize(list.next(), std::forward<Values>(values)..., list.data.consume());
        }

        template<typename DataType, typename... DataTypes>
        json_finline ClassType realize(DataList<DataType, DataTypes...>& list) {
            return realize(list.next(), list.data.consume());
        }

        //For initializing a DataMember
        template<typename DataType, typename... Values>
        json_finline void transfer_to(DataMember<ClassType>& into, DataList<DataType>& list,
                                             Values&&... values) {
            into.write(std::forward<Values>(values)..., list.data.consume());
        }

        template<typename DataType, typename... DataTypes, typename... Values>
        json_finline void transfer_to(DataMember<ClassType>& into,
                                             DataList<DataType, DataTypes...>& list, Values&&... values) {
            transfer_to(into, list.next(), std::forward<Values>(values)..., list.data.consume());
        }

        template<typename DataType, typename... DataTypes>
        json_finline void transfer_to(DataMember<ClassType>& into,
                                             DataList<DataType, DataTypes...>& list) {
            transfer_to(into, list.next(), list.data.consume());
        }
    };
}
}

#endif
