#ifndef GEOMETRY_DISTANCERAY_HPP
#define GEOMETRY_DISTANCERAY_HPP

#include <Geometry/Ray.hpp>
#include <Geometry/Utils/Compiler.hpp>
#include <linal/Vec.hpp>
#include <linal/VecOperations.hpp>
#include <linal/utils/Eps.hpp>

namespace Geometry
{

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(const Ray<T, D>& ray, linal::Vec<T, D> vec)
{
  linal::Vec<T, D> vecO{vec - ray.getOrigin()};
  const linal::Vec<T, D> rayDirection = ray.getDirection();

  const T dotProduct = linal::dot(vecO, rayDirection);
  linal::Vec<T, D> distanceVec;
  if (linal::isGreater(dotProduct, T(0)))
    distanceVec = linal::rejection(vecO, rayDirection);
  else
    distanceVec = ray.getOrigin() - vec;

  return linal::norm2(distanceVec);
}

template <typename T, std::size_t D>
GEO_NODISCARD GEO_CONSTEXPR T distance(linal::Vec<T, D> vec, const Ray<T, D>& ray)
{
  return distance(ray, vec);
}

} // namespace Geometry

#endif // GEOMETRY_DISTANCERAY_HPP
