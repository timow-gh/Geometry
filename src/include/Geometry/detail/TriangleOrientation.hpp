#ifndef GEOMETRY_DETAIL_TRIANGLEORIENTATION_HPP
#define GEOMETRY_DETAIL_TRIANGLEORIENTATION_HPP

#include "Geometry/Utils/Compiler.hpp"

#include <cstdint>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <type_traits>

namespace Geometry
{
namespace detail
{

/**
 * \internal
 * \brief Orientation of the triangle (first, apex, last): the area vector (cross product of two edges) in 3D,
 * the signed doubled area in 2D.
 *
 * One primitive for both dimensions, so orientation tests (flip checks, hole triangulation) are
 * written once: two triangles agree in orientation iff \c orientation_dot of theirs is positive, and
 * a triangle is degenerate iff \c orientation_dot with itself is zero.
 */
template <typename T, std::uint8_t D>
GEO_NODISCARD auto triangle_orientation(const linal::vec<T, D>& first, const linal::vec<T, D>& apex,
                                        const linal::vec<T, D>& last) noexcept
{
  static_assert(D == 2 || D == 3, "orientation is defined for planar and spatial triangles only");
  using Vec = linal::vec<T, D>;

  const Vec toApex{apex - first};
  const Vec toLast{last - first};
  if constexpr (D == 3)
  {
    return linal::vec3<T>{linal::cross(toApex, toLast)};
  }
  else
  {
    return toApex[0] * toLast[1] - toApex[1] * toLast[0];
  }
}

/**
 * \internal
 * \brief Inner product of two \c triangle_orientation values: positive when the triangles agree in
 * orientation; with itself, the squared doubled area.
 */
template <typename TOrientation>
GEO_NODISCARD auto orientation_dot(const TOrientation& lhs, const TOrientation& rhs) noexcept
{
  if constexpr (std::is_arithmetic_v<TOrientation>)
  {
    return lhs * rhs;
  }
  else
  {
    return linal::dot(lhs, rhs);
  }
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_DETAIL_TRIANGLEORIENTATION_HPP
