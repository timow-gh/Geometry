#ifndef GEOMETRY_POINTQUERIES_HPP
#define GEOMETRY_POINTQUERIES_HPP

#include "Geometry/Utils/Compiler.hpp"
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

namespace Geometry
{
    template<typename T, std::uint8_t D>
    GEO_NODISCARD constexpr bool points_are_collinear(const linal::vec<T, D>& p1, const linal::vec<T, D>& p2, const linal::vec<T, D>& p3) noexcept
    {
      return linal::collinear(p2 - p1, p3 - p1);
    }
} // namespace Geometry

#endif // GEOMETRY_TRIANGLE_HPP