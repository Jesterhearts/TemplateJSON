#pragma once
#ifndef __JSON_HPP__
#define __JSON_HPP__

#include "json_common_macros.hpp"
#include "json_value_parser.hpp"
#include "json_class_parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

/* constructor macros */
#define JSON_CLASS_IMPL(CLASS_NAME, ...)                                                        \
    class CLASS_NAME : public JSON::JSONBase<CLASS_NAME> {                                      \
        /* inject the stuff that we need to function. It shouldn't be public */                 \
        /* These need to be able to hit functions even if private */                            \
        template<class ThisClass>                                                               \
        friend class JSON::JSONBase;                                                            \
                                                                                                \
        template <class ThisClass, const wchar_t *const *classString, size_t offset>            \
        friend struct JSON::VarJSONFnInvoker;                                                   \
                                                                                                \
        template <class ThisClass>                                                              \
        friend struct JSON::TypeJSONFnInvoker;                                                  \
                                                                                                \
        /* This gives us the string for the class that we parse at compile time */              \
        static constexpr const wchar_t* __THIS_JSON_CLASS_STRING__ = WIDEN(#__VA_ARGS__);       \
                                                                                                \
        /* Here we actually make the rest of the class for them */                              \
        __VA_ARGS__                                                                             \
    }                                                                                           \

#define JSON_CLASS_HELPER(...)  \
    JSON_CLASS_IMPL(__VA_ARGS__)

/* Do indirection so macros in the class body get invoked properly */
#define JSON_CLASS(CLASS_NAME, ...)  \
    JSON_CLASS_HELPER(CLASS_NAME, __VA_ARGS__)

#define JSON_VAR_PREAMBLE(TYPE, VARNAME, JSONKEY, KEY, ...)  \
    TYPE VARNAME __VA_ARGS__;                                                               \
    /* This is the thing our tokenizer looks for, KEY gives it the ID for the               \
       specialization.                                                                      \
     */                                                                                     \
    JSON_VAR_DECORATOR KEY = EXPAND_MACRO_WIDEN(KEY);                                       \
    /* prevent duplicate keys */                                                            \
    static constexpr void* __KEYARG_##JSONKEY = nullptr;                                    \

////////////////////////////////////////////////////////////////////////////////////////////////////
//  These macros are for creating basic variables
//  - numeric types, classes, non-array pointers
/////
#define JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, KEY, ...)                                     \
    JSON_VAR_PREAMBLE(TYPE, VARNAME, JSONKEY, KEY, __VA_ARGS__)                             \
    /* This function does the actual work */                                                \
    static void VarToJSON(const JSONBase::__JSONType* classFrom,                            \
                          std::wstring& jsonData,                                           \
                          const JSON::VarToJSONIdentifier<                                  \
                                    JSON::VarNameHasher<&KEY>::Hash()>&& key) {             \
        jsonData += L"\"" #JSONKEY L"\":";                                                  \
        jsonData += JSON::TypeJSONFnInvoker<TYPE>                                           \
                        ::ToJSON(&classFrom->VARNAME);                                      \
    }

/* This makes sure our JSON key (KEY_ARG) for hashing and searching are the same */
#define JSON_VAR_HELPER(TYPE, VARNAME, JSONKEY, KEY_ARG, ...)    \
    JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, KEY_ARG, __VA_ARGS__)

/* Make a variable and use a specific key. The key must be in plain text (not a string).
 */
#define JSON_VAR_AND_KEY(TYPE, VARNAME, JSONKEY, ...)    \
    JSON_VAR_HELPER(TYPE, VARNAME, JSONKEY, MAKE_UNIQUE_VAL(VARNAME), __VA_ARGS__)

/* Make a variable. The varargs will become the the value the variable is initialized to, if
   specified. The variable name becomes the key in JSON
 */
#define JSON_VAR(TYPE, VARNAME, ...)    \
    JSON_VAR_AND_KEY(TYPE, VARNAME, VARNAME, __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  These macros are for creating constant-sized arrays
////
#define JSON_ARRAY_IMPL(TYPE, VARNAME, AR_DIMS, JSONKEY, KEY, ...)                          \
    JSON_VAR_PREAMBLE(TYPE, VARNAME AR_DIMS, JSONKEY, KEY, __VA_ARGS__)                     \
                                                                                            \
    /* This function does the actual work */                                                \
    static void VarToJSON(const JSONBase::__JSONType* classFrom,                            \
                          std::wstring& jsonData,                                           \
                          const JSON::VarToJSONIdentifier<                                  \
                                    JSON::VarNameHasher<&KEY>::Hash()>&& key) {             \
        jsonData += L"\"" #JSONKEY L"\":";                                                  \
        jsonData += JSON::JSONArrayHandler<const TYPE AR_DIMS>                              \
                        ::ToJSON(classFrom->VARNAME);                                       \
    }

#define JSON_ARRAY_HELPER(TYPE, VARNAME, AR_DIMS, JSONKEY, KEY_ARG, ...)    \
    JSON_ARRAY_IMPL(TYPE, VARNAME, AR_DIMS, JSONKEY, KEY_ARG, __VA_ARGS__)

#define JSON_ARRAY_AND_KEY(TYPE, VARNAME, AR_DIMS, JSONKEY, ...) \
    JSON_ARRAY_HELPER(TYPE, VARNAME, AR_DIMS, JSONKEY, MAKE_UNIQUE_VAL(VARNAME), __VA_ARGS__)

#define JSON_ARRAY(TYPE, VARNAME, AR_DIMS, ...) \
    JSON_ARRAY_AND_KEY(TYPE, VARNAME, AR_DIMS, VARNAME, __VA_ARGS__)


#define JSON_VARIABLES(ClassFor, ...)                                         \
    namespace JSON {                                                          \
        const static VariableSet<ClassFor, __VA_ARGS__> __JSON_VARIABLES__;   \
    }                                                                         \

#define __jref(Type_, Attr_)                    \
        JSON::__jref_impl<Type_>(&Type_::Attr_)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  These functions and definitions generate the to/from JSON functions for the class at compile
//  time
////
namespace JSON {
    template<typename C, const wchar_t* const* S> struct ClassJSONFnsInvoker;

    template<typename ClassFor, size_t... VariableDiffs>
    struct VariableSet {
        static constexpr size_t VariableOffsets[sizeof...(VariableDiffs)] = {VariableDiffs...};
    };

    /* Helpers for the template programs */
    template<unsigned int uniqueID>
    struct VarToJSONIdentifier {
        static constexpr unsigned int help = uniqueID;
    };

    template <typename T, typename U>
    constexpr size_t __jref_impl(U T::* a) {
        return (char const*)&(((T*)0)->*a) - (char const*)0;
    }

    typedef std::unordered_map<std::wstring, std::wstring> DataMap;
    typedef std::pair<std::wstring, std::wstring> DataType;

    template<typename ClassFor>
    class JSONBase {
    public:
        std::wstring ToJSON() const {
            return JSON::ClassJSONFnsInvoker<ClassFor,
                                             &ClassFor::__THIS_JSON_CLASS_STRING__
                                            >::InvokeToJSONFns(static_cast<const ClassFor*>(this));
        }
    protected:
        typedef ClassFor __JSONType;
    };

////////////////////////////////////////////////////////////////////////////////
// JSONVarJSONFnInvoker implementation
////
    template<typename classOn,
             const wchar_t *const *classString,
             size_t offset>
    struct VarJSONFnInvoker {
        json_finline inline static void ToJSON(const classOn* classFrom, std::wstring& jsonData) {
            constexpr unsigned int id = VarNameHasher<classString, offset>::Hash();
            classOn::VarToJSON(classFrom, jsonData, JSON::VarToJSONIdentifier<id>());
        }
    };

////////////////////////////////////////////////////////////////////////////////
// ClassJSONFnsBuilder implementation
////
    template<typename ClassFor,
             const wchar_t *const *classInfo,
             size_t offset = 0,
             bool first = false,
             bool started = false,
             bool noMoreVars = false>
    struct ClassJSONFnsBuilder {
        json_finline inline static void BuildFns (const ClassFor* classOn, std::wstring& jsonData) {
            constexpr size_t tokenEnd = ClassParserTokenFinder<classInfo, offset>::FindJSONToken();
            ClassJSONFnsBuilder<ClassFor,
                                classInfo,
                                tokenEnd,
                                true,
                                true,
                                tokenEnd == 0
                               >::BuildFns(classOn, jsonData);
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo,
             size_t offset,
             bool first>
    struct ClassJSONFnsBuilder<ClassFor,
                               classInfo,
                               offset,
                               first,
                               /* started */ true,
                               /* noMoreVars */ false> {
        json_finline inline static void BuildFns (const ClassFor* classOn, std::wstring& jsonData) {
            if(!first) {
                jsonData += L",";
            }
            /* Do the function call */
            constexpr unsigned int varKey = VarNameHasher<classInfo, offset>::Hash();
            VarJSONFnInvoker<ClassFor, classInfo, offset>::ToJSON(classOn, jsonData);

            constexpr size_t tokenEnd = ClassParserTokenFinder<classInfo, offset>::FindJSONToken();
            /* Try to do more calls */
            ClassJSONFnsBuilder<ClassFor,
                                classInfo,
                                tokenEnd,
                                false,
                                true,
                                tokenEnd == 0
                               >::BuildFns(classOn, jsonData);
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo,
             size_t offset,
             bool first>
    struct ClassJSONFnsBuilder<ClassFor,
                               classInfo,
                               offset,
                               first,
                               /* started */ true,
                               /* noMoreVars */ true> {
        json_finline inline static void BuildFns (const ClassFor* classOn, std::wstring& jsonData) {
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo>
    struct ClassJSONFnsInvoker {
        json_finline inline static std::wstring InvokeToJSONFns(const ClassFor* classOn) {
            std::wstring jsonData(L"{");
            ClassJSONFnsBuilder<ClassFor, classInfo>::BuildFns(classOn, jsonData);
            jsonData += L"}";

            return jsonData;
        }
    };
}
#endif
