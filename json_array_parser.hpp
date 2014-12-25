#pragma once
#ifndef __JSON_ARRAY_PARSER_HPP__
#define __JSON_ARRAY_PARSER_HPP__

namespace tjson {
namespace detail {
    template<typename ClassType,
             enable_if<ClassType, std::is_array> = true>
    json_finline void to_json(const ClassType& from, detail::Stringbuf& out) {
        if(std::extent<ClassType>::value == 0) {
            out.append("[]", 2);
        }

        out.push_back('[');
        detail::to_json<typename std::remove_extent<ClassType>::type>(from[0], out);

        for(size_t i = 1; i < std::extent<ClassType>::value; ++i) {
            out.push_back(',');
            detail::to_json<typename std::remove_extent<ClassType>::type>(from[i], out);
        }
        out.push_back(']');
    }

    template<typename ClassType,
             enable_if<ClassType, std::is_array> = true>
    json_finline jsonIter from_json(jsonIter iter, ClassType* into) {
        iter = advance_past_whitespace(iter);

        if(*iter != '[') {
            json_parsing_error(iter, "No array start token");
        }
        ++iter;

        for(size_t i = 0; i < std::extent<ClassType>::value; ++i) {
            iter = detail::from_json(iter, (*into)[i]);
            iter = advance_past_whitespace(iter);

            if(*iter != ',' && i < std::extent<ClassType>::value - 1) {
                json_parsing_error(iter, "Missing comma in JSON array");
            }
            else if(*iter == ',') {
                ++iter;
            }
        }

        if(*iter != ']') {
            json_parsing_error(iter, "No end or too many items in JSON array");
        }

        ++iter;
        return iter;
    }
} /* detail */
} /* JSON */
#endif
