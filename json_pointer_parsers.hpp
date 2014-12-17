#pragma once
#ifndef __JSON_POINTER_PARSERS_HPP__
#define __JSON_POINTER_PARSERS_HPP__

namespace std {
    template<typename T, typename D> class unique_ptr;
    template<typename T> class shared_ptr;
    template<typename T> class weak_ptr;
    template<typename T> class auto_ptr;
}

namespace JSON {
    namespace detail {
        template<typename ClassType,
                 enable_if<ClassType, std::is_pointer> = true>
        json_finline std::string ToJSON(const ClassType& from) {
            if(from == nullptr) {
                return nullToken;
            }
            return detail::ToJSON(*from);
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_pointer> = true>
        json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, ClassType& into) {
            iter = AdvancePastWhitespace(iter, end);

            if(iter != end &&
                std::distance(iter, end) > nullToken.length()) {
                //TODO fixme
                std::string info(iter, iter + nullToken.length());
                if(nullToken == info) {
                    into = nullptr;
                    return iter + nullToken.length();
                }
            }

            typedef typename std::remove_pointer<ClassType>::type InternalClass;
            into = new InternalClass;
            iter = detail::FromJSON(iter, end, *into);
            return iter;
        }

        namespace pointers {
            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            json_finline std::string ToJSON(const SmartPointerType<T, D...>& from) {
                if(!from) {
                    return nullToken;
                }

                return detail::ToJSON(*from.get());
            }


            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            json_finline jsonIter FromJSON(jsonIter iter, jsonIter end,
                                           SmartPointerType<T, D...>& into) {
                T* temp;
                iter = detail::FromJSON(iter, end, temp);
                into.reset(temp);
                return iter;
            }
        }
    }

    template<typename T>
    json_finline std::string ToJSON(const std::shared_ptr<T>& from) {
        return detail::pointers::ToJSON(from);
    }

    template<typename T>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::shared_ptr<T>& into) {
        return detail::pointers::FromJSON(iter, end, into);
    }

    template<typename T>
    json_finline std::string ToJSON(const std::weak_ptr<T>& from) {
        return detail::pointers::ToJSON(from);
    }

    template<typename T>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::weak_ptr<T>& into) {
        return detail::pointers::FromJSON(iter, end, into);
    }

    template<typename T>
    json_finline std::string ToJSON(const std::auto_ptr<T>& from) {
        return detail::pointers::ToJSON(from);
    }

    template<typename T>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::auto_ptr<T>& into) {
        return detail::pointers::FromJSON(iter, end, into);
    }

    template<typename T, typename D>
    json_finline std::string ToJSON(const std::unique_ptr<T, D>& from) {
        return detail::pointers::ToJSON(from);
    }

    template<typename T, typename D>
    json_finline jsonIter FromJSON(jsonIter iter, jsonIter end, std::unique_ptr<T, D>& into) {
        return detail::pointers::FromJSON(iter, end, into);
    }
}

#endif
