#ifndef GEOMETRY_INVALIDINDEX_HPP
#define GEOMETRY_INVALIDINDEX_HPP

#include <Core/Utils/Compiler.hpp>
#include <limits>

namespace Geometry
{
CORE_CONSTEXPR std::size_t INVALID_INDEX =
    std::numeric_limits<std::size_t>::max();
}

#endif // GEOMETRY_INVALIDINDEX_HPP
