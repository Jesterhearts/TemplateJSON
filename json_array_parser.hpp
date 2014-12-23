#pragma once
#ifndef __JSON_ARRAY_PARSER_HPP__
#define __JSON_ARRAY_PARSER_HPP__

namespace JSON {
namespace detail {
    template<typename ClassType,
             enable_if<ClassType, std::is_array> = true>
    json_finline void ToJSON(const ClassType& from, detail::stringbuf& out) {
        if(std::extent<ClassType>::value == 0) {
            out.append("[]", 2);
        }

        out.push_back('[');
        detail::ToJSON<typename std::remove_extent<ClassType>::type>(from[0], out);

        for(size_t i = 1; i < std::extent<ClassType>::value; ++i) {
            out.push_back(',');
            detail::ToJSON<typename std::remove_extent<ClassType>::type>(from[i], out);
        }
        out.push_back(']');
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_array> = true>
    json_finline jsonIter FromJSON(jsonIter iter, ClassType& into) {
        iter = AdvancePastWhitespace(iter);

        if(*iter != '[') {
            ThrowBadJSONError(iter, "No array start token");
        }
        ++iter;

        for(size_t i = 0; i < std::extent<ClassType>::value; ++i) {
            iter = detail::FromJSON(iter, into[i]);
            iter = AdvancePastWhitespace(iter);

            if(*iter != ',' && i < std::extent<ClassType>::value - 1) {
                ThrowBadJSONError(iter, "Missing comma in JSON array");
            }
            else if(*iter == ',') {
                ++iter;
            }
        }

        if(*iter != ']') {
            ThrowBadJSONError(iter, "No end or too many items in JSON array");
        }

        ++iter;
        return iter;
    }
} /* detail */
} /* JSON */
#endif
