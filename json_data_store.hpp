#pragma once
#ifndef __JSON_DATA_STORE_HPP__
#define __JSON_DATA_STORE_HPP__

#include <typeinfo>
#include <type_traits>
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

    template<typename StoredType>
    struct DataMember {
        template<typename... Args>
        json_finline void write(Args&&... args) {
            new (&storage) StoredType(std::forward<Args>(args)...);
        }

        json_finline StoredType&& consume() {
            return std::move(*static_cast<StoredType*>(static_cast<void*>(&storage)));
        }

        ~DataMember() {
            static_cast<StoredType*>(static_cast<void*>(&storage))->~StoredType();
        }

        raw_data<StoredType> storage;
    };

    template<typename... Types>
    struct DataList {};

    template<typename StoredType>
    struct DataList<StoredType> {
        DataMember<StoredType> data;
        DataList<> next;
    };

    template<typename StoredType, typename NextType, typename... Types>
    struct DataList<StoredType, NextType, Types...> {
        DataMember<StoredType> data;
        DataList<NextType, Types...> next;
    };

    template<template<typename... M> class ML, typename... DataMembers>
    constexpr auto data_list_type(ML<>&&, DataMembers&&...)
        -> DataList<DataMembers...> {}

    template<typename member, typename... members, template<typename... M> class ML,
             typename... DataMembers>
    constexpr auto data_list_type(ML<member, members...>&&, DataMembers&&... datas)
        -> decltype(data_list_type(ML<members...>(), std::forward<DataMembers>(datas)...,
                                   typename underlying<member>::type())) {}

    template<typename member, typename... members,
             template<typename... M> class ML>
    constexpr auto data_list_type(ML<member, members...>&&)
        -> decltype(data_list_type(ML<members...>(), typename underlying<member>::type())) {}

    template<typename ClassType>
    struct DataStore {
    public:
        /**
         * Data store is no longer valid after this call
         */
        json_finline ClassType realize() {
            return realize(data_list);
        }

        /**
         * Data store is no longer valid after this call
         */
        json_finline void transfer_to(DataMember<ClassType>& into) {
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
            return realize(list.next, std::forward<Values>(values)..., list.data.consume());
        }

        template<typename DataType, typename... DataTypes>
        json_finline ClassType realize(DataList<DataType, DataTypes...>& list) {
            return realize(list.next, list.data.consume());
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
            transfer_to(into, list.next, std::forward<Values>(values)..., list.data.consume());
        }

        template<typename DataType, typename... DataTypes>
        json_finline void transfer_to(DataMember<ClassType>& into,
                                      DataList<DataType, DataTypes...>& list) {
            transfer_to(into, list.next, list.data.consume());
        }

    };
}
}

#endif
