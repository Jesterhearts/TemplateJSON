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
            json.append(",");
            json.append(detail::ToJSON(from[i]));
        }
        json.append("]");

        return json;
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_array> = true>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
        if(end - iter < 2) {
            ThrowBadJSONError(iter, end, "No array tokens");
        }

        iter = AdvancePastWhitespace(iter, end);
        if(*iter != L'[') {
            ThrowBadJSONError(iter, end, "No array start token");
        }
        ++iter;

        for(size_t i = 0; i < std::extent<ClassType>::value; ++i) {
            iter = detail::FromJSON(iter, end, into[i]);
            iter = AdvancePastWhitespace(iter, end);

            if(iter == end) {
                ThrowBadJSONError(iter, end, "Not enough items in JSON array");
            }

            if(*iter != L',' && i < std::extent<ClassType>::value - 1) {
                ThrowBadJSONError(iter, end, "Missing comma in JSON array");
            }
            else if(*iter == L',') {
                ++iter;
            }
        }

        if(iter == end || *iter != L']') {
            ThrowBadJSONError(iter, end, "No end or too many items in JSON array");
        }

        ++iter;
        return iter;
    }
} /* detail */
} /* JSON */
#endif
