#ifndef __JSON_OBJECT_HINTS_HPP__
#define __JSON_OBJECT_HINTS_HPP__

#include "json_internal_declarations.hpp"

namespace tjson {

namespace object_hints {
    struct non_trivially_constructible : detail::reference_only {};
    struct trivially_constructible : detail::reference_only {};
} /* object_hints */

template<typename ClassType>
struct ObjectHints : detail::reference_only {
    using construction_type = object_hints::trivially_constructible;
};
} /* namespace tjson*/

#endif
