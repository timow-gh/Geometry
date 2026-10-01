#ifndef GEOMETRY_PREDICATES_HPP
#define GEOMETRY_PREDICATES_HPP

#include "Geometry/Utils/Assert.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include "Geometry/detail/TriangleOrientation.hpp"

#include <cmath>
#include <cstdint>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <type_traits>

namespace Geometry
{

/**
 * \brief Sign of an orientation predicate.
 *
 * The underlying values are the signs themselves, so callers can multiply or negate them as integers.
 */
enum class Orientation : std::int8_t
{
  Negative = -1,
  Zero = 0,
  Positive = 1
};

namespace detail
{

/**
 * \internal
 * \brief Maps a determinant to its \c Orientation.
 *
 * The comparison is against exact zero on purpose: an epsilon would only move the wrong answers to
 * a different place, while the exact predicates that replace the placeholders have no epsilon either.
 */
template <typename T>
GEO_NODISCARD constexpr Orientation orientation_from_determinant(const T determinant) noexcept
{
  if (determinant > T{0})
  {
    return Orientation::Positive;
  }
  if (determinant < T{0})
  {
    return Orientation::Negative;
  }
  return Orientation::Zero;
}

} // namespace detail

/**
 * \brief Whether \p query lies left of (\c Positive), right of (\c Negative) or on (\c Zero) the
 * directed line from \p first to \p second; equivalently, whether (first, second, query) is
 * counterclockwise.
 *
 * Every 2D side-of-line decision of the mesh Booleans goes through this function, so making them
 * exact is a change in one place.
 *
 * Limits:
 * - Placeholder: a plain floating-point determinant. Near-degenerate input can give the wrong sign or
 *   a false \c Zero. It will be replaced by Shewchuk's adaptive exact predicate, which uses the same
 *   sign convention.
 * - Even the exact version is exact only for the explicit coordinates it is given. A constructed
 *   intersection point is stored rounded, so a predicate on it decides for the rounded point, not
 *   the true one; such points go through \c detail::ImplicitPoint instead.
 *
 * Exact (including \c Zero) when all coordinates are small integers, since then no operation rounds.
 * O(1).
 */
template <typename T>
GEO_NODISCARD Orientation orient2d(const linal::vec2<T>& first,
                                   const linal::vec2<T>& second,
                                   const linal::vec2<T>& query) noexcept
{
  // Restricted to floating point because the exact replacement is defined only for it.
  static_assert(std::is_floating_point_v<T>, "orientation predicates require floating-point coordinates");
  return detail::orientation_from_determinant(detail::triangle_orientation(first, second, query));
}

/**
 * \brief Whether \p query lies on the side of the plane through (first, second, third) that the
 * right-hand normal (second - first) x (third - first) points to (\c Positive), on the other side
 * (\c Negative), or on the plane (\c Zero).
 *
 * Equivalently, the sign of the volume of the tetrahedron (first, second, third, query). For an
 * outward-oriented face, \c Positive means \p query is outside. Every 3D side-of-plane decision of
 * the mesh Booleans goes through this function, so making them exact is a change in one place.
 *
 * Limits:
 * - Placeholder: a plain floating-point determinant. Near-degenerate input can give the wrong sign or
 *   a false \c Zero. It will be replaced by Shewchuk's adaptive exact predicate. Shewchuk's orient3d
 *   uses the opposite sign (positive below the plane), so the replacement must negate its result.
 * - Even the exact version is exact only for the explicit coordinates it is given. A constructed
 *   intersection point is stored rounded, so a predicate on it decides for the rounded point, not
 *   the true one; such points go through \c detail::ImplicitPoint instead.
 *
 * Exact (including \c Zero) when all coordinates are small integers, since then no operation rounds.
 * O(1).
 */
template <typename T>
GEO_NODISCARD Orientation orient3d(const linal::vec3<T>& first,
                                   const linal::vec3<T>& second,
                                   const linal::vec3<T>& third,
                                   const linal::vec3<T>& query) noexcept
{
  // Restricted to floating point because the exact replacement is defined only for it.
  static_assert(std::is_floating_point_v<T>, "orientation predicates require floating-point coordinates");
  using Vec3 = linal::vec3<T>;

  const Vec3 normal = detail::triangle_orientation(first, second, third);
  const Vec3 toQuery{query - first};
  return detail::orientation_from_determinant(linal::dot(normal, toQuery));
}

namespace detail
{

/**
 * \internal
 * \brief Index of the largest-magnitude component of \p normal; on ties the lowest index wins.
 *
 * Coplanar tests project onto the axis-aligned plane that drops this axis, which keeps the projected
 * triangle as large as possible and never degenerate. The axis only has to be chosen consistently
 * for all tests within one face, so pass the face's area vector from \c triangle_orientation
 * rather than deriving it from \c orient3d results. Ties break deterministically so that the same
 * face always gets the same axis.
 *
 * \pre \p normal is not the zero vector (the face is not degenerate).
 * O(1).
 */
template <typename T>
GEO_NODISCARD std::uint8_t dominant_axis(const linal::vec3<T>& normal) noexcept
{
  std::uint8_t axis = 0;
  T largest = std::abs(normal[0]);
  for (std::uint8_t candidate = 1; candidate < 3; ++candidate)
  {
    const T magnitude = std::abs(normal[candidate]);
    if (magnitude > largest)
    {
      largest = magnitude;
      axis = candidate;
    }
  }
  // Exact comparison: linal's vector equality has a tolerance, which would reject tiny but valid faces.
  GEO_ASSERT(largest > T{0});
  return axis;
}

/**
 * \internal
 * \brief \p point with coordinate \p axis removed.
 *
 * The remaining coordinates are only copied, never computed, so the projection is exact and
 * \c orient2d on projected points is as exact as \c orient2d itself. They keep their cyclic order
 * (axis + 1, axis + 2), so a triangle projects counterclockwise iff its normal's component along
 * \p axis is positive; orientations can therefore be compared across faces that share an axis.
 *
 * \pre \p axis < 3.
 * O(1).
 */
template <typename T>
GEO_NODISCARD linal::vec2<T> project_dropping_axis(const linal::vec3<T>& point, const std::uint8_t axis) noexcept
{
  GEO_ASSERT(axis < 3);
  return linal::vec2<T>{point[(axis + 1) % 3], point[(axis + 2) % 3]};
}

} // namespace detail
} // namespace Geometry

#endif // GEOMETRY_PREDICATES_HPP
