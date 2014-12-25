#pragma once
#ifndef __JSON_POINTER_PARSERS_HPP__
#define __JSON_POINTER_PARSERS_HPP__

namespace std {
    template<typename T, typename D> class unique_ptr;
    template<typename T> class shared_ptr;
    template<typename T> class weak_ptr;
    template<typename T> class auto_ptr;
}

namespace tjson {
    namespace detail {
        template<typename ClassType,
                 enable_if<ClassType, std::is_pointer> = true>
        json_finline void to_json(ClassType from, detail::Stringbuf& out) {
            if(!from) {
                out.append("null", 4);
            }
            else {
                typedef typename std::decay<decltype(*from)>::type pointed_at;
                detail::to_json<pointed_at>(*from, out);
            }
        }

        template<typename ClassType,
                 enable_if<ClassType, std::is_pointer> = true>
        json_finline jsonIter from_json(jsonIter iter, ClassType* into) {
            //Placement TODO
            iter = advance_past_whitespace(iter);

            if(memcmp("null", iter, 4) == 0) {
                *into = nullptr;
                return iter + 4;
            }

            typedef typename std::remove_pointer<ClassType>::type InternalClass;
            *into = new InternalClass;
            iter = detail::from_json<InternalClass>(iter, *into);
            return iter;
        }

        namespace pointers {
            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            json_finline void to_json(const SmartPointerType<T, D...>& from, detail::Stringbuf& out) {
                if(!from) {
                    out.append("null", 4);
                }
                else {
                    typedef typename std::decay<decltype(*from.get())>::type pointed_at;
                    detail::to_json<pointed_at>(*from.get(), out);
                }
            }


            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            json_finline jsonIter from_json(jsonIter iter, SmartPointerType<T, D...>* into) {
                //Placement TODO
                T* temp;
                iter = detail::from_json(iter, &temp);
                into->reset(temp);
                return iter;
            }
        }
    }

    template<typename T>
    json_finline void to_json(const std::shared_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T>
    json_finline jsonIter from_json(jsonIter iter, std::shared_ptr<T>* into) {
        return detail::pointers::from_json(iter, into);
    }

    template<typename T>
    json_finline void to_json(const std::weak_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T>
    json_finline jsonIter from_json(jsonIter iter, std::weak_ptr<T>* into) {
        return detail::pointers::from_json(iter, into);
    }

    template<typename T>
    json_finline void to_json(const std::auto_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T>
    json_finline jsonIter from_json(jsonIter iter, std::auto_ptr<T>* into) {
        return detail::pointers::from_json(iter, into);
    }

    template<typename T, typename D>
    json_finline void to_json(const std::unique_ptr<T, D>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T, typename D>
    jsonIter from_json(jsonIter iter, std::unique_ptr<T, D>* into) {
        return detail::pointers::from_json(iter, into);
    }
}

#endif
