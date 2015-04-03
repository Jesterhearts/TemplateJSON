#ifndef __JSON_OBJECT_HINTS_HPP__
#define __JSON_OBJECT_HINTS_HPP__

#include "json_internal_declarations.hpp"

namespace tjson {

enum struct object_hints {
    non_trivially_constructible,
    trivially_constructible
};

template<typename ClassType>
struct ConstructHint : detail::reference_only {
    static const object_hints construction_type;
};

template<typename ClassType>
const object_hints ConstructHint<ClassType>::construction_type
    = object_hints::non_trivially_constructible;
} /* namespace tjson*/

#endif
