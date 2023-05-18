#ifndef GEOMETRY_INTERSECTIONPLANE_HPP
#define GEOMETRY_INTERSECTIONPLANE_HPP

#include <Core/Math/Eps.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <linal/Vec3.hpp>
#include <optional>

namespace Geometry
{
template <typename T>
std::optional<T>
calcIntersectionParameter(linal::Vec3<T> planeOrigin, linal::Vec3<T> planeNormal, linal::Vec3<T> lineOrigin, linal::Vec3<T> lineDir, T eps)
{
  // Check parallel
  if (Core::isZero(linal::dot(lineDir, planeNormal), eps))
    return std::nullopt;

  const linal::Vec3<T> vec = planeOrigin - lineOrigin;
  return (linal::dot(vec, planeNormal)) / linal::dot(lineDir, planeNormal);
}

template <typename T>
GEO_CONSTEXPR std::optional<linal::Vec3<T>> intersection(Plane<T> plane, Line3<T> line, T eps = Core::eps_traits<T>::value())
{
  linal::Vec3<T> planeOrigin = plane.getOrigin();
  linal::Vec3<T> planeNormal = plane.getNormal();
  linal::Vec3<T> lineOrigin = line.getOrigin();
  linal::Vec3<T> lineDir = line.getDirection();

  auto dirDot = linal::dot(lineDir, planeNormal);
  // Check parallel
  if (Core::isZero(dirDot, eps))
    return std::nullopt;

  const auto paramD = linal::dot(linal::Vec3<T>{planeOrigin - lineOrigin}, planeNormal) / dirDot;
  return lineOrigin + paramD * lineDir;
}

template <typename T>
GEO_CONSTEXPR std::optional<linal::Vec3<T>> intersection(Plane<T> plane, Ray3<T> ray, T eps = Core::eps_traits<T>::value())
{
  linal::Vec3<T> planeOrigin = plane.getOrigin();
  linal::Vec3<T> planeNormal = plane.getNormal();
  linal::Vec3<T> rayOrigin = ray.getOrigin();
  linal::Vec3<T> rayDir = ray.getDirection();

  if (auto paramD = calcIntersectionParameter(planeOrigin, planeNormal, rayOrigin, rayDir, eps))
  {
    if (Core::isGreater(*paramD, T(0), eps))
      return rayOrigin + paramD.value() * rayDir;
  }
  return std::nullopt;
}

template <typename T>
GEO_CONSTEXPR std::optional<linal::Vec3<T>> intersection(Plane<T> plane, Segment3<T> seg, T eps = Core::eps_traits<T>::value())
{
  linal::Vec3<T> planeOrigin = plane.getOrigin();
  linal::Vec3<T> planeNormal = plane.getNormal();
  linal::Vec3<T> segSource = seg.getSource();
  linal::Vec3<T> segDir = seg.getTarget() - segSource;

  if (auto paramD = calcIntersectionParameter(planeOrigin, planeNormal, segSource, segDir, eps))
  {
    if (Core::isGreater(*paramD, T(0), eps) && Core::isLessEq(*paramD, T(1), eps))
      return segSource + paramD.value() * segDir;
  }
  return std::nullopt;
}

} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONPLANE_HPP
