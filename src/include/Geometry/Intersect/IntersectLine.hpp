#ifndef GEOMETRY_INTERSECTLINE_HPP
#define GEOMETRY_INTERSECTLINE_HPP

#include "Geometry/ClosetPointOnLine.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <optional>

namespace Geometry
{

/** @brief Calculates the single intersection point of two lines.
 *
 * Parallel lines don't intersect.
 */
template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec2<T>> intersect(Line2<T> lhs, Line2<T> rhs, T eps = linal::eps<T>::value) noexcept
{
  linal::vec2<T> lhsSource = lhs.get_origin();
  linal::vec2<T> rhsSource = rhs.get_origin();
  linal::vec2<T> lhsDir = lhs.get_direction();
  linal::vec2<T> rhsDir = rhs.get_direction();

  // Calculate the determinant Det(A) of the matrix A = [lhsDir, rhsDir]
  T detA = lhsDir[0] * rhsDir[1] - rhsDir[0] * lhsDir[1];

  // Check if lines are parallel (Det(A) = 0)
  if (linal::isZero(std::abs(detA), eps))
  {
    return std::nullopt; // Lines are parallel, no intersection
  }

  linal::vec2<T> dist = rhsSource - lhsSource;
  T t = (dist[0] * rhsDir[1] - dist[1] * rhsDir[0]) / detA;

  return lhsSource + t * lhsDir;
}

/** @brief Calculates single the intersection point of two lines.
 *
 * Parallel lines don't intersect.
 */
template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>> intersect(Line3<T> lhs, Line3<T> rhs, T eps = linal::eps<T>::value) noexcept
{
  linal::vec3<T> lhsDir = lhs.get_direction(); // Direction vector of the first line
  linal::vec3<T> rhsDir = rhs.get_direction(); // Direction vector of the second line

  linal::vec3<T> lhsSource = lhs.get_origin();
  linal::vec3<T> rhsSource = rhs.get_origin();
  auto params = details::closest_point_on_line_parameters(lhsSource, lhsDir, rhsSource, rhsDir);
  if (!params)
  {
    return std::nullopt; // Lines are parallel
  }

  linal::vec3<T> lhsPoint = lhsSource + (params->t * lhsDir);
  linal::vec3<T> rhsPoint = rhsSource + (params->s * rhsDir);

  if (linal::isZero(linal::length(lhsPoint - rhsPoint), eps))
  {
    return lhsPoint; // Lines intersect
  }
  return std::nullopt; // Lines are skew
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTLINE_HPP
