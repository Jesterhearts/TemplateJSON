#ifndef __JSON_OBJECT_HINTS_HPP__
#define __JSON_OBJECT_HINTS_HPP__

#include "json_internal_declarations.hpp"

namespace tjson {

enum struct object_hints {
    non_trivially_constructible,
    trivially_constructible
};

template<typename ClassType>
struct ObjectHints : detail::reference_only {
    constexpr static const object_hints construction_type = object_hints::non_trivially_constructible;
};
} /* namespace tjson*/

#endif
