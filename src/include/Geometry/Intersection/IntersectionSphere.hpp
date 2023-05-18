#ifndef GEOMETRY_INTERSECTIONSPHERE_HPP
#define GEOMETRY_INTERSECTIONSPHERE_HPP

#include <linal/utils/Eps.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <Geometry/Sphere.hpp>
#include <linal/Vec3.hpp>
#include <optional>

namespace Geometry
{

template <typename T>
struct SphereIntersection
{
  std::optional<linal::Vec3<T>> first;
  std::optional<linal::Vec3<T>> second;

  SphereIntersection() = default;
  explicit SphereIntersection(const linal::Vec3<T>& firstIntersection)
      : first(firstIntersection)
  {
  }
  SphereIntersection(const linal::Vec3<T>& firstIntersection, const linal::Vec3<T>& secondIntersection)
      : first(firstIntersection)
      , second(secondIntersection)
  {
  }

  GEO_NODISCARD bool hasIntersection() const { return first || second; }
};

template <typename T>
SphereIntersection<T> intersection(const Sphere<T>& sphere, const Line3<T>& line, T eps = linal::eps<T>::value)
{
  // See Schneider - Geometric Tools, Linear Components and Spheres
  const linal::Vec3<T>& sphereOrigin = sphere.getOrigin();
  const T radius = sphere.getRadius();
  const linal::Vec3<T>& lineOrigin = line.getOrigin();
  const linal::Vec3<T>& lineDir = line.getDirection();

  const linal::Vec3<T> deltaOrigin = (lineOrigin - sphereOrigin);
  const T a = linal::dot(lineDir, lineDir);
  const T b = 2 * linal::dot(lineDir, deltaOrigin);
  const T c = linal::dot(deltaOrigin, deltaOrigin) - radius * radius;
  const T discriminant = b * b - 4 * a * c;

  if (linal::isGreater(discriminant, T(0), eps))
  {
    T sqrtDiscr = std::sqrt(discriminant);
    T twoA = 2 * a;
    T t1 = (-b + sqrtDiscr) / twoA;
    T t2 = (-b - sqrtDiscr) / twoA;
    return SphereIntersection<T>(lineOrigin + t1 * lineDir, lineOrigin + t2 * lineDir);
  }
  else if (linal::isZero(discriminant, eps))
  {
    T t = -b / 2 * a;
    return SphereIntersection<T>(lineOrigin + t * lineDir);
  }
  return {};
}

template <typename T>
SphereIntersection<T> intersection(const Sphere<T>& sphere, const Ray3<T>& ray, T eps = linal::eps<T>::value)
{
  // See Schneider - Geometric Tools, Linear Components and Spheres
  const linal::Vec3<T>& sphereOrigin = sphere.getOrigin();
  const T radius = sphere.getRadius();
  const linal::Vec3<T>& rayOrigin = ray.getOrigin();
  const linal::Vec3<T>& rayDir = ray.getDirection();

  const linal::Vec3<T> deltaOrigin = (rayOrigin - sphereOrigin);
  const T a = linal::dot(rayDir, rayDir);
  const T b = 2 * linal::dot(rayDir, deltaOrigin);
  const T c = linal::dot(deltaOrigin, deltaOrigin) - radius * radius;
  const T discriminant = b * b - 4 * a * c;

  if (linal::isGreater(discriminant, T(0), eps))
  {
    T sqrtDiscr = std::sqrt(discriminant);
    T twoA = 2 * a;
    T t1 = (-b + sqrtDiscr) / twoA;
    T t2 = (-b - sqrtDiscr) / twoA;

    SphereIntersection<T> result;
    if (linal::isGreaterEq(t1, T(0), eps))
      result.first = rayOrigin + t1 * rayDir;
    if (linal::isGreaterEq(t2, T(0), eps))
      result.second = rayOrigin + t2 * rayDir;
    return result;
  }
  else if (linal::isZero(discriminant, eps))
  {
    T t = -b / 2 * a;
    if (linal::isGreaterEq(t, T(0), eps))
      return SphereIntersection<T>(rayOrigin + t * rayDir);
  }
  return {};
}

template <typename T>
SphereIntersection<T> intersection(const Sphere<T>& sphere, const Segment3<T>& segment, T eps = linal::eps<T>::value)
{
  // See Schneider - Geometric Tools, Linear Components and Spheres
  const linal::Vec3<T>& sphereOrigin = sphere.getOrigin();
  const T radius = sphere.getRadius();
  const linal::Vec3<T>& segSource = segment.getSource();
  const linal::Vec3<T>& segTarget = segment.getTarget();

  const linal::Vec3<T> deltaOrigin = (segSource - sphereOrigin);
  const linal::Vec3<T> segDir = (segTarget - segSource);

  const T a = linal::dot(segDir, segDir);
  const T b = 2 * linal::dot(segDir, deltaOrigin);
  const T c = linal::dot(deltaOrigin, deltaOrigin) - radius * radius;
  const T discriminant = b * b - 4 * a * c;

  if (linal::isGreater(discriminant, T(0), eps))
  {
    T sqrtDiscr = std::sqrt(discriminant);
    T twoA = 2 * a;
    T t1 = (-b + sqrtDiscr) / twoA;
    T t2 = (-b - sqrtDiscr) / twoA;

    SphereIntersection<T> result;
    if (linal::isGreaterEq(t1, T(0), eps) && linal::isLessEq(t1, T(1), eps))
      result.first = segSource + t1 * segDir;
    if (linal::isGreaterEq(t2, T(0), eps) && linal::isLessEq(t2, T(1), eps))
      result.second = segSource + t2 * segDir;
    if (!result.first && !result.second)
      return {};
    return result;
  }
  else if (linal::isZero(discriminant, eps))
  {
    T t = -b / 2 * a;
    if (linal::isGreaterEq(t, T(0), eps) && linal::isLessEq(t, T(1), eps))
      return SphereIntersection<T>(segSource + t * segDir);
  }
  return {};
}
} // namespace Geometry

#endif // GEOMETRY_INTERSECTIONSPHERE_HPP
