#ifndef GEOMETRY_INTERSECTSEGMENT_HPP
#define GEOMETRY_INTERSECTSEGMENT_HPP

#include "Geometry/ClosetPointOnLine.hpp"
#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>
#include <optional>

namespace Geometry
{

/** @brief Calculates the single intersection point of two 2d line segments.
 *
 * Parallel segments don't intersect.
 * Doesn't handle overlapping segments.
 */
template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec2<T>>
intersect(Segment2<T> first, Segment2<T> second, T eps = linal::eps<T>::value) noexcept
{
  linal::vec2<T> fSource = first.get_source();
  linal::vec2<T> sSource = second.get_source();
  linal::vec2<T> fDir = first.get_target() - fSource;
  linal::vec2<T> sDir = second.get_target() - sSource;

  // Calculate the determinant Det(A) of the matrix A = [fDir, sDir]
  T detA = fDir[0] * sDir[1] - sDir[0] * fDir[1];

  // Check if segments are parallel (Det(A) = 0)
  if (linal::isZero(detA, eps))
  {
    return std::nullopt; // Segments are parallel, no intersection
  }

  linal::vec2<T> fDist = sSource - fSource;
  T t = (fDist[0] * sDir[1] - fDist[1] * sDir[0]) / detA;
  // Flip the sign of the fDist vector, so that the sign of s is correct
  T s = (fDir[0] * -fDist[1] - fDir[1] * -fDist[0]) / detA;

  // Check if the intersection point lies on both line segments
  if (s < 0 || s > 1 || t < 0 || t > 1)
  {
    return std::nullopt;
  }

  GEO_ASSERT(linal::vec2<T>{fSource + t * fDir} == linal::vec2<T>{sSource + s * sDir});
  return fSource + fDir * t;
}

/** @brief Calculates the single intersection point of two 3d line segments.
 *
 * Parallel segments don't intersect.
 * Doesn't handle overlapping segments.
 */
template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>>
intersect(const Segment3<T>& first, const Segment3<T>& second, T eps = linal::eps<T>::value) noexcept
{
  linal::vec3<T> fSource = first.get_source();
  linal::vec3<T> sSource = second.get_source();
  linal::vec3<T> fDir = first.get_target() - fSource;
  linal::vec3<T> sDir = second.get_target() - sSource;

  // Calculate the cross product of the direction vectors
  linal::vec3<T> cross = linal::cross(fDir, sDir);

  // Check if lines are parallel
  if (linal::isZero(linal::length(cross), eps))
  {
    return std::nullopt;
  }

  auto params = details::closest_point_on_line_parameters(fSource, fDir, sSource, sDir);
  if (!params)
  {
    return std::nullopt; // parallel
  }

  // Check if the intersection point lies on both line segments
  if (params->s < 0 || params->s > 1 || params->t < 0 || params->t > 1)
  {
    return std::nullopt;
  }

  return fSource + fDir * params->t;
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTSEGMENT_HPP
