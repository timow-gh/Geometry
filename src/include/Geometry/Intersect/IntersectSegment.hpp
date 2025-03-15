#ifndef GEOMETRY_INTERSECTSEGMENT_HPP
#define GEOMETRY_INTERSECTSEGMENT_HPP

#include "Geometry/ClosetPointOnLine.hpp"
#include "Geometry/Ray.hpp"
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
  T detA = (fDir[0] * sDir[1]) - (sDir[0] * fDir[1]);

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
  if (linal::isLess(s, T(0.0), eps) || linal::isGreater(s, T(1.0), eps) || linal::isLess(t, T(0.0), eps) || linal::isGreater(t, T(1.0), eps))
  {
    return std::nullopt;
  }

  GEO_ASSERT(linal::vec2<T>{fSource + (t * fDir)} == linal::vec2<T>{sSource + (s * sDir)});
  return fSource + (fDir * t);
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

  auto params = details::closest_point_on_line_parameters(fSource, fDir, sSource, sDir, eps);
  if (!params)
  {
    return std::nullopt; // parallel
  }

  // Check if the intersection point lies on both line segments
  if (linal::isLess(params->s, T(0.0), eps) || linal::isGreater(params->s, T(1.0), eps) || linal::isLess(params->t, T(0.0), eps) ||
      linal::isGreater(params->t, T(1.0), eps))
  {
    return std::nullopt;
  }

  return fSource + (fDir * params->t);
}

template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>>
intersect(const Segment3<T>& segment, const Ray3<T>& ray, T eps = linal::eps<T>::value) noexcept
{
  linal::vec3<T> segSource = segment.get_source();
  linal::vec3<T> segDir = segment.get_target() - segSource;
  linal::vec3<T> raySource = ray.get_origin();
  linal::vec3<T> rayDir = ray.get_direction();

  auto params = details::closest_point_on_line_parameters(segSource, segDir, raySource, rayDir, eps);
  if (!params)
  {
    return std::nullopt; // parallel
  }

  // Check if the intersection point lies on both, line and ray
  if (linal::isLess(params->t, T(0.0), eps) || linal::isGreater(params->t, T(1.0), eps) || linal::isLess(params->s, T(0.0), eps))
  {
    return std::nullopt;
  }

  return raySource + (rayDir * params->s);
}

template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>>
intersect(const Ray3<T>& ray, const Segment3<T>& segment, T eps = linal::eps<T>::value) noexcept
{
  return intersect(segment, ray, eps);
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTSEGMENT_HPP
