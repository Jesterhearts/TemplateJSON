#pragma once
#ifndef __JSON_HPP_
#define __JSON_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "json_common_macros.hpp"
#include "json_class_parser.hpp"
#include "json_value_parser.hpp"

/* Don't put this anywhere in your class definition unless you like reading horrific template
   errors */
#define JSON_VAR_DECORATOR \
    static constexpr JSON::__json_var

/* constructor macros */
#define JSON_CLASS_IMPL(CLASS_NAME, ...)                                                        \
    class CLASS_NAME {                                                                          \
        typedef CLASS_NAME __THIS_JSON_CLASS__;                                                 \
        /* inject the stuff that we need to function. It shouldn't be public */                 \
        /* These need to be able to hit functions even if private */                            \
        template <class ThisClass, const wchar_t *const *classString, size_t offset>            \
        friend struct JSON::VarJSONFnInvoker;                                                 \
                                                                                                \
        template <class ThisClass>                                                              \
        friend struct JSON::TypeJSONFnInvoker;                                                \
                                                                                                \
        /* This gives us the string for the class that we parse at compile time */              \
        static constexpr const wchar_t* __##CLASS_NAME = WIDEN(#__VA_ARGS__);                   \
                                                                                                \
    public:                                                                                     \
        std::wstring ToJSON() const {                                                           \
            return JSON::ClassJSONFnsInvoker<CLASS_NAME, &__##CLASS_NAME>                       \
                            ::InvokeToJSONFns(*this);                                           \
        }                                                                                       \
    private:                                                                                    \
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


#define JSON_VAR_IMPL(TYPE, VARNAME, JSONKEY, KEY, ...)                                     \
    JSON_VAR_PREAMBLE(TYPE, VARNAME, JSONKEY, KEY, __VA_ARGS__)                             \
                                                                                            \
    /* This function does the actual work */                                                \
    static void VarToJSON(const __THIS_JSON_CLASS__& classFrom,                             \
                          std::wstring& jsonData,                                           \
                          const JSON::VarToJSONIdentifier<                                  \
                                    JSON::VarNameHasher<&KEY>::Hash()>&& key) {             \
        jsonData += L"\"" #JSONKEY L"\":";                                                  \
        jsonData += JSON::TypeJSONFnInvoker<TYPE>                                         \
                        ::ToJSON(static_cast<TYPE>(classFrom.VARNAME));                     \
    }                                                                                       \

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


/* holy shit so many templates */
namespace JSON {

    typedef std::unordered_map<std::wstring, std::wstring> DataMap;
    typedef std::pair<std::wstring, std::wstring> DataType;

    /* Makes a unique fn identifier */
    template<unsigned int uniqueID>
    struct VarToJSONIdentifier {
        static constexpr unsigned int help = uniqueID;
    };

////////////////////////////////////////////////////////////////////////////////
// JSONVarJSONFnInvoker implementation
////
    template<typename classOn,
             const wchar_t *const *classString,
             size_t offset>
    struct VarJSONFnInvoker {
        inline static void ToJSON(const classOn& classFrom, std::wstring& jsonData) json_finline {
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
        inline static void BuildFns (const ClassFor& classOn, std::wstring& jsonData) json_finline {
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
        inline static void BuildFns (const ClassFor& classOn, std::wstring& jsonData) json_finline {
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
        inline static void BuildFns (const ClassFor& classOn, std::wstring& jsonData) json_finline {
        }
    };

    template<typename ClassFor,
             const wchar_t *const *classInfo>
    struct ClassJSONFnsInvoker {
        inline static std::wstring InvokeToJSONFns(const ClassFor& classOn) json_finline {
            std::wstring jsonData(L"{");
            ClassJSONFnsBuilder<ClassFor, classInfo>::BuildFns(classOn, jsonData);
            jsonData += L"}";

            return jsonData;
        }
    };
}
#endif
