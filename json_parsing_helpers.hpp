#pragma once
#ifndef __JSON_PARSING_HELPERS_HPP__
#define __JSON_PARSING_HELPERS_HPP__

namespace JSON {
    const std::string nullToken("null");

    namespace detail {
        template<typename ClassType, typename tag>
        std::string ToJSON(const ClassType&, tag&&);

        template<typename ClassType>
        std::string ToJSON(const ClassType& from) {
            return ToJSON(from, typename TypeTag<ClassType>::tag());
        }

        template<typename ClassType, typename tag>
        jsonIter FromJSON(jsonIter, jsonIter, ClassType&, tag&&);

        template<typename ClassType>
        jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& classInto) {
            return FromJSON(iter, end, classInto, typename TypeTag<ClassType>::tag());
        }
    }

    json_no_return inline void ThrowBadJSONError(jsonIter iter, jsonIter end,
                                                 const std::string&& errmsg) {
        jsonIter endIter = (std::distance(iter, end) > 1000) ? iter + 1000 : end;
        std::string badJson(iter, endIter);
        throw std::invalid_argument(errmsg + ": " + badJson);
    }

    inline jsonIter AdvancePastWhitespace(jsonIter iter, jsonIter end) {
        while(iter != end) {
            switch(*iter) {
            case L' ':
            case L'\t':
            case L'\n':
            case L'\r':
                break;

            default:
                return iter;
            }

            ++iter;
        }

        return iter;
    }

    inline jsonIter AdvancePastNumbers(jsonIter iter, jsonIter end) {
        if(iter != end &&
           *iter == L'-') {
            ++iter;
        }

        while(iter != end) {
            switch(*iter) {
            case L'.':
            case L'0':
            case L'1':
            case L'2':
            case L'3':
            case L'4':
            case L'5':
            case L'6':
            case L'7':
            case L'8':
            case L'9':
                break;

            default:
                return iter;
            }

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
}
#endif
