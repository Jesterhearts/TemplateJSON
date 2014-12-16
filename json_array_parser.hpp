#pragma once
#ifndef __JSON_ARRAY_PARSER_HPP__
#define __JSON_ARRAY_PARSER_HPP__

namespace JSON {
/////////////////////////////////////////
// Compile-time sized array handler
    template<typename ClassOn,
             size_t rank>
    struct JSONArrayHandler {
        json_finline static std::string ToJSON(const ClassOn& classFrom) {
            if(std::extent<ClassOn>::value == 0) {
                return "[]";
            }

            typedef typename std::remove_extent<ClassOn>::type valueType;

            std::string json("[");
            json += detail::ToJSON(classFrom[0]);

            for(size_t i = 1; i < std::extent<ClassOn>::value; ++i) {
                json += ",";
                json += detail::ToJSON(classFrom[i]);
            }
            json += "]";
            return json;
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            if(end - iter < 2) {
                ThrowBadJSONError(iter, end, "No array tokens");
            }

            iter = AdvancePastWhitespace(iter, end);
            if(*iter != L'[') {
                ThrowBadJSONError(iter, end, "No array start token");
            }
            ++iter;

            typedef typename std::remove_extent<ClassOn>::type valueType;
            for(size_t i = 0; i < std::extent<ClassOn>::value; ++i) {
                iter = detail::FromJSON(iter, end, into[i]);
                iter = AdvancePastWhitespace(iter, end);

                if(iter == end) {
                    ThrowBadJSONError(iter, end, "Not enough items in JSON array");
                }

                if(*iter != L',' && i < std::extent<ClassOn>::value - 1) {
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
    };

    /* Not an array */
    template<typename ClassOn>
    struct JSONArrayHandler<ClassOn, 0> {
        json_finline static std::string ToJSON(const ClassOn& classFrom) {
            return detail::ToJSON(classFrom);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, jsonIter end, ClassOn& into) {
            return detail::FromJSON(iter, end, into);
        }
    };

    namespace detail {
        template<typename ClassType>
        json_finline std::string ToJSON(const ClassType& from, _array&& a) {
            return JSONArrayHandler<ClassType>::ToJSON(from);
        }

        template<typename ClassType>
        json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into, _array&& a) {
            return JSONArrayHandler<ClassType>::FromJSON(iter, end, into);
        }
    }
}
#endif
