#pragma once
#ifndef __JSON_PARSING_HELPERS_HPP__
#define __JSON_PARSING_HELPERS_HPP__

#include <stdexcept>

namespace JSON {
    const std::string nullToken("null");

    namespace detail {
        template<typename ClassType>
        using basic_type = typename std::remove_reference<ClassType>::type;

        template<typename ClassType>
        using non_const = typename std::enable_if<!std::is_const<basic_type<ClassType>>::value, ClassType>::type;

        template<typename ClassType, template<typename C> class decider>
        using enable_if = typename std::enable_if<decider<non_const<ClassType>>::value, bool>::type;

        template<typename ClassType>
        using enable_if_const = typename std::enable_if<std::is_const<basic_type<ClassType>>::value, bool>::type;

        template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
        void ToJSON(ClassType from, std::string& out);

        template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_integral> = true>
        void ToJSON(ClassType from, std::string& out);

        template<typename ClassType, enable_if<ClassType, std::is_integral> = true>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
        void ToJSON(ClassType from, std::string& out);

        template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
        void ToJSON(ClassType from, std::string& out);

        template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_array> = true>
        void ToJSON(ClassType& from, std::string& out);

        template<typename ClassType, enable_if<ClassType, std::is_array> = true>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);

        template<typename ClassType, enable_if<ClassType, std::is_class> = true>
        void ToJSON(const ClassType& from, std::string& out);

        template<typename ClassType, enable_if<ClassType, std::is_class> = true>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);

        template<typename ClassType, enable_if_const<ClassType> = true>
        json_deserialize_const_warning
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into);
    }

    json_no_return inline void ThrowBadJSONError(jsonIter iter, jsonIter end,
                                                 std::string&& errmsg) {
        jsonIter endIter = (std::distance(iter, end) > 1000) ? iter + 1000 : end;
        std::string badJson(iter, endIter);
        throw std::invalid_argument(errmsg + ": " + badJson);
    }

    inline jsonIter AdvancePastWhitespace(jsonIter iter, jsonIter end) {
        while(iter != end && std::isspace(*iter)) {
            ++iter;
        }

        return iter;
    }

    inline jsonIter AdvancePastNumbers(jsonIter iter, jsonIter end) {
        if(iter == end) {
            return iter;
        }

        if(*iter == L'-') {
            ++iter;
        }

        while(iter != end && std::isdigit(*iter)) {
            ++iter;
        }

        if(iter == end) {
            return iter;
        }

        if(*iter == '.') {
            ++iter;
        }

        while(iter != end && std::isdigit(*iter)) {
            ++iter;
        }

        if(iter == end) {
            return iter;
        }

        if(*iter == 'e') {
            ++iter;
        }

        while(iter != end && std::isdigit(*iter)) {
            ++iter;
        }

        return iter;
    }

    inline jsonIter AdvanceToEndOfString(jsonIter iter, jsonIter end) {
        bool escaping = true;

        while(iter != end) {
            if(*iter == L'\\' || escaping) {
                escaping = !escaping;
            }
            else if(!escaping && *iter == L'\"') {
                return iter;
            }

            ++iter;
        }

        return iter;
    }

    inline jsonIter ValidateObjectStart(jsonIter iter, jsonIter end) {
        iter = AdvancePastWhitespace(iter, end);
        if(iter == end || *iter != '{') {
            ThrowBadJSONError(iter, end, "No object start token");
        }
        ++iter;

        return iter;
    }

    inline jsonIter ValidateObjectEnd(jsonIter iter, jsonIter end) {
        iter = AdvancePastWhitespace(iter, end);
        if(iter == end || *iter != '}') {
            ThrowBadJSONError(iter, end, "No object end token");
        }
        ++iter;
        return iter;
    }

    inline jsonIter ValidateKeyValueMapping(jsonIter iter, jsonIter end) {
        iter = AdvancePastWhitespace(iter, end);
        if(iter == end || *iter != ':') {
            ThrowBadJSONError(iter, end, "Not a valid key-value mapping");
        }
        ++iter;
        return iter;
    }

    json_finline jsonIter ParseNextKey(jsonIter iter, jsonIter end, std::string& nextKey) {
        return detail::FromJSON(iter, end, nextKey);
    }
}
#endif
