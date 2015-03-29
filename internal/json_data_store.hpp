#pragma once
#ifndef __JSON_DATA_STORE_HPP__
#define __JSON_DATA_STORE_HPP__

#include <cassert>
#include <cstring>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "json_common_defs.hpp"
#include "json_functions.hpp"
#include "json_internal_declarations.hpp"
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
        using type = basic_type<UnderlyingType>;
    };

    struct data_internal_store_tag : reference_only {};
    //TODO
    struct data_external_store_tag : reference_only {};

    template<typename, typename enabled = bool>
    struct initialized_flag;

    template<typename ClassType>
    struct initialized_flag<ClassType,
                            typename std::enable_if<
                                !std::is_trivially_destructible<ClassType>::value, bool
                            >::type> {
        json_force_inline constexpr initialized_flag() : initialized(false) {}

        json_force_inline void set_initialized() { initialized = true; }
        json_force_inline bool is_initialized() { return initialized; }

    private:
        bool initialized;
    };

    template<typename ClassType>
    struct initialized_flag<ClassType,
                            typename std::enable_if<
                                std::is_trivially_destructible<ClassType>::value, bool
                            >::type> {
        json_force_inline void set_initialized() { }
        json_force_inline bool is_initialized() { return true; }
    };

    template<typename StoredType, typename store_tag>
    struct DataMember : initialized_flag<StoredType> {

        template<typename... Args>
        json_force_inline void write(Args&&... args) {
            new (&storage) StoredType{ std::forward<Args>(args)... };
            set_initialized();
        }

        json_force_inline StoredType&& consume() {
            assert(is_initialized());
            return std::move(*static_cast<StoredType*>(static_cast<void*>(&storage)));
        }

        json_force_inline StoredType& access() {
            assert(is_initialized());
            return *static_cast<StoredType*>(static_cast<void*>(&storage));
        }

        json_force_inline ~DataMember() {
            DestroyStorage<StoredType>();
        }

    private:
        using initialized_flag<StoredType>::is_initialized;
        using initialized_flag<StoredType>::set_initialized;

        template<typename Destroying,
                 typename std::enable_if<
                    !std::is_trivially_destructible<Destroying>::value, bool>
                 ::type = true>
        json_force_inline void DestroyStorage() {
            if(is_initialized()) {
                static_cast<StoredType*>(static_cast<void*>(&storage))->~StoredType();
            }
        }

        template<typename Destroying,
                 typename std::enable_if<
                    std::is_trivially_destructible<Destroying>::value, bool>
                 ::type = true>
        json_force_inline void DestroyStorage() { }

        raw_data<StoredType> storage;
    };

    template<typename... Types>
    struct DataList {};

    template<typename StoredType>
    struct DataList<StoredType> : DataList<> {
        DataMember<StoredType> data;
    };

    template<typename StoredType, typename NextType, typename... Types>
    struct DataList<StoredType, NextType, Types...> : DataList<NextType, Types...> {
        DataMember<StoredType> data;
    };

    /**
     * Used to deduce the type of the DataList required to store the members for a particular class
     */
    template<typename... members>
    constexpr DataList<typename underlying<members>::type...>
    data_list_type(MemberList<members...>&&) {
        return DataList<typename underlying<members>::type...>();
    }

    /**
     * Used to get the next node in the data list.
     */
    template<typename DataType, typename... DataTypes>
    constexpr DataList<DataTypes...>& data_list_next(DataList<DataType, DataTypes...>& list) {
        return static_cast<DataList<DataTypes...>&>(list);
    }

    template<typename ClassType>
    struct DataStore {
    public:
        /**
         * Data store is no longer valid after this call
         */
        json_force_inline ClassType realize() {
            return realize(data_list);
        }

        /**
         * Data store is no longer valid after this call
         */
        json_force_inline void transfer_to(DataMember<ClassType>& into) {
            transfer_to(into, data_list);
        }

        decltype(data_list_type(MembersHolder<ClassType>::members())) data_list;

    private:
        //For constructing/returning an object
        template<typename... Values>
        json_force_inline ClassType realize(DataList<>& list,
                                            Values&&... values) {
            return ClassType{ std::forward<Values>(values)... };
        }

        template<typename DataType, typename... DataTypes, typename... Values>
        json_force_inline ClassType realize(DataList<DataType, DataTypes...>& list,
                                            Values&&...                       values) {
            return realize(data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }

        //For initializing a DataMember
        template<typename... Values>
        json_force_inline void transfer_to(DataMember<ClassType>& into,
                                           DataList<>&            list,
                                           Values&&...            values) {
            into.write(std::forward<Values>(values)...);
        }

        template<typename DataType, typename... DataTypes, typename... Values>
        json_force_inline void transfer_to(DataMember<ClassType>&         into,
                                        DataList<DataType, DataTypes...>& list,
                                        Values&&...                       values) {
            transfer_to(into, data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }
    };
}
}

#endif
