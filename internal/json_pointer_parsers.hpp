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
        inline void to_json(ClassType from, detail::Stringbuf& out) {
            if(!from) {
                out.append("null", 4);
            }
            else {
                typedef typename std::decay<decltype(*from)>::type pointed_at;
                detail::to_json<pointed_at>(*from, out);
            }
        }

        template<typename ClassType, typename store_tag,
                 enable_if<ClassType, std::is_pointer>>
        inline void from_json(Tokenizer& tokenizer, DataMember<ClassType, store_tag>& into) {

            tokenizer.seek();
            if(memcmp("null", tokenizer.position(), 4) == 0) {
                into.write(nullptr);
                tokenizer.skip(4);
                return;
            }

            typedef typename std::remove_pointer<ClassType>::type InternalClass;
            DataMember<InternalClass, detail::data_internal_store_tag> data;
            detail::from_json<InternalClass>(tokenizer, data);
            into.write(new InternalClass{ data.consume() });
        }

        namespace pointers {
            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType>
            inline void to_json(const SmartPointerType<T, D...>& from, detail::Stringbuf& out) {
                if(!from) {
                    out.append("null", 4);
                }
                else {
                    typedef typename std::decay<decltype(*from.get())>::type pointed_at;
                    detail::to_json<pointed_at>(*from.get(), out);
                }
            }


            template<typename T, typename... D,
                     template<typename T, typename... D> class SmartPointerType,
                     typename store_tag>
            inline void from_json(Tokenizer& tokenizer, DataMember<SmartPointerType<T, D...>, store_tag>& into) {
                DataMember<T, detail::data_internal_store_tag> data;
                detail::from_json(tokenizer, data);
                into.write(new T{ data.consume() });
            }
        }
    }

    template<typename T>
    inline void to_json(const std::shared_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T, typename store_tag>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::shared_ptr<T>, store_tag>& into) {
        return detail::pointers::from_json(tokenizer, into);
    }

    template<typename T>
    inline void to_json(const std::weak_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T, typename store_tag>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::weak_ptr<T>, store_tag>& into) {
        return detail::pointers::from_json(tokenizer, into);
    }

    template<typename T>
    inline void to_json(const std::auto_ptr<T>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T, typename store_tag>
    inline void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::auto_ptr<T>, store_tag>& into) {
        return detail::pointers::from_json(tokenizer, into);
    }

    template<typename T, typename D>
    inline void to_json(const std::unique_ptr<T, D>& from, detail::Stringbuf& out) {
        detail::pointers::to_json(from, out);
    }

    template<typename T, typename D, typename store_tag>
    void from_json(detail::Tokenizer& tokenizer, detail::DataMember<std::unique_ptr<T, D>, store_tag>& into) {
        return detail::pointers::from_json(tokenizer, into);
    }
}

#endif
