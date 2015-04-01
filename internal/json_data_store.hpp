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
    struct destroy_storage_flag;

    template<typename ClassType>
    struct destroy_storage_flag<ClassType,
                            typename std::enable_if<
                                !std::is_trivially_destructible<ClassType>::value, bool
                            >::type> {
        json_force_inline constexpr destroy_storage_flag() : initialized(false) {}

        json_force_inline void set_should_destroy_storage() { initialized = true; }
        json_force_inline void unset_should_destroy_storage() { initialized = false; }
        json_force_inline bool should_destroy_storage() { return initialized; }
        json_force_inline bool is_initialized() { return initialized; }

    private:
        bool initialized;
    };

    template<typename ClassType>
    struct destroy_storage_flag<ClassType,
                            typename std::enable_if<
                                std::is_trivially_destructible<ClassType>::value, bool
                            >::type> {
        json_force_inline void set_should_destroy_storage() { }
        json_force_inline void unset_should_destroy_storage() { }
        json_force_inline bool should_destroy_storage() { return false; }
        json_force_inline bool is_initialized() { return true; }
    };

    template<typename StoredType, typename store_tag>
    struct data_member_storage : destroy_storage_flag<StoredType> {
        json_force_inline StoredType* storage_ptr() {
            return static_cast<StoredType*>(static_cast<void*>(&storage));
        }

        raw_data<StoredType> storage;
    };

    template<typename StoredType>
    struct data_member_storage<StoredType, data_emplace_store_tag> : destroy_storage_flag<StoredType> {
        json_force_inline StoredType* storage_ptr() {
            return storage;
        }

        StoredType* storage;
    };

    template<typename StoredType, typename store_tag>
    struct DataMember : data_member_storage<StoredType, store_tag> {

        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true>
        json_force_inline DataMember() {}

        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true>
        json_force_inline DataMember(StoredType* storage) :
            data_member_storage<StoredType, store_tag>(storage) {}

        template<typename... Args>
        json_force_inline void write(Args&&... args) {
            new (storage_ptr()) StoredType{ std::forward<Args>(args)... };
            set_should_destroy_storage();
        }

        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true>
        json_force_inline StoredType&& consume() {
            assert(is_initialized());
            return std::move(*storage_ptr());
        }

        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true>
        json_force_inline void consume() {
            assert(is_initialized());
            unset_should_destroy_storage();
        }

        json_force_inline StoredType& access() {
            assert(is_initialized());
            return *storage_ptr();
        }

        json_force_inline StoredType* get_ptr() {
            return storage_ptr();
        }

        json_force_inline ~DataMember() {
            if(should_destroy_storage()) {
                storage_ptr()->~StoredType();
            }
        }

        using destroy_storage_flag<StoredType>::set_should_destroy_storage;
    private:
        using destroy_storage_flag<StoredType>::is_initialized;
        using destroy_storage_flag<StoredType>::should_destroy_storage;
        using destroy_storage_flag<StoredType>::unset_should_destroy_storage;
        using data_member_storage<StoredType, store_tag>::storage_ptr;
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
            data(&(instance->*(member_pointer<member>::value)))
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

    //See cases outlined in data store
    template<typename StoredType, typename store_tag, typename construct_tag>
    struct data_storage : destroy_storage_flag<StoredType> {
        using storage_ptr = void;
    };

    template<typename StoredType>
    struct data_storage<StoredType,
                        data_internal_store_tag,
                        object_hints::trivially_constructible> : destroy_storage_flag<StoredType>
    {
        json_force_inline StoredType* storage_ptr() {
            return static_cast<StoredType*>(static_cast<void*>(&storage));
        }

        raw_data<StoredType> storage;
    };

    template<typename StoredType>
    struct data_storage<StoredType,
                        data_emplace_store_tag,
                        object_hints::trivially_constructible> : destroy_storage_flag<StoredType>
    {
        json_force_inline StoredType* storage_ptr() {
            return storage;
        }

        StoredType* storage;
    };

    template<typename ClassType, typename store_tag = data_internal_store_tag>
    struct DataStore : data_storage<ClassType,
                                    store_tag,
                                    typename ObjectHints<ClassType>::construction_type> {
    public:

        /* Four cases:
         * 1. We are emplacing a trivially constructible type:
         *    - Build the items in-place in the passed-in object
         *    - Do nothing in transfer_to
         *    - realize() is not a valid operation
         *    - Do nothing in the destructor
         *
         * 2. We are emplacing a non-trivially constructible type:
         *    - Build the data list out of separate objects
         *    - Construct the target object during transfer_to
         *    - realize() is not a valid operation
         *    - Do nothing in destructor (tear down is handled by data list/owner of target)
         *
         * 3. We are constructing a trivially constructible type (no emplacement):
         *    - Build the items in-place in an internal buffer
         *    - return the internal buffer during realize()
         *    - transfer_to is not a valid operation
         *    - call destructor if realize() was invoked (data list teardown handles partial objs)
         *
         * 4. We are constructing a non-trivially constructible type (no emplacement):
         *    - Build the data list from separate objects
         *    - Construct the object and return it during realize()
         *    - transfer_to is not a valid operation
         *    - do nothing in destructor
         */

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true>
        json_force_inline DataStore() {}

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::trivially_constructible>::value, bool>::type = true>
        json_force_inline DataStore(ClassType* storage) :
            data_storage<ClassType,
                         store_tag,
                         typename ObjectHints<ClassType>::construction_type
            >(storage) {}

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::non_trivially_constructible>::value, bool>::type = true>
        json_force_inline DataStore(ClassType* storage) {}
        /**
         * Data store is no longer valid after this call
         */
        template<typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true>
        json_force_inline ClassType realize() {
            return realize(data_list);
        }

        /**
         * Data store is no longer valid after this call
         */
        template<typename member_store_tag,
                 typename _store_tag = store_tag,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true>
        json_force_inline void transfer_to(DataMember<ClassType, member_store_tag>& into) {
            transfer_to(into, data_list);
        }

        json_force_inline ~DataStore() {
            DestroyStorage();
        }

        decltype(data_list_type<ClassType>(MembersHolder<ClassType>::members())) data_list;

    private:
        using data_storage<ClassType,
                           store_tag,
                           typename ObjectHints<ClassType>::construction_type
                          >::storage_ptr;
        using destroy_storage_flag<ClassType>::should_destroy_storage;
        using destroy_storage_flag<ClassType>::set_should_destroy_storage;

        //For constructing/returning an object
        //Creating a non trivially constructible type
        template<typename member_store_tag,
                 typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::non_trivially_constructible>::value, bool>::type = true>
        json_force_inline
        ClassType realize(DataList<member_store_tag>& list,
                          Values&&...          values)
        {
            return ClassType{ std::forward<Values>(values)... };
        }

        template<typename member_store_tag,
                 typename DataType,
                 typename... DataTypes,
                 typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::non_trivially_constructible>::value, bool>::type = true>
        json_force_inline
        ClassType realize(DataList<member_store_tag, DataType, DataTypes...>& list,
                          Values&&...                                  values)
        {
            return realize(data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }

        //Creating a trivially constructible type
        template<typename member_store_tag,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::trivially_constructible>::value, bool>::type = true>
        json_force_inline
        ClassType realize(DataList<member_store_tag>& list)
        {
            set_should_destroy_storage();
            return std::move(*storage_ptr());
        }

        template<typename member_store_tag,
                 typename DataType,
                 typename... DataTypes,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::trivially_constructible>::value, bool>::type = true>
        json_force_inline
        ClassType realize(DataList<member_store_tag, DataType, DataTypes...>& list)
        {
            list.data.consume();
            return realize(data_list_next(list));
        }

        //For initializing a DataMember
        //Transferring a non trivially constructible type
        template<typename member_store_tag,
                 typename list_store_tag,
                 typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::non_trivially_constructible>::value, bool>::type = true>
        json_force_inline
        void transfer_to(DataMember<ClassType, member_store_tag>& into,
                         DataList<list_store_tag>&                list,
                         Values&&...                              values)
        {
            into.write(std::forward<Values>(values)...);
        }

        template<typename member_store_tag,
                 typename list_store_tag,
                 typename DataType,
                 typename... DataTypes,
                 typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::non_trivially_constructible>::value, bool>::type = true>
        json_force_inline
        void transfer_to(DataMember<ClassType, member_store_tag>&          into,
                         DataList<list_store_tag, DataType, DataTypes...>& list,
                         Values&&...                                       values)
        {
            transfer_to(into, data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }

        //Transferring a trivially constructible type
        template<typename member_store_tag,
                 typename list_store_tag,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::trivially_constructible>::value, bool>::type = true>
        json_force_inline
        void transfer_to(DataMember<ClassType, member_store_tag>& into,
                         DataList<list_store_tag>&                list)
        {
            //we no longer handle cleanup of the created value, so turn it over to the data list
            into.set_should_destroy_storage();
        }

        template<typename member_store_tag,
                 typename list_store_tag,
                 typename DataType,
                 typename... DataTypes,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value, bool>::type = true,
                 typename std::enable_if<
                    std::is_same<construct_tag,
                                 object_hints::trivially_constructible>::value, bool>::type = true>
        json_force_inline
        void transfer_to(DataMember<ClassType, member_store_tag>&          into,
                         DataList<list_store_tag, DataType, DataTypes...>& list)
        {
            list.data.consume();
            transfer_to(into, data_list_next(list));
        }

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_emplace_store_tag>::value
                    || std::is_same<construct_tag,
                                    object_hints::non_trivially_constructible>::value, bool>::type = true>
        json_force_inline void DestroyStorage() {}

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ObjectHints<ClassType>::construction_type,
                 typename std::enable_if<
                    std::is_same<_store_tag, data_internal_store_tag>::value
                    && std::is_same<construct_tag,
                                    object_hints::trivially_constructible>::value, bool>::type = true>
        json_force_inline void DestroyStorage() {
            if(should_destroy_storage()) {
                storage_ptr()->~ClassType();
            }
        }
    };
}
}

#endif
