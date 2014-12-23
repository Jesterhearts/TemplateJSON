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
        json_finline void ToJSON(ClassType from, std::string& out) {
            if(!from) {
                out.append("null", 4);
            }
            else {
                detail::ToJSON(*from, out);
            }
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_pointer> = true>
        json_finline jsonIter FromJSON(jsonIter iter, ClassType& into) {
            iter = AdvancePastWhitespace(iter);

            if(memcmp("null", iter, 4) == 0) {
                into = nullptr;
                return iter + 4;
            }

            typedef typename std::remove_pointer<ClassType>::type InternalClass;
            into = new InternalClass;
            iter = detail::FromJSON(iter, *into);
            return iter;
        }

        namespace pointers {
            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            json_finline void ToJSON(const SmartPointerType<T, D...>& from, std::string& out) {
                if(!from) {
                    out.append("null", 4);
                }
                else {
                    detail::ToJSON(*from.get(), out);
                }
            }


            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            json_finline jsonIter FromJSON(jsonIter iter, SmartPointerType<T, D...>& into) {
                T* temp;
                iter = detail::FromJSON(iter, temp);
                into.reset(temp);
                return iter;
            }
        }
    }

    template<typename T>
    json_finline void ToJSON(const std::shared_ptr<T>& from, std::string& out) {
        detail::pointers::ToJSON(from, out);
    }

    template<typename T>
    json_finline jsonIter FromJSON(jsonIter iter, std::shared_ptr<T>& into) {
        return detail::pointers::FromJSON(iter, into);
    }

    template<typename T>
    json_finline void ToJSON(const std::weak_ptr<T>& from, std::string& out) {
        detail::pointers::ToJSON(from, out);
    }

    template<typename T>
    json_finline jsonIter FromJSON(jsonIter iter, std::weak_ptr<T>& into) {
        return detail::pointers::FromJSON(iter, into);
    }

    template<typename T>
    json_finline void ToJSON(const std::auto_ptr<T>& from, std::string& out) {
        detail::pointers::ToJSON(from, out);
    }

    template<typename T>
    json_finline jsonIter FromJSON(jsonIter iter, std::auto_ptr<T>& into) {
        return detail::pointers::FromJSON(iter, into);
    }

    template<typename T, typename D>
    json_finline void ToJSON(const std::unique_ptr<T, D>& from, std::string& out) {
        detail::pointers::ToJSON(from, out);
    }

    template<typename T, typename D>
    jsonIter FromJSON(jsonIter iter, std::unique_ptr<T, D>& into) {
        return detail::pointers::FromJSON(iter, into);
    }
}

#endif
