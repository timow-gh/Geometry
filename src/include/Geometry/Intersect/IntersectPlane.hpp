#ifndef GEOMETRY_INTERSECTPLANE_HPP
#define GEOMETRY_INTERSECTPLANE_HPP

#include "Geometry/Line.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Segment.hpp"
#include "Geometry/Utils/Compiler.hpp"
#include <linal/utils/eps.hpp>
#include <linal/vec3.hpp>
#include <optional>

namespace Geometry
{
template <typename T>
GEO_NODISCARD std::optional<T> calc_intersection_parameter(linal::vec3<T> planeOrigin,
                                                           linal::vec3<T> planeNormal,
                                                           linal::vec3<T> lineOrigin,
                                                           linal::vec3<T> lineDir,
                                                           T eps) noexcept
{
  // Check parallel
  if (linal::isZero(linal::dot(lineDir, planeNormal), eps))
    return std::nullopt;

  const linal::vec3<T> vec = planeOrigin - lineOrigin;
  return (linal::dot(vec, planeNormal)) / linal::dot(lineDir, planeNormal);
}

template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>> intersect(Plane<T> plane, Line3<T> line, T eps = linal::eps<T>::value) noexcept
{
  linal::vec3<T> planeOrigin = plane.get_origin();
  linal::vec3<T> planeNormal = plane.get_normal();
  linal::vec3<T> lineOrigin = line.get_origin();
  linal::vec3<T> lineDir = line.get_direction();

  auto dirDot = linal::dot(lineDir, planeNormal);
  // Check parallel
  if (linal::isZero(dirDot, eps))
  {
    return std::nullopt;
  }

  const auto paramD = linal::dot(linal::vec3<T>{planeOrigin - lineOrigin}, planeNormal) / dirDot;
  return lineOrigin + paramD * lineDir;
}

template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>> intersect(Plane<T> plane, Ray3<T> ray, T eps = linal::eps<T>::value) noexcept
{
  linal::vec3<T> planeOrigin = plane.get_origin();
  linal::vec3<T> planeNormal = plane.get_normal();
  linal::vec3<T> rayOrigin = ray.get_origin();
  linal::vec3<T> rayDir = ray.get_direction();

  if (auto paramD = calc_intersection_parameter(planeOrigin, planeNormal, rayOrigin, rayDir, eps))
  {
    if (linal::isGreater(*paramD, T(0), eps))
    {
      return rayOrigin + paramD.value() * rayDir;
    }
  }
  return std::nullopt;
}

template <typename T>
GEO_NODISCARD constexpr std::optional<linal::vec3<T>> intersect(Plane<T> plane, Segment3<T> seg, T eps = linal::eps<T>::value) noexcept
{
  linal::vec3<T> planeOrigin = plane.get_origin();
  linal::vec3<T> planeNormal = plane.get_normal();
  linal::vec3<T> segSource = seg.get_source();
  linal::vec3<T> segDir = seg.get_target() - segSource;

  if (auto paramD = calc_intersection_parameter(planeOrigin, planeNormal, segSource, segDir, eps))
  {
    if (linal::isGreater(*paramD, T(0), eps) && linal::isLessEq(*paramD, T(1), eps))
    {
      return segSource + paramD.value() * segDir;
    }
  }
  return std::nullopt;
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTPLANE_HPP
