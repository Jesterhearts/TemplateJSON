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
                 enable_if<ClassType, std::is_pointer>>
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
                 enable_if<ClassType, std::is_pointer>>
        json_finline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into) {
            iter = advance_past_whitespace(iter);

            if(memcmp("null", iter, 4) == 0) {
                into.write(nullptr);
                return iter + 4;
            }

            typedef typename std::remove_pointer<ClassType>::type InternalClass;
            DataMember<InternalClass> data;
            iter = detail::from_json<InternalClass>(iter, data);
            into.write(new InternalClass{ data.consume() });

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
            json_finline jsonIter from_json(jsonIter iter, DataMember<SmartPointerType<T, D...>>& into) {
                DataMember<T> data;
                iter = detail::from_json(iter, data);
                into.write(new T{ data.consume() });
                return iter;
            }
        }
    }

    template<typename T>
    json_finline void to_json(const std::shared_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T>
    json_finline jsonIter from_json(jsonIter iter, detail::DataMember<std::shared_ptr<T>>& into) {
        return detail::pointers::from_json(iter, into);
    }

    template<typename T>
    json_finline void to_json(const std::weak_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T>
    json_finline jsonIter from_json(jsonIter iter, detail::DataMember<std::weak_ptr<T>>& into) {
        return detail::pointers::from_json(iter, into);
    }

    template<typename T>
    json_finline void to_json(const std::auto_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T>
    json_finline jsonIter from_json(jsonIter iter, detail::DataMember<std::auto_ptr<T>>& into) {
        return detail::pointers::from_json(iter, into);
    }

    template<typename T, typename D>
    json_finline void to_json(const std::unique_ptr<T, D>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T, typename D>
    jsonIter from_json(jsonIter iter, detail::DataMember<std::unique_ptr<T, D>>& into) {
        return detail::pointers::from_json(iter, into);
    }
}

#endif
