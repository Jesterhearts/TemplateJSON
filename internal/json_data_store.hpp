#pragma once
#ifndef __JSON_DATA_STORE_HPP__
#define __JSON_DATA_STORE_HPP__

#include <cassert>
#include <cstring>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "json_functions.hpp"
#include "json_object_hints.hpp"
#include "json_common_defs.hpp"
#include "json_internal_declarations.hpp"
#include "json_member_mapper.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4291)
#endif
//TODO: Pretty much none of this file will play nice with custom allocators. Need to add support
//  later

//The standard mandates that calls to a non-throwing new check for null. The default placement new
//  inserts a null check and jump after every call, which is unnecessary since we know that the
//  pointer is not null. This makes gcc remove the null check.
json_force_inline
void* operator new(size_t, void*, tjson::detail::placement_new) json_return_nonull;

json_force_inline
void* operator new(size_t, void* ptr, tjson::detail::placement_new) {
    return ptr;
}

namespace tjson {
namespace detail {

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
        json_force_inline static UnderlyingType* apply(ClassType* instance) {
            return &(instance->*member);
        }
    };

    struct data_internal_store_tag : reference_only {};
    struct data_emplace_store_tag : reference_only {};

    template<typename store_tag>
    struct is_internal_store_tag : reference_only {
       constexpr static const bool value = std::is_same<store_tag, data_internal_store_tag>::value;
    };

    template<typename store_tag>
    struct is_emplace_store_tag : reference_only {
       constexpr static const bool value = std::is_same<store_tag, data_emplace_store_tag>::value;
    };

    template<typename construct_tag>
    struct is_non_trivial_construct_tag : reference_only {
       constexpr static const bool value = std::is_same<construct_tag, object_hints::non_trivially_constructible>::value;
    };

    template<typename construct_tag>
    struct is_trivial_construct_tag : reference_only {
       constexpr static const bool value = std::is_same<construct_tag, object_hints::trivially_constructible>::value;
    };

    template<typename ClassType>
    struct class_store_tag : reference_only {
        /**
         * Selects emplace store if the class is trivially constructible, otherwise selects internal
         * store so that the member can be built and copied/moved over
         */
        using type = typename std::conditional<
            is_non_trivial_construct_tag<typename ConstructHint<ClassType>::construction_type>::value,
            data_internal_store_tag,
            data_emplace_store_tag>::type;
    };

    template<typename, typename enabled = bool>
    struct destroy_storage_flag;

    template<typename ClassType>
    struct destroy_storage_flag<ClassType,
                            typename std::enable_if<
                                !std::is_trivially_destructible<ClassType>::value, bool
                            >::type>
    {
        json_force_inline constexpr destroy_storage_flag() noexcept : initialized(false) {}

        json_force_inline void set_should_destroy_storage() noexcept { initialized = true; }
        json_force_inline void unset_should_destroy_storage() noexcept { initialized = false; }
        json_force_inline bool should_destroy_storage() noexcept { return initialized; }
        json_force_inline bool is_initialized() noexcept { return initialized; }

    private:
        bool initialized;
    };

    template<typename ClassType>
    struct destroy_storage_flag<ClassType,
                            typename std::enable_if<
                                std::is_trivially_destructible<ClassType>::value, bool
                            >::type>
    {
        json_force_inline void set_should_destroy_storage() noexcept { }
        json_force_inline void unset_should_destroy_storage() noexcept { }
        json_force_inline bool should_destroy_storage() noexcept { return false; }
        json_force_inline bool is_initialized() noexcept { return true; }
    };

    template<typename StoredType>
    struct data_member_storage_base : destroy_storage_flag<StoredType> {

        json_force_inline data_member_storage_base(StoredType* storage) noexcept :
            _storage_ptr(storage)
        {}

        json_force_inline StoredType* storage_ptr() json_return_nonull { return _storage_ptr; }
        StoredType* _storage_ptr;
    };

    template<typename StoredType>
    struct DataMember : data_member_storage_base<StoredType> {
        json_force_inline DataMember(StoredType* storage) noexcept :
            data_member_storage_base<StoredType>(storage)
        {}

        template<typename... Args,
                 typename std::enable_if<!std::is_nothrow_constructible<StoredType, Args...>::value,
                    bool>::type = true>
        json_force_inline void write(Args&&... args) {
            new (storage_ptr(), placement_new::invoke) StoredType{ std::forward<Args>(args)... };
            //This must happen AFTER calling new, since if that throws we don't want to try to tear
            //  down a class that hasn't been built yet
            set_should_destroy_storage();
        }

        template<typename... Args,
                 typename std::enable_if<std::is_nothrow_constructible<StoredType, Args...>::value,
                    bool>::type = true>
        json_force_inline void write(Args&&... args) noexcept {
            new (storage_ptr(), placement_new::invoke) StoredType{ std::forward<Args>(args)... };
            set_should_destroy_storage();
        }

        using destroy_storage_flag<StoredType>::set_should_destroy_storage;
        using data_member_storage_base<StoredType>::storage_ptr;
    };

    template<typename StoredType, typename store_tag>
    struct data_member_storage : DataMember<StoredType> {
        json_force_inline data_member_storage() noexcept :
            DataMember<StoredType>(static_cast<StoredType*>(static_cast<void*>(&storage)))
        {}

        raw_data<StoredType> storage;
    };

    template<typename StoredType>
    struct data_member_storage<StoredType, data_emplace_store_tag> : DataMember<StoredType> {
        json_force_inline data_member_storage(StoredType* storage) noexcept :
            DataMember<StoredType>(storage)
        {}
    };

    template<typename StoredType, typename store_tag>
    struct DataMemberImpl : data_member_storage<StoredType, store_tag> {

        template<typename _store_tag = store_tag,
                 enable_if<_store_tag, is_internal_store_tag> = true>
        json_force_inline DataMemberImpl() noexcept {}

        template<typename _store_tag = store_tag,
                 enable_if<_store_tag, is_emplace_store_tag> = true>
        json_force_inline DataMemberImpl(StoredType* storage) noexcept :
            data_member_storage<StoredType, store_tag>(storage)
        {}

        template<typename _store_tag = store_tag,
                 enable_if<_store_tag, is_internal_store_tag> = true>
        json_force_inline StoredType&& consume() noexcept {
            assert(is_initialized());
            return std::move(*storage_ptr());
        }

        template<typename _store_tag = store_tag,
                 enable_if<_store_tag, is_emplace_store_tag> = true>
        json_force_inline void consume() noexcept {
            assert(is_initialized());
            unset_should_destroy_storage();
        }

        json_force_inline StoredType& access() noexcept {
            assert(is_initialized());
            return *storage_ptr();
        }

        json_force_inline ~DataMemberImpl() {
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

    //TODO: This stores members in the "wrong" order, since they are reverse-order in memory
    //  I *think* this doesn't matter since it looks like from the assembly the compiler just
    //  removes this class because of all the inlining and builds directly on the members, making
    //  the member memory-order irrelevant.
    //As long as there is no ##% performance difference in changing the order, this will stay as-is
    //  since it's much easier to understand/reason about
    template<typename ClassFor, typename... Types>
    struct DataList {};

    template<typename ClassFor, typename StoredType>
    struct DataList<ClassFor, StoredType> : DataList<ClassFor> {

        template<typename store_tag = typename class_store_tag<ClassFor>::type,
                 typename member, typename... members,
                 enable_if<store_tag, is_emplace_store_tag> = true>
        json_force_inline DataList(ClassFor* instance, MemberList<member, members...>&&) noexcept :
            data(member_pointer<member>::apply(instance))
        {}

        template<typename store_tag = typename class_store_tag<ClassFor>::type,
                 enable_if<store_tag, is_internal_store_tag> = true>
        json_force_inline DataList() noexcept {}

        DataMemberImpl<StoredType, typename class_store_tag<ClassFor>::type> data;
    };

    template<typename ClassFor, typename StoredType, typename NextType, typename... Types>
    struct DataList<ClassFor, StoredType, NextType, Types...> : DataList<ClassFor,
                                                                         NextType,
                                                                         Types...>
    {
        template<typename store_tag = typename class_store_tag<ClassFor>::type,
                 typename member,
                 typename... members,
                 enable_if<store_tag, is_emplace_store_tag> = true>
        json_force_inline DataList(ClassFor* instance, MemberList<member, members...>&&) noexcept :
            DataList<ClassFor, NextType, Types...>(instance, MemberList<members...>()),
            data(member_pointer<member>::apply(instance))
        {}

        template<typename store_tag = typename class_store_tag<ClassFor>::type,
                 enable_if<store_tag, is_internal_store_tag> = true>
        json_force_inline DataList() noexcept {}

        DataMemberImpl<StoredType, typename class_store_tag<ClassFor>::type> data;
    };

    /**
     * Used to deduce the type of the DataList required to store the members for a particular class
     */
    template<typename ClassFor, typename... members,
             typename std::enable_if<
                std::is_same<typename class_store_tag<ClassFor>::type,
                             data_internal_store_tag>::value, bool>::type = true>
    constexpr DataList<ClassFor, typename underlying<members>::type...>
    data_list_type(MemberList<members...>&&) noexcept {
        return DataList<ClassFor, typename underlying<members>::type...>();
    }

    template<typename ClassFor, typename... members,
             typename std::enable_if<
                std::is_same<typename class_store_tag<ClassFor>::type,
                             data_emplace_store_tag>::value, bool>::type = true>
    DataList<ClassFor, typename underlying<members>::type...>
    data_list_type(MemberList<members...>&&) noexcept {
        return DataList<ClassFor, typename underlying<members>::type...>(
            nullptr, MemberList<members...>()
        );
    }

    /**
     * Used to get the next node in the data list.
     */
    template<typename ClassFor, typename DataType, typename... DataTypes>
    json_force_inline
    constexpr DataList<ClassFor, DataTypes...>&
    data_list_next(DataList<ClassFor, DataType, DataTypes...>& list) noexcept {
        return static_cast<DataList<ClassFor, DataTypes...>&>(list);
    }

    template<typename StoredType>
    struct DataStore : destroy_storage_flag<StoredType> {

        template<typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline DataStore(StoredType* storage) noexcept
            : data_list(storage, MembersHolder<StoredType>::members())
        {}

        template<typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline DataStore() noexcept {}

        decltype(data_list_type<StoredType>(MembersHolder<StoredType>::members())) data_list;
    };

    //See cases outlined in data store
    template<typename StoredType, typename store_tag, typename construct_tag>
    struct data_storage : DataStore<StoredType> {
        using storage_ptr = void;
    };

    template<typename StoredType>
    struct data_storage<StoredType,
                        data_internal_store_tag,
                        object_hints::trivially_constructible> : DataStore<StoredType>
    {
        json_force_inline data_storage() noexcept :
            DataStore<StoredType>(storage_ptr())
        {}

        json_force_inline StoredType* storage_ptr() noexcept json_return_nonull {
            return static_cast<StoredType*>(static_cast<void*>(&storage));
        }

        raw_data<StoredType> storage;
    };

    template<typename StoredType, typename construct_tag>
    struct data_storage<StoredType,
                        data_emplace_store_tag,
                        construct_tag> : DataStore<StoredType>
    {
        template<typename _construct_tag = construct_tag,
                 enable_if<_construct_tag, is_trivial_construct_tag> = true>
        json_force_inline data_storage(StoredType* storage) noexcept :
            DataStore<StoredType>(storage),
            storage(storage)
        {}

        template<typename _construct_tag = construct_tag,
                 enable_if<_construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline data_storage(StoredType* storage) noexcept :
            storage(storage)
        {}

        json_force_inline StoredType* storage_ptr() json_return_nonull {
            return storage;
        }

        StoredType* storage;
    };

    template<typename StoredType, typename store_tag = data_internal_store_tag>
    struct DataStoreImpl : data_storage<StoredType,
                                        store_tag,
                                        typename ConstructHint<StoredType>::construction_type>
    {
        /* Four cases:
         * 1. We are emplacing a trivially constructible type:
         *    - Build the items in-place in the passed-in object
         *    - Do nothing in transfer_storage, just flag that owning object will do teardown
         *    - realize() is not a valid operation
         *    - Do nothing in the destructor, teardown is now owner's problem
         *
         * 2. We are emplacing a non-trivially constructible type:
         *    - Build the data list out of separate objects
         *    - Construct the target object during transfer_storage
         *    - realize() is not a valid operation
         *    - Do nothing in destructor (tear down is handled by data list/owner of target)
         *
         * 3. We are constructing a trivially constructible type (no emplacement):
         *    - Build the items in-place in an internal buffer
         *    - return the internal buffer during realize()
         *    - transfer_storage is not a valid operation
         *    - call destructor if realize() was invoked (data list teardown handles partial objs)
         *
         * 4. We are constructing a non-trivially constructible type (no emplacement):
         *    - Build the data list from separate objects
         *    - Construct the object and return it during realize()
         *    - transfer_storage is not a valid operation
         *    - do nothing in destructor
         */

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_internal_store_tag> = true>
        json_force_inline DataStoreImpl() noexcept {}

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_emplace_store_tag> = true,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline DataStoreImpl(StoredType* storage) noexcept :
            data_storage<StoredType, store_tag, construct_tag>(storage)
        {}

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_emplace_store_tag> = true,
                 enable_if<construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline DataStoreImpl(StoredType* storage) noexcept :
            data_storage<StoredType, store_tag, construct_tag>(storage)
        {}

        /**
         * Data store is no longer valid after this call
         */
        template<typename _store_tag = store_tag,
                 enable_if<_store_tag, is_internal_store_tag> = true>
        json_force_inline StoredType realize() {
            return realize(data_list);
        }

        /**
         * Data store is no longer valid after this call
         */
        template<typename _store_tag = store_tag,
                 enable_if<_store_tag, is_emplace_store_tag> = true>
        json_force_inline void transfer_storage(DataMember<StoredType>& into) {
            assert(into.storage_ptr() == storage_ptr());
            transfer_storage(data_list);
            into.set_should_destroy_storage();
        }

        json_force_inline ~DataStoreImpl() {
            DestroyStorage();
        }

        using DataStore<StoredType>::data_list;

    private:
        using data_storage<StoredType,
                           store_tag,
                           typename ConstructHint<StoredType>::construction_type
                          >::storage_ptr;
        using destroy_storage_flag<StoredType>::should_destroy_storage;
        using destroy_storage_flag<StoredType>::set_should_destroy_storage;

        //For constructing/returning an object
        //Creating a non trivially constructible type
        template<typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_internal_store_tag> = true,
                 enable_if<construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline
        static StoredType realize(DataList<StoredType>&,
                                  Values&&... values)
        {
            return StoredType{ std::forward<Values>(values)... };
        }

        template<typename DataType,
                 typename... DataTypes,
                 typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_internal_store_tag> = true,
                 enable_if<construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline
        static StoredType realize(DataList<StoredType, DataType, DataTypes...>& list,
                                  Values&&...                                   values)
        {
            return realize(data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }

        //Creating a trivially constructible type
        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_internal_store_tag> = true,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline
        StoredType realize(DataList<StoredType>&) noexcept {
            //class has been fully created at this point, need to make sure to call destructor after
            //  calling move
            set_should_destroy_storage();
            return std::move(*storage_ptr());
        }

        template<typename DataType,
                 typename... DataTypes,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_internal_store_tag> = true,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline
        StoredType realize(DataList<StoredType, DataType, DataTypes...>& list) noexcept {
            //Need to flag all datalist members as non-destroying, since we'll handle that after
            //  this point. This puts it in a temporary partial-destroying state, but it should be
            //  safe since no exceptions can be thrown in the middle of this process.
            //This consume is different than the consume for a non-trivial type, since that still
            //  needs to call the destructor
            list.data.consume();
            return realize(data_list_next(list));
        }

        //For initializing a DataMember
        //Transferring a non trivially constructible type
        template<typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_emplace_store_tag> = true,
                 enable_if<construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline
        void transfer_storage(DataList<StoredType>&,
                              Values&&... values)
        {
            new (storage_ptr(), placement_new::invoke) StoredType{std::forward<Values>(values)...};
        }

        template<typename DataType,
                 typename... DataTypes,
                 typename... Values,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_emplace_store_tag> = true,
                 enable_if<construct_tag, is_non_trivial_construct_tag> = true>
        json_force_inline
        void transfer_storage(DataList<StoredType, DataType, DataTypes...>& list,
                              Values&&...                                   values)
        {
            transfer_storage(data_list_next(list), std::forward<Values>(values)..., list.data.consume());
        }

        //Transferring a trivially constructible type
        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_emplace_store_tag> = true,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline
        static void transfer_storage(DataList<StoredType>&) noexcept{}

        template<typename DataType,
                 typename... DataTypes,
                 typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_emplace_store_tag> = true,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline
        static void transfer_storage(DataList<StoredType, DataType, DataTypes...>& list) noexcept{
            list.data.consume();
            transfer_storage(data_list_next(list));
        }

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 typename std::enable_if<
                    is_emplace_store_tag<_store_tag>::value
                    || is_non_trivial_construct_tag<construct_tag>::value, bool>::type = true>
        json_force_inline void DestroyStorage() noexcept {}

        template<typename _store_tag = store_tag,
                 typename construct_tag = typename ConstructHint<StoredType>::construction_type,
                 enable_if<_store_tag, is_internal_store_tag> = true,
                 enable_if<construct_tag, is_trivial_construct_tag> = true>
        json_force_inline void DestroyStorage() noexcept {
            if(should_destroy_storage()) {
                storage_ptr()->~StoredType();
            }
        }
    };
}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
