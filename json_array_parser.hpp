#pragma once
#ifndef __JSON_ARRAY_PARSER_HPP__
#define __JSON_ARRAY_PARSER_HPP__

namespace JSON {
namespace detail {
    template<typename ClassType,
             enable_if<ClassType, std::is_array> = true>
    json_finline std::string ToJSON(const ClassType& from) {
        if(std::extent<ClassType>::value == 0) {
            return "[]";
        }

        std::string json("[");
        json.append(detail::ToJSON(from[0]));

        for(size_t i = 1; i < std::extent<ClassType>::value; ++i) {
            json.push_back(',');
            json.append(detail::ToJSON(from[i]));
        }
        json.push_back(']');

        return json;
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
