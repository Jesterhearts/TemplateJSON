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
#include "json_object_hints.hpp"
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

    template<typename>
    struct member_pointer;

    template<typename ClassType, typename UnderlyingType, UnderlyingType ClassType::* member>
    struct member_pointer<MemberInfo<UnderlyingType ClassType::*, member>> {
        constexpr static const UnderlyingType ClassType::* value = member;
    };

    template<typename ClassType>
    struct class_store_tag : reference_only {
        using type = typename std::conditional<
            std::is_same<typename ObjectHints<ClassType>::construction_type,
                         object_hints::non_trivially_constructible>::value,
            data_internal_store_tag,
            data_emplace_store_tag>::type;
    };

    struct data_internal_store_tag : reference_only {};
    //TODO
    struct data_emplace_store_tag : reference_only {};

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
    struct data_storage : initialized_flag<StoredType> {
        raw_data<StoredType> storage;
    };

    template<typename StoredType>
    struct data_storage<StoredType, data_emplace_store_tag> : initialized_flag<StoredType> {
        StoredType* storage;
    };

    template<typename StoredType, typename store_tag>
    struct DataMember : data_storage<StoredType, store_tag> {

        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true>
        json_force_inline DataMember(StoredType* storage) :
            data_storage<StoredType, store_tag>(storage) {}

        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true>
        json_force_inline DataMember() {}

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
        using data_storage<StoredType, store_tag>::storage;

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
    };

    template<typename ClassFor, typename... Types>
    struct DataList {};

    template<typename ClassFor, typename StoredType>
    struct DataList<ClassFor, StoredType> : DataList<ClassFor> {
        DataMember<StoredType, typename class_store_tag<ClassFor>::type> data;
    };

    template<typename ClassFor, typename StoredType, typename NextType, typename... Types>
    struct DataList<ClassFor, StoredType, NextType, Types...> : DataList<ClassFor, NextType, Types...> {

        template<typename store_tag = typename class_store_tag<ClassFor>::type,
                 typename member, typename... members,
                 typename std::enable_if<
                    std::is_same<store_tag, data_emplace_store_tag>::value, bool>::type = true>
        json_force_inline DataList(ClassFor* instance, MemberList<member, members...>&&) :
            DataList<ClassFor, NextType, Types...>(instance, MemberList<members...>()),
            data(instance->*(member_pointer<member>::value))
        {}

        template<typename store_tag = typename class_store_tag<ClassFor>::type,
                 typename std::enable_if<
                    std::is_same<store_tag, data_internal_store_tag>::value, bool>::type = true>
        json_force_inline DataList() {}

        DataMember<StoredType, typename class_store_tag<ClassFor>::type> data;
    };

    /**
     * Used to deduce the type of the DataList required to store the members for a particular class
     */
    template<typename ClassFor, typename... members>
    constexpr DataList<ClassFor,
                       typename underlying<members>::type...>
    data_list_type(MemberList<members...>&&)
    {
        return DataList<ClassFor, typename underlying<members>::type...>();
    }

    /**
     * Used to get the next node in the data list.
     */
    template<typename store_tag, typename DataType, typename... DataTypes>
    json_force_inline
    constexpr DataList<store_tag, DataTypes...>&
    data_list_next(DataList<store_tag, DataType, DataTypes...>& list)
    {
        return static_cast<DataList<store_tag, DataTypes...>&>(list);
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
        template<typename store_tag>
        json_force_inline void transfer_to(DataMember<ClassType, store_tag>& into) {
            transfer_to(into, data_list);
        }

        decltype(data_list_type<ClassType>(MembersHolder<ClassType>::members())) data_list;

    private:
        //For constructing/returning an object
        template<typename store_tag, typename... Values>
        json_force_inline
        ClassType realize(DataList<store_tag>& list,
                          Values&&...          values)
        {
            return ClassType{ std::forward<Values>(values)... };
        }

        template<typename store_tag, typename DataType, typename... DataTypes, typename... Values>
        json_force_inline
        ClassType realize(DataList<store_tag, DataType, DataTypes...>& list,
                          Values&&...                                  values)
        {
            return realize(data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }

        //For initializing a DataMember
        template<typename store_tag, typename list_store_tag, typename... Values>
        json_force_inline void transfer_to(DataMember<ClassType, store_tag>&    into,
                                           DataList<list_store_tag>& list,
                                           Values&&...               values) {
            into.write(std::forward<Values>(values)...);
        }

        template<typename store_tag, typename list_store_tag,
                 typename DataType, typename... DataTypes, typename... Values>
        json_force_inline
        void transfer_to(DataMember<ClassType, store_tag>&                 into,
                         DataList<list_store_tag, DataType, DataTypes...>& list,
                         Values&&...                                       values)
        {
            transfer_to(into, data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }
    };
}
}

#endif
